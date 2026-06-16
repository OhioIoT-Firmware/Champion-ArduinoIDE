

#pragma once


/*
	Handler signatures.

	ThreeHandler  — topic, payload, error buffer.
	                Return true if you claim the message (success OR failure).
	                On failure, write a reason into the error buffer.
	                Return false only if the message is not yours.

	TwoHandler    — two strings (e.g. topic+payload, or group_name+auth_key).
	OneHandler    — payload only.   Returns nothing.
	ZeroHandler   — no arguments.   Returns nothing.
*/

using ThreeHandler  = bool (*)(const char*, const char*, char*);
using TwoHandler    = bool (*)(const char*, const char*);
using OtaHandler    = void (*)(const char*, const char*);
using OneHandler    = void (*)(const char*);
using ZeroHandler   = void (*)();

using MessageHandler = void (*)(char *, char *);



class Messages {

	public:

		Messages();

		static bool main_handler(char*, char*);		// the framework router; give this to mqtt.set_std_callback. returns true if it claimed the message.


		// Route-specific handlers.  The framework calls these when
		// it detects a matching topic (settings / ota / command / clear_counters / restart).

		void set_settings_handler(TwoHandler);
		void set_ota_handler(OtaHandler);
		void set_command_handler(ThreeHandler);
		void set_clear_counter_handler(ZeroHandler);
		void set_restart_handler(ZeroHandler);


		// NOTE: the user's "catch-all" raw handler is no longer registered here.
		// It now lives in mqtt (mqtt.set_callback), and the router falls through
		// to it automatically for any message it doesn't claim.

		// Group subscriptions moved to the mqtt module:
		//
		//   mqtt.set_command_namespaces(...)  → each group subscribes to
		//                                       ~/{group}/command/+ and ~/{group}/command/+/+
		//   mqtt.set_settings_namespaces(...) → each group subscribes to
		//                                       ~/{group}/settings/+
		//
		// Per-device addressing (~/{device_id}/...) is always installed by
		// the controller — namespaces are opt-in additions on top.
		//
		// OTA is per-device only.  The cloud addresses each device individually
		// at ~/~/ota/{group_name}, so there is no group OTA namespace —
		// device targeting is controlled by the cloud's device list, not by
		// firmware subscriptions.


	private:

		TwoHandler    _settings_handler      = nullptr;
		OtaHandler    _ota_handler           = nullptr;
		ThreeHandler  _command_handler       = nullptr;
		ZeroHandler   _clear_counter_handler = nullptr;
		ZeroHandler   _restart_handler       = nullptr;

		void _print_message(char * topic, char * message);
		void _get_segment(char* topic, int index, char* result);

};


extern Messages messages;