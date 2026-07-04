

#include "mqtt/mqtt.h"
#include "Arduino.h"

// #include credentials.h here to allow Arduino IDE users to toggle 
// ALLOW_INSECURE_MQTT in their credentials.h.  PlatformIO users
// can still use the build flag.

// #include "credentials.h"



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

    // #ifdef ALLOW_INSECURE_MQTT
    //     _wifi_client.setInsecure();
    // #else
    //     _wifi_client.setCACert(ca_cert);
    // #endif

    _mqtt_client.setClient(_wifi_client);
    _mqtt_client.setServer(mqtt_host, mqtt_port);
    strcpy(_device_id, device_id);
    strcpy(_username, username);
    strcpy(_password, password);

	_retry_timer = millis();

}

void Mqtt::maintain() {

    if (!_mqtt_client.connected() || !is_connected) {

        // we just noticed a drop — was connected last we knew, isn't now.
        // (the controller observes is_connected on each tick and counts
        // drops itself; this module no longer reaches into a callback.)
        is_connected = false;

        bool should_reconnect = _is_first_connect || millis() - _retry_timer > MQTT_RETRY_INTERVAL;
        if (should_reconnect) {
            Serial.println("\tmqtt connecting...");

            // a "retry" is any attempt that isn't the very first one after boot.
            // capture this BEFORE the connect call so success doesn't reset it.
            // bool is_retry = !_is_first_connect;

            if (_mqtt_client.connect(_device_id, _username, _password)) {
                Serial.println("\tmqtt connected...\n");
                is_connected = true;
                   // ← previously never flipped; meant retry timer
                                             //   was effectively bypassed forever.
                _subscribe_to_all();

            } else {
                Serial.println("\txx failed to connect to mqtt");
                is_connected = false;
                _retry_timer = millis();
                Serial.print("\txx failed to connect to mqtt, rc=");
                Serial.println(_mqtt_client.state());
            }

			_is_first_connect = false;

            // return is_retry;   // first-ever attempt isn't a retry; everything after is
        }

        // return false;          // dropped, but waiting on retry timer

    } else {
        _mqtt_client.loop();
        // return false;          // just servicing an existing connection
    }
  
}


void Mqtt::report_disconnect() {
    is_connected = false;
}


// use by both publish and subscribe
void Mqtt::_build_topic(const char * topic, char * output) {
    if (topic[0] == '~' && topic[1] == '/' && topic[2] == '~' && topic[3] == '/') {
        strcpy(output, _username);
        strcat(output, "/");
        strcat(output, _device_id);
        strcat(output, "/");
        strcat(output, topic + 4);
    } else if (topic[0] == '~' && topic[1] == '/') {
        strcpy(output, _username);
        strcat(output, "/");
        strcat(output, topic + 2);
    } else {
        strcpy(output, topic);
    }
}