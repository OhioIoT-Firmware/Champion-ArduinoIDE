



#include "mqtt/mqtt.h"
#include "Arduino.h"
#include "wifi_tools/serial_mask.h"


void Mqtt::publish_namespaces() {
    if (_command_namespaces == nullptr) return;

    char payload[160];
    payload[0] = '\0';
    size_t used = 0;

    for (int i = 0; _command_namespaces[i] != nullptr; i++) {
        const char * name = _command_namespaces[i];
        size_t name_len = strlen(name);
        size_t comma = used > 0 ? 1 : 0;
        if (used + comma + name_len >= sizeof(payload)) {
            Serial.println("\txx command namespace list is too long — extra names left out");
            break;
        }
        if (comma) {
            payload[used++] = ',';
            payload[used] = '\0';
        }
        strlcpy(payload + used, name, sizeof(payload) - used);
        used += name_len;
    }

    publish("~/~/status/cmd_ns", payload);
}


void Mqtt::publish(const char * topic, const char * payload) {
    _publish(topic, payload);
}

void Mqtt::publish(const char * topic, float number) {
    char payload[32];
    dtostrf(number, 0, 2, payload);
    _publish(topic, payload);
}

void Mqtt::publish(const char * topic, int number) {
    char payload[32];
    itoa(number, payload, 10);
    _publish(topic, payload);
}

void Mqtt::publish(const char * topic, bool payload) {
    _publish(topic, payload ? "true" : "false");
}


void Mqtt::publish(const char * topic) {
    _publish(topic, "");
}

void padEnd(const char * str, int distance) {

    if (strlen(str) < distance) {
        for (int i = 0; i < distance - strlen(str); i++ ) {
            Serial.print(" ");
        }
    }
}


void Mqtt::_publish(const char * topic, const char * payload) {

    char topic_build[MQTT_TOPIC_BUF];

    if (!_build_topic(topic, topic_build, sizeof(topic_build))) return;
    if (payload == nullptr) payload = "";

    if (is_connected) {
        Serial.print("\tsending: ");
        Serial.print(topic_build);
        padEnd(topic_build, 30);
        Serial.print("  ");
        if (payload && payload[0]) {
            // Settings echoes look like {"api_token":"the-real-value"}.
            // Mask that on the serial line only.  The bytes published to
            // the broker are still the real payload, and ordinary fields
            // such as {"threshold":"2.5"} are printed unchanged.
            char redacted[256];
            if (mask_json_secret(payload, redacted, sizeof(redacted))) {
                Serial.println(redacted);
            } else if (mask_text_secrets(payload, redacted, sizeof(redacted))) {
                // A published reading that happens to contain ?auth_key= or
                // password= is masked on the serial line only.
                Serial.println(redacted);
            } else {
                Serial.println(payload);
            }
        } else {
            Serial.println("[ empty ]");
        }

        // Same ceiling PubSubClient enforces: a 5-byte header reserve, a
        // 2-byte topic-length field, then the topic and the payload. The
        // library returns false and sends nothing, with no reason. Check
        // first so a too-big JSON payload is obvious on the serial monitor.
        const size_t packet_bytes = (size_t)MQTT_MAX_HEADER_SIZE + 2
            + strlen(topic_build) + strlen(payload);
        const uint16_t buffer_size = _mqtt_client.getBufferSize();
        if (packet_bytes > buffer_size) {
            Serial.print("\txx not sent, too big for the mqtt buffer: ");
            Serial.print(topic_build);
            Serial.print(" needs ");
            Serial.print((unsigned long)packet_bytes);
            Serial.print(" bytes, buffer is ");
            Serial.print((unsigned)buffer_size);
            Serial.println(" bytes. Raise MQTT_BUFFER_SIZE and rebuild.");
            return;
        }

        if (!_mqtt_client.publish(topic_build, payload)) {
            Serial.print("\txx publish failed: ");
            Serial.println(topic_build);
        }
    }
}
