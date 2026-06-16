

#include "messages/messages.h"

#include "Arduino.h"
#include "mqtt/mqtt.h"



Messages::Messages() {}

Messages messages;



void Messages::set_settings_handler(TwoHandler h)        { _settings_handler      = h; }
void Messages::set_ota_handler(OtaHandler h)             { _ota_handler           = h; }
void Messages::set_command_handler(ThreeHandler h)       { _command_handler       = h; }
void Messages::set_clear_counter_handler(ZeroHandler h)  { _clear_counter_handler = h; }
void Messages::set_restart_handler(ZeroHandler h)        { _restart_handler       = h; }


// Group namespace subscriptions now live in the mqtt module —
// see mqtt.set_command_namespaces / mqtt.set_settings_namespaces.
// Topics are built fresh at every connect, so nothing is stored here.



bool Messages::main_handler(char* topic, char* payload) {

	/*
		Messages is an administrative wrapper between incoming MQTT messages
		and the user's handlers.  It parses the topic, dispatches to the right
		handler, and publishes response topics based on success / failure.
	*/


	Serial.print("\n\t### Message:");
	messages._print_message(topic, payload);

	char third_elem[40], fourth_elem[40], fifth_elem[40], error[128];
	messages._get_segment(topic, 2, third_elem);
	messages._get_segment(topic, 3, fourth_elem);



	// SETTINGS UPDATES ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "settings") == 0) {
			Serial.println("\t@@ got a settings update");
			bool success = false;
			if (messages._settings_handler) success = messages._settings_handler(fourth_elem, payload);
			if (success) return true;
			// TODO:  we need to send an mqtt showing the new value, so that it hits the twin
		}


	// COMMANDS ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "command") == 0) {
			Serial.println("\t@@ got a command");

			error[0] = '\0';                        // clear the error buffer before each call
			bool claimed = false;

			if (messages._command_handler) {
				claimed = messages._command_handler(fourth_elem, payload, error);
			}

			if (claimed) {

				bool succeeded = (error[0] == '\0');

				char response_topic[100] = "~/~/response/command/";

				messages._get_segment(topic, 4, fifth_elem);
				if (fifth_elem[0]) {
					strcat(response_topic, fifth_elem);
					strcat(response_topic, "/");
				}
				strcat(response_topic, succeeded ? "success" : "fail");

				mqtt.publish(response_topic, succeeded ? "" : error);
				return true;
			} else {
				Serial.println("comamnd not claimed by the device");
			}
			// not claimed — fall through to user handler / default below
		}


	// OTA ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "ota") == 0) {
			Serial.println("\t@@ got an OTA update");
			if (messages._ota_handler) {
				// fourth_elem is the group_name (from topic).
				// payload is the auth_key (raw string, not JSON).
				messages._ota_handler(fourth_elem, payload);
				return true;
			}
		}


	// SYSTEM COUNTERS ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "clear_counters") == 0) {
			if (messages._clear_counter_handler) {
				messages._clear_counter_handler();
				return true;
			}
		}


	// RESTART ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "restart") == 0) {
			if (messages._restart_handler) {
				messages._restart_handler();
				return true;
			}
		}


	// NOTHING CLAIMED IT ++++++++++++++++++++++++++++++++++++++
	//
	// No framework route matched.  Return false so mqtt falls through to the
	// user's raw handler (mqtt.set_callback), if they set one.  If they didn't,
	// the message is simply ignored — the user owns un-routed traffic.

	return false;

}