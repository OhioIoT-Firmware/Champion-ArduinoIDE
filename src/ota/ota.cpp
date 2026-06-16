

#include "ota/ota.h"

#include "Arduino.h"

#include "esp_https_ota.h"




OTA::OTA() {}

OTA ota;


void OTA::set_ca_cert(const char * cert) {
    // Stored as a pointer (see NOTE in update_firmware): pass a cert with
    // program-long lifetime — a global, static, or string literal.
    _ota_server_certificate = cert;
}


// http_config and the URL buffer must outlive update_firmware() because
// esp_https_ota holds pointers into them across many perform() calls.
// File-scope statics keep them alive for the duration of the download.
static esp_http_client_config_t _http_config = {};
static char _ota_url[256];



void OTA::initialize() {
    Serial.println("\n-- ENTERING ota.initialize()");
    _rollback_timer = millis();
    _need_to_check_rollback_once = true;

    _mark_firmware_as_valid_if_appropriate();
    Serial.println("-- LEAVING ota.initialize()\n");
}


bool OTA::update_firmware(const char * group_name, const char * auth_key) {

    // Refuse new OTA starts unless we're fully idle.  This catches the
    // SUCCESS-but-not-yet-rebooted window where an echoed MQTT message
    // could otherwise kick off a second OTA between SUCCESS and reboot.
    if (_state != IDLE) {
        Serial.println("\txxx OTA not idle; ignoring new request");
        return false;
    }

    uint32_t free_heap_size     = esp_get_free_heap_size();
    uint32_t min_free_heap_size = esp_get_minimum_free_heap_size();
    printf("\n\tfree heap size = %d \t  min_free_heap_size = %d \n", free_heap_size, min_free_heap_size);

    const char * key_to_use = auth_key;
    if (!key_to_use || !*key_to_use) {
        #ifdef AUTH_KEY
            key_to_use = AUTH_KEY;
        #else
            Serial.println("\txxx OTA aborted: no auth_key passed in and AUTH_KEY not #defined");
            return false;
        #endif
    }

    if (!group_name || !*group_name) {
        Serial.println("\txxx OTA aborted: missing group_name");
        return false;
    }

    if (!_ota_server_certificate) {
        Serial.println("\txxx OTA aborted: CA cert not set (call ota.set_ca_cert() in setup)");
        return false;
    }

    // Remember the group across the impending reboot so the next boot can
    // report accepted/rolled_back tagged with the correct group.
    save_ota_group(group_name);

    snprintf(_ota_url, sizeof(_ota_url), "%s/download/%s?auth_key=%s",
             OTA_URL, group_name, key_to_use);

    Serial.print("\ttotal url: ");
    Serial.println(_ota_url);
    Serial.println("\n\t.. starting OTA");

    _http_config = {};
    _http_config.url               = _ota_url;
    _http_config.event_handler     = &OTA::_http_event_handler;
    _http_config.timeout_ms        = OTA_TIMEOUT;
    _http_config.keep_alive_enable = true;
    // NOTE: cert_pem stores the POINTER, not a copy — the cert string must
    // outlive the whole OTA download. Only ever pass a global/static/literal.
    _http_config.cert_pem          = _ota_server_certificate;

    esp_https_ota_config_t ota_config = {};
    ota_config.http_config = &_http_config;

    esp_https_ota_handle_t handle = nullptr;
    esp_err_t err = esp_https_ota_begin(&ota_config, &handle);
    if (err != ESP_OK) {
        Serial.printf("\txxx esp_https_ota_begin failed: %s (0x%x)\n", esp_err_to_name(err), err);
        _state = FAILED;
        return false;
    }

    _ota_handle = (void *) handle;
    _percent    = 0;
    _state      = IN_PROGRESS;

    Serial.println("\t.. OTA started (non-blocking); progress will report from check()");
    return true;
}


void OTA::check() {

    if (_state != IN_PROGRESS) return;

    esp_err_t err = esp_https_ota_perform((esp_https_ota_handle_t) _ota_handle);
    if (err == ESP_ERR_HTTPS_OTA_IN_PROGRESS) return;   // not done yet

    // perform() returned something terminal — finalize the partition
    esp_err_t finish_err = esp_https_ota_finish((esp_https_ota_handle_t) _ota_handle);
    _ota_handle = nullptr;

    if (err == ESP_OK && finish_err == ESP_OK) {
        Serial.println("\n\n--- OTA SUCCESS ---\n");
        set_ota_pending();
        _state = SUCCESS;
    } else {
        Serial.printf("\n\nOTA FAILED: perform=%s finish=%s\n",
                      esp_err_to_name(err), esp_err_to_name(finish_err));
        _state = FAILED;
    }
}


void OTA::acknowledge() {
    // caller has handled SUCCESS or FAILED — reset to IDLE so a future
    // update_firmware() call can run.  no-op if we're already IDLE or IN_PROGRESS.
    if (_state == SUCCESS || _state == FAILED) {
        _state   = IDLE;
        _percent = 0;
    }
}