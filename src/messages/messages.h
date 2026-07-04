

#pragma once


/*
	Handler types.  Named for what they do, not how many arguments they take —
	and the parameter names below are part of the documentation: they tell you
	exactly what your function will receive.

	Return-value rule (applies to CommandHandler only): return true if you
	claim the message — success OR failure.  On failure, write a reason into
	the error buffer.  Return false only if the message is not yours.
*/

using CommandHandler   = bool (*)(const char * name, const char * payload, char * error);
using SettingsHandler  = bool (*)(const char * key,  const char * value);
using OtaStartHandler  = void (*)(const char * group, const char * auth_key);
using ActionHandler    = void (*)();	// fire-and-forget, no arguments
using TrackedActionHandler = void (*)(const char * code);	// code is "" when untracked



class Messages {

	public:

		Messages();

		// The framework router.  The controller gives this to
		// mqtt.set_std_callback.  Returns true if it claimed the message;
		// anything unclaimed falls through to the user's raw handler
		// (mqtt.set_callback).
		static bool main_handler(char *, char *);


		// Route-specific handlers.  The framework calls these when it
		// detects a matching topic (settings / command / ota / clear_counters
		// / restart).  All are registered by the controller — except
		// set_command_handler, which the user registers from main.cpp.

		void set_settings_handler(SettingsHandler);
		void set_command_handler(CommandHandler);
		void set_ota_handler(OtaStartHandler);

		// Tracked device actions.  The handler receives the optional tracking
		// code from the topic (~/~/clear_counters/{code}) — empty string if
		// none was supplied.  Whether/what to publish in response is the
		// handler's business; passive confirmation is the state report:
		// counters hit zero; starts increments / uptime resets on next boot.
		void set_clear_counter_handler(TrackedActionHandler);
		void set_restart_handler(TrackedActionHandler);

		// OTA lifecycle actions (champion).  Registered in the controller's
		// @Champion block.
		void set_ota_abort_handler(ActionHandler);     // cancel an in-flight download
		void set_ota_accept_handler(ActionHandler);    // mark pending firmware valid
		void set_ota_rollback_handler(ActionHandler);  // revert to the previous image


		// Group subscriptions live in the mqtt module:
		//
		//   mqtt.set_command_namespaces(...)  → ~/{group}/command/+ and ~/{group}/command/+/+
		//   mqtt.set_settings_namespaces(...) → ~/{group}/settings/+
		//
		// Per-device addressing (~/{device_id}/...) is always installed by
		// the controller — namespaces are opt-in additions on top.
		//
		// OTA is per-device only.  The cloud addresses each device
		// individually at ~/~/ota/{group_name}, so there is no group OTA
		// namespace — device targeting is controlled by the cloud's device
		// list, not by firmware subscriptions.


	private:

		SettingsHandler  _settings_handler      = nullptr;
		CommandHandler   _command_handler       = nullptr;
		OtaStartHandler  _ota_handler           = nullptr;
		TrackedActionHandler _clear_counter_handler = nullptr;
		TrackedActionHandler _restart_handler       = nullptr;
		ActionHandler    _ota_abort_handler     = nullptr;
		ActionHandler    _ota_accept_handler    = nullptr;
		ActionHandler    _ota_rollback_handler  = nullptr;

		void _print_message(char * topic, char * message);
		void _get_segment(char * topic, int index, char * result);

};


extern Messages messages;