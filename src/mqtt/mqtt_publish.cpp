

#include "mqtt/mqtt.h"
#include "Arduino.h"


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

// void Mqtt::publish(const char * topic, bool state) {
//     char payload[32];
//     itoa(state, payload, 1);
//     _publish(topic, payload);
// }

void Mqtt::publish(const char * topic) {
    _publish(topic, "");
}

void padEnd(const char * str, int distance) {
    // Serial.print("str: ");
    // Serial.println(str);
    // Serial.print("\tstrlen: ");
    // Serial.println(strlen(str));
    // Serial.print("\tstrlen(str) < distance: ");
    // Serial.println(strlen(str) < distance);

    if (strlen(str) < distance) {
        for (int i = 0; i < distance - strlen(str); i++ ) {
            Serial.print(" ");
        }
    }
}


void Mqtt::_publish(const char * topic, const char * payload) {

    char topic_build[100];

    _build_topic(topic, topic_build);   // replaces '~/' with user values

    if (is_connected) {
        Serial.print("\tsending: ");
        Serial.print(topic_build);
        padEnd(topic_build, 30);
        Serial.print("  ");
        if (strlen(payload)) {
            Serial.println(payload);
        } else {
            Serial.println("[ empty ]");
        }
        _mqtt_client.publish(topic_build, payload);
    }
}


