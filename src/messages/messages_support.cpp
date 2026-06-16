


#include "messages/messages.h"

#include "Arduino.h"


void Messages::_print_message(char * topic, char * payload) {
	Serial.print("\ttopic: ");
	Serial.print(topic);
	Serial.print("\t\tpayload: ");
	Serial.println(payload);
}




void Messages::_get_segment(char* topic, int index, char* result) {
    int current_segment = 0;
    int topic_pos = 0;
    int result_pos = 0;
    result[0] = 0;
    if (index == 0) {
        while (topic[topic_pos] != 0 && topic[topic_pos] != '/') {
            if (result_pos < 39) {
                result[result_pos] = topic[topic_pos];
                result_pos++;
            }
            topic_pos++;
        }
        result[result_pos] = 0;
        return;  // changed
    }
    while (topic[topic_pos] != 0 && current_segment < index) {
        if (topic[topic_pos] == '/') {
            current_segment++;
        }
        topic_pos++;
    }
    if (current_segment < index) {
        return;  // changed
    }
    while (topic[topic_pos] != 0 && topic[topic_pos] != '/') {
        if (result_pos < 39) {
            result[result_pos] = topic[topic_pos];
            result_pos++;
        }
        topic_pos++;
    }
    result[result_pos] = 0;
}

