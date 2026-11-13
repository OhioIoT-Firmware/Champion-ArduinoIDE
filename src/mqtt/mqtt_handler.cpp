

#include "mqtt/mqtt.h"
#include "Arduino.h"


void Mqtt::set_callback(MessageHandler handler) {
    _stored_handler = handler;
    _mqtt_client.setCallback(_callback_wrapper);
}

void Mqtt::set_std_callback(StdMessageHandler handler) {
    _std_handler = handler;
    _mqtt_client.setCallback(_callback_wrapper);
}

// void Mqtt::_callback_wrapper(char * topic, byte * payload, unsigned int length) {
//     char message[length+1];
//     memcpy(message, payload, length);
//     message[length] = 0;
//     mqtt._stored_handler(topic, message);
// }

void Mqtt::_callback_wrapper(char * topic, byte * payload, unsigned int length) {
    char message[256];   // matches PubSubClient's MQTT_MAX_PACKET_SIZE default;
                         // the library refuses packets larger than that, so this
                         // is sized for the worst case it can hand us.
    if (payload == nullptr) length = 0;
    if (length >= sizeof(message)) {
        Serial.println("\txx mqtt payload too long — truncated to 255 characters");
        length = sizeof(message) - 1;
    }
    if (length > 0) memcpy(message, payload, length);
    message[length] = '\0';
    // Same constant as the packet buffer. Static, not a local array: if a
    // sketch raises MQTT_BUFFER_SIZE, a big stack temporary in the
    // callback can blow the loop task's stack. PubSubClient only calls us
    // with a payload that already fit in its buffer, so this holds it.
    static char message[MQTT_BUFFER_SIZE];
    if (length >= sizeof(message)) {
        Serial.print("\txx incoming mqtt message truncated from ");
        Serial.print(length);
        Serial.print(" to ");
        Serial.print(sizeof(message) - 1);
        Serial.println(" bytes");
        length = sizeof(message) - 1;
    }
    memcpy(message, payload, length);
    message[length] = 0;

    // Framework router gets first look (Scaler/Champion only — nullptr otherwise).
    // If it claims the message it returns true and we stop here.
    if (mqtt._std_handler != nullptr) {
        if (mqtt._std_handler(topic, message)) return;
    }

    // Otherwise the user's raw handler runs (any tier, if they set one).
    if (mqtt._stored_handler != nullptr) {
        mqtt._stored_handler(topic, message);
    }
}