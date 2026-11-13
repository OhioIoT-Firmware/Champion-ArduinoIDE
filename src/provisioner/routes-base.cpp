

#include "provisioner/provisioner.h"
#include "Arduino.h"
#include "wifi_tools/serial_mask.h"
#include "provisioner/html.h"

void Provisioner::_set_base_routes() {

    _web_server.on("/", [this]() {
        Serial.println("\t.. base route hit");
        _web_server.send(200, "text/html", CONFIG_PAGE);
    });

    _web_server.on("/save", [this]() {

        Serial.println("\t.. save route hit");

        size_t ssid_len = strlcpy(_ssid, _web_server.arg("ssid").c_str(), WIFI_SSID_LEN);
        size_t pass_len = strlcpy(_pass, _web_server.arg("pass").c_str(), WIFI_PASS_LEN);

        // A chopped password is a different password. Refuse it instead of
        // saving the truncated text and pretending provisioning worked.
        if (ssid_len >= WIFI_SSID_LEN || pass_len >= WIFI_PASS_LEN) {
            Serial.println("\txx SSID max is 32 characters and password max is 63 — not saved");
            _ssid[0] = '\0';
            _pass[0] = '\0';
            _web_server.send(200, "text/html", INSTRUCTIONS_PAGE);
            return;
        }

        // SSID stays visible so you can see which network was saved.
        // The password is masked — serial logs get copied into forum
        // posts and videos, and this line used to be the whole passphrase.
        char masked_pass[16];
        mask_secret(_pass, masked_pass, sizeof(masked_pass));
        Serial.print("\n\tnew SSID: "); Serial.println(_ssid);
        Serial.print("\tnew PASS: ");   Serial.println(masked_pass);

        if (strlen(_ssid) < 4 || strlen(_pass) < 4) {
            Serial.println("\t.. error - SSID or password failed validation");
            _web_server.send(200, "text/html", INSTRUCTIONS_PAGE);
        } else {
            _provisioning_complete = true;
            _web_server.send(200, "text/html", SAVED_PAGE);
        }

    });

}

