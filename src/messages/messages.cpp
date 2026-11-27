

#include "messages/messages.h"

#include "Arduino.h"
#include "mqtt/mqtt.h"



Messages::Messages() {}

Messages messages;



void Messages::set_settings_handler(SettingsHandler h)   { _settings_handler      = h; }
void Messages::set_command_handler(CommandHandler h)     { _command_handler       = h; }
void Messages::set_ota_handler(OtaStartHandler h)        { _ota_handler           = h; }
void Messages::set_clear_counter_handler(TrackedActionHandler h){ _clear_counter_handler = h; }
void Messages::set_restart_handler(TrackedActionHandler h)      { _restart_handler       = h; }
void Messages::set_ota_abort_handler(ActionHandler h)    { _ota_abort_handler     = h; }
void Messages::set_ota_accept_handler(ActionHandler h)   { _ota_accept_handler    = h; }
void Messages::set_ota_rollback_handler(ActionHandler h) { _ota_rollback_handler  = h; }


// group namespace subscriptions are built in the mqtt module on each connect and are not stored here



bool Messages::main_handler(char* topic, char* payload) {

	/*
		Messages is an administrative wrapper between incoming MQTT messages
		and the user's handlers.  It parses the topic, dispatches to the right
		handler, and publishes response topics based on success / failure.
	*/


	Serial.print("\n\t### Message:");
	messages._print_message(topic, payload);

    // with a username the route is the third segment and without one it is the second
    int first = mqtt.has_username() ? 2 : 1;

	char third_elem[40], fourth_elem[40], fifth_elem[40], error[128];
    messages._get_segment(topic, first, third_elem, sizeof(third_elem));
    messages._get_segment(topic, first + 1, fourth_elem, sizeof(fourth_elem));
    messages._get_segment(topic, first + 2, fifth_elem, sizeof(fifth_elem));


	// SETTINGS UPDATES ++++++++++++++++++++++++++++++++++++++

		if (strcmp(third_elem, "settings") == 0) {
			Serial.println("\t@@ got a settings update");
			// fourth_elem is the setting's key (from ~/~/settings/{key});
			// payload is the raw new value.  Settings topics are framework-
			// owned, so a registered handler always claims the message —
			// even when the update is refused.  The handler (installed by
			// the controller) applies the update and echoes the value the
			// device holds afterward, which is what feeds the twin and
			// keeps the dashboard honest.
			if (messages._settings_handler) {
				messages._settings_handler(fourth_elem, payload);
				return true;
			}
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

				char response_topic[100];
				const char * outcome = succeeded ? "success" : "fail";
				int wrote;
				if (fifth_elem[0]) {
					wrote = snprintf(response_topic, sizeof(response_topic),
					                 "~/~/response/command/%s/%s", fifth_elem, outcome);
				} else {
					wrote = snprintf(response_topic, sizeof(response_topic),
					                 "~/~/response/command/%s", outcome);
				}
				if (wrote < 0 || (size_t)wrote >= sizeof(response_topic)) {
					Serial.println("\txx command response topic too long — not published");
				} else {
					mqtt.publish(response_topic, succeeded ? "" : error);
				}
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
	// Topic: ~/~/clear_counters or ~/~/clear_counters/{code}.  fourth_elem
	// was already parsed at the top of this function — empty string if no
	// code segment was present.  Response (if any) is published by the
	// registered handler itself, isolated under .../response/clear_counters/...
	// — never .../response/command/..., so it can't be confused with the
	// generic command-response channel.

		if (strcmp(third_elem, "clear_counters") == 0) {
			if (messages._clear_counter_handler) {
				messages._clear_counter_handler(fourth_elem);
				return true;
			}
		}


	// RESTART ++++++++++++++++++++++++++++++++++++++
	// Topic: ~/~/restart or ~/~/restart/{code}.  Same code-passing as
	// clear_counters above.  Response (if any) is published by the
	// registered handler under .../response/restart/..., before it reboots.

		if (strcmp(third_elem, "restart") == 0) {
			if (messages._restart_handler) {
				messages._restart_handler(fourth_elem);
				return true;
			}
		}


	// OTA LIFECYCLE ACTIONS ++++++++++++++++++++++++++++++++++++++
	// Parameterless, same shape as restart.  Single-segment topics, so they do
	// NOT collide with the ~/~/ota/{group} start route (which matches "ota").

		if (strcmp(third_elem, "ota_abort") == 0) {
			if (messages._ota_abort_handler) {
				messages._ota_abort_handler();
				return true;
			}
		}

		if (strcmp(third_elem, "ota_accept") == 0) {
			if (messages._ota_accept_handler) {
				messages._ota_accept_handler();
				return true;
			}
		}

		if (strcmp(third_elem, "ota_rollback") == 0) {
			if (messages._ota_rollback_handler) {
				messages._ota_rollback_handler();
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