

#include "mqtt/mqtt.h"
#include "Arduino.h"


// Copy src into dest, always null-terminated. If src does not fit, dest is
// cleared and a warning is printed — a chopped password or client id would
// be a different credential, so it is not used at all.
static bool _copy_or_drop(char * dest, size_t dest_len, const char * src, const char * label) {
    if (dest == nullptr || dest_len == 0) return false;
    if (src == nullptr) src = "";
    size_t copied = strlcpy(dest, src, dest_len);
    if (copied >= dest_len) {
        Serial.print("\txx ");
        Serial.print(label);
        Serial.print(" is too long (max ");
        Serial.print(dest_len - 1);
        Serial.println(" characters) — not used");
        dest[0] = '\0';
        return false;
    }
    return true;
}



#include "esp_random.h"


// How far apart retries get, and how long one attempt is allowed to sit
// inside connect(). The attempt itself still blocks (the MQTT client
// connects synchronously); these only bound that stall and the gap after it.
static const unsigned long MQTT_RETRY_MAX_INTERVAL = 60000UL;
static const unsigned long MQTT_RETRY_JITTER_PERCENT = 25UL;
static const uint16_t MQTT_ATTEMPT_TIMEOUT_S = 5;


// +/- 25% so a fleet doesn't retry in lockstep, clamped to the cap.
static unsigned long with_jitter(unsigned long base_ms) {
	unsigned long span = base_ms * MQTT_RETRY_JITTER_PERCENT / 100UL;
	unsigned long delay_ms = base_ms;
	if (span > 0) {
		delay_ms = base_ms - span + (esp_random() % (span * 2UL + 1UL));
	}
	if (delay_ms < 1000UL) delay_ms = 1000UL;
	if (delay_ms > MQTT_RETRY_MAX_INTERVAL) delay_ms = MQTT_RETRY_MAX_INTERVAL;
	return delay_ms;
}


Mqtt::Mqtt() {}

Mqtt mqtt;

void Mqtt::setup(const char * mqtt_host, int mqtt_port, const char * username, const char * password, const char * device_id, const char * ca_cert) {

	// NOTE: setCACert() / cert_pem store the POINTER to this string, not a copy.
	// The cert text must stay alive for the whole connection / OTA download.
	// OHIOIOT_CA_CERT is a global, so it's always safe. Only ever pass a cert
	// with program-long lifetime (global, static, or string literal) — never a
	// local buffer created inside a function, or the pointer goes stale.

    #ifndef ALLOW_INSECURE_MQTT
        _wifi_client.setCACert(ca_cert);
    #endif

    _mqtt_client.setClient(_wifi_client);
    _mqtt_client.setServer(mqtt_host, mqtt_port);
    _copy_or_drop(_device_id, sizeof(_device_id), device_id, "device id");
    _copy_or_drop(_username, sizeof(_username), username, "mqtt username");
    _copy_or_drop(_password, sizeof(_password), password, "mqtt password");

    // Before the first connect (that happens later, in maintain). The client
    // allocates 256 bytes in its constructor. setBufferSize is what replaces
    // it, and only when it returns true. Doing this after connect can leave
    // getBufferSize() looking right while publish still refuses the message.
    if (_mqtt_client.setBufferSize(MQTT_BUFFER_SIZE)) {
        Serial.print("\tmqtt packet buffer: ");
        Serial.print((unsigned)_mqtt_client.getBufferSize());
        Serial.println(" bytes");
    } else {
        Serial.print("\txx mqtt packet buffer stayed at ");
        Serial.print((unsigned)_mqtt_client.getBufferSize());
        Serial.print(" bytes (wanted ");
        Serial.print((unsigned)MQTT_BUFFER_SIZE);
        Serial.println("). Messages larger than that will be dropped.");
    }

    strcpy(_device_id, device_id);
    strcpy(_username, username);
    strcpy(_password, password);

    // WiFiClientSecure otherwise allows one try to freeze loop() for 30s of
    // TCP plus up to 120s of TLS handshake. Cap both. This does not make the
    // connect non-blocking — PubSubClient::connect() still runs to completion
    // before maintain() returns — it just stops a single try from stalling
    // the rest of the sketch for half a minute or more.
    // Plain (non-TLS) WiFiClient already gives up on TCP after about 3s.
    #ifndef ALLOW_INSECURE_MQTT
        // Handshake timeout is in seconds on Arduino core 2.x and 3.x.
        _wifi_client.setHandshakeTimeout(MQTT_ATTEMPT_TIMEOUT_S);
        #if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
            // 3.x stores the connect timeout in milliseconds.
            _wifi_client.setConnectionTimeout(MQTT_ATTEMPT_TIMEOUT_S * 1000UL);
        #else
            // 2.x setTimeout() takes seconds; connect() uses that value.
            _wifi_client.setTimeout(MQTT_ATTEMPT_TIMEOUT_S);
        #endif
    #endif

	_reset_retry();
	_retry_timer = millis();

}

void Mqtt::maintain() {

    if (!_mqtt_client.connected() || !is_connected) {

        // we just noticed a drop — was connected last we knew, isn't now.
        // (the controller observes is_connected on each tick and counts
        // drops itself; this module no longer reaches into a callback.)
        is_connected = false;

        // First attempt after boot runs immediately. After a failure we wait
        // out the jittered backoff. A successful connect clears that wait, so
        // the next drop tries once right away and only then backs off again.
        // Nothing here calls delay(); maintain() just returns until it's due.
        bool due = (millis() - _retry_timer) >= _retry_wait_ms;
        bool should_reconnect = _is_first_connect || !_retry_waiting || due;
        if (should_reconnect) {
            Serial.println("\tmqtt connecting...");

            // After TCP+TLS succeed, PubSubClient waits for CONNACK in a tight
            // loop (no yield) for its socket timeout, which defaults to 15s.
            // Shorten that for this attempt only, then put the normal timeout
            // back so a healthy session isn't held to the attempt budget.
            _mqtt_client.setSocketTimeout(MQTT_ATTEMPT_TIMEOUT_S);
            bool connected = _mqtt_client.connect(_device_id, _username, _password);
            _mqtt_client.setSocketTimeout(MQTT_SOCKET_TIMEOUT);

            if (connected) {
                Serial.println("\tmqtt connected...\n");
                is_connected = true;
                _reset_retry();
                _subscribe_to_all();

            } else {
                is_connected = false;
                _schedule_retry();
                Serial.print("\txx failed to connect to mqtt, rc=");
                Serial.println(_mqtt_client.state());
                Serial.print("\t   next try in ");
                Serial.print(_retry_wait_ms);
                Serial.println(" ms");
            }

			_is_first_connect = false;
        }

    } else {
        _mqtt_client.loop();
    }
  
}


void Mqtt::_schedule_retry() {
	_retry_wait_ms = with_jitter(_retry_base_ms);
	_retry_timer = millis();
	_retry_waiting = true;

	// 3s, 6s, 12s, 24s, 48s, then sit at the 60s cap.
	if (_retry_base_ms > MQTT_RETRY_MAX_INTERVAL / 2UL) {
		_retry_base_ms = MQTT_RETRY_MAX_INTERVAL;
	} else {
		_retry_base_ms *= 2UL;
	}
}


void Mqtt::_reset_retry() {
	_retry_base_ms = MQTT_RETRY_INTERVAL;
	_retry_wait_ms = 0;
	_retry_waiting = false;
}


void Mqtt::report_disconnect() {
    is_connected = false;
}


// use by both publish and subscribe.
// Returns false when the expanded topic does not fit. output is left empty
// so the caller does not publish or subscribe to a chopped topic.
bool Mqtt::_build_topic(const char * topic, char * output, size_t output_len) {
    if (output == nullptr || output_len == 0) return false;
    output[0] = '\0';

    if (topic == nullptr) {
        Serial.println("\txx topic missing — not used");
        return false;
    }

    int wrote;
    if (topic[0] == '~' && topic[1] == '/' && topic[2] == '~' && topic[3] == '/') {
        wrote = snprintf(output, output_len, "%s/%s/%s", _username, _device_id, topic + 4);
    } else if (topic[0] == '~' && topic[1] == '/') {
        wrote = snprintf(output, output_len, "%s/%s", _username, topic + 2);
    } else {
        wrote = snprintf(output, output_len, "%s", topic);
    }

    if (wrote < 0 || (size_t)wrote >= output_len) {
        Serial.print("\txx topic too long — not used: ");
        Serial.println(topic);
        output[0] = '\0';
        return false;
    }
    return true;
}