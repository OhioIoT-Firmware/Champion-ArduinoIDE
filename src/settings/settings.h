

#pragma once

#include <stddef.h>


/*
	SETTINGS — remotely updatable, persisted device values.

	The rules (all three of them):

	  1. Every setting lives in a char buffer of SETTINGS_BUF_SIZE.
	     The buffer's initializer is its compiled default.

	  2. Anything you want to use as a number gets its own typed variable,
	     derived from the buffer inside your conversion function.

	  3. You write ONE conversion function.  No keys, no ifs — just a
	     straight list, one line per numeric setting.  The framework calls
	     it once at boot (after stored values load) and again after every
	     remote update.  Strings need no line at all — the buffer IS the
	     setting.

	Everything else — NVS persistence, MQTT routing, echoing values to the
	device dashboard — is handled by the controller.  You never touch it.
*/


// A setting's value is a string of up to SETTINGS_BUF_SIZE - 1 characters.
// Override with a build flag if you truly need longer.
#ifndef SETTINGS_BUF_SIZE
	#define SETTINGS_BUF_SIZE 32
#endif

#define SETTINGS_MAX_KEY_LEN	16		// NVS limit: 15 chars + null
#define SETTINGS_MAX_ROWS		64		// runaway guard — if we walk this far, the sentinel row is missing
#define SETTINGS_NAMESPACE		"settings"


/*
	One row per setting:  { "dashboard_key", buffer }

	  key     — the setting's public name.  It's what shows on the device
	            dashboard, what keys the stored value, and what rides in
	            the MQTT topic.  Max 15 chars; no '/', '+', '#', or spaces.
	  buffer  — your char[SETTINGS_BUF_SIZE].  Name it whatever you like;
	            the cloud only ever sees the key.

	End the table with a  { nullptr, nullptr }  row — same convention as
	the subscription lists.
*/
struct SettingRow {
	const char *	key;
	char *			buffer;
};


using SettingsChangedHandler = void (*)();				// user's conversion function
using SettingsFailureHandler = void (*)(const char *);	// receives a human-readable failure line


class Settings {

	public:

		Settings();

		// --- user-facing ---------------------------------------------

		// Hand over the table and your conversion function.  Loads any
		// stored values from flash (first boot persists your compiled
		// defaults instead), then runs your conversion function once so
		// your typed variables are populated before loop() ever runs.
		void begin(SettingRow * table, SettingsChangedHandler on_change);

		// Conversion helpers for use inside your conversion function.
		// The fallback flavor returns your fallback (instead of 0) when
		// the buffer doesn't parse — and reports the failure (see
		// set_failure_handler).
		float	convert_to_float(const char * buffer);
		float	convert_to_float(const char * buffer, float fallback);
		int		convert_to_int(const char * buffer);
		int		convert_to_int(const char * buffer, int fallback);
		bool	convert_to_bool(const char * buffer);	// "1" / "true" / "on"

		// --- framework-facing (called by the controller) --------------

		// Apply a remote update.  Refuses (returns false) if the key is
		// unknown or the value doesn't fit the buffer; otherwise stores,
		// persists, and runs the user's conversion function.
		bool set(const char * key, const char * value);

		// Where conversion-failure text goes.  The controller injects a
		// function here that publishes to the device's message channel —
		// settings itself never touches MQTT.
		void set_failure_handler(SettingsFailureHandler);

		// Iteration — lets the controller publish the settings snapshot.
		int				count();
		const char *	key_at(int index);
		const char *	value_at(int index);

		bool has(const char * key);
		void print();


	private:

		SettingRow *			_table = nullptr;
		int						_count = 0;

		SettingsChangedHandler	_on_change = nullptr;
		SettingsFailureHandler	_on_failure = nullptr;

		int		_find(const char * key);
		bool	_key_is_valid(const char * key);
		void	_report_failure(const char * buffer, const char * detail);
		const char *	_key_for_buffer(const char * buffer);

};


extern Settings settings;