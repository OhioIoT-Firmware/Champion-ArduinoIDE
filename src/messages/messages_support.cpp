

#include "messages/messages.h"

#include "Arduino.h"
#include "wifi_tools/serial_mask.h"
#include "mqtt/mqtt.h"


void Messages::_print_message(char * topic, char * payload) {
	Serial.print("\ttopic: ");
	Serial.print(topic);
	Serial.print("\t\tpayload: ");

	// The OTA route's payload IS the auth key.  A settings or command
	// route carries a secret only when its name says so (password, token,
	// api_key, ...).  Everything else — start, threshold, inbox text —
	// is still printed in full.
    // same position as the router uses
    int first = mqtt.has_username() ? 2 : 1;

	char route[40];
	char name[40];
    _get_segment(topic, first, route, sizeof(route));

	bool hide = false;
	if (strcmp(route, "ota") == 0) {
		hide = true;
	} else if (strcmp(route, "settings") == 0 || strcmp(route, "command") == 0) {
        _get_segment(topic, first + 1, name, sizeof(name));
		hide = name_is_secret(name);
	}

	if (hide) {
		char masked[16];
		mask_secret(payload, masked, sizeof(masked));
		Serial.println(masked);
	} else {
		char scrubbed[256];
		if (payload && mask_text_secrets(payload, scrubbed, sizeof(scrubbed))) {
			Serial.println(scrubbed);
		} else {
			Serial.println(payload);
		}
	}
}




void Messages::_get_segment(char * topic, int index, char * result, size_t result_len) {
    if (result == nullptr || result_len == 0) return;
    result[0] = '\0';
    if (topic == nullptr || index < 0) return;

    size_t max_chars = result_len - 1;
    int topic_pos = 0;
    size_t result_pos = 0;
    bool truncated = false;

    if (index == 0) {
        while (topic[topic_pos] != '\0' && topic[topic_pos] != '/') {
            if (result_pos < max_chars) {
                result[result_pos++] = topic[topic_pos];
            } else {
                truncated = true;
            }
            topic_pos++;
        }
        result[result_pos] = '\0';
        if (truncated) {
            Serial.print("\txx topic segment 0 truncated to ");
            Serial.print((unsigned)max_chars);
            Serial.println(" characters");
        }
        return;
    }

    int current_segment = 0;
    while (topic[topic_pos] != '\0' && current_segment < index) {
        if (topic[topic_pos] == '/') {
            current_segment++;
        }
        topic_pos++;
    }
    if (current_segment < index) {
        return;
    }
    while (topic[topic_pos] != '\0' && topic[topic_pos] != '/') {
        if (result_pos < max_chars) {
            result[result_pos++] = topic[topic_pos];
        } else {
            truncated = true;
        }
        topic_pos++;
    }
    result[result_pos] = '\0';
    if (truncated) {
        Serial.print("\txx topic segment ");
        Serial.print(index);
        Serial.print(" truncated to ");
        Serial.print((unsigned)max_chars);
        Serial.println(" characters");
    }
}
