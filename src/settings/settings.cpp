

#include "settings/settings.h"
#include "Arduino.h"
#include "Preferences.h"

static Preferences _prefs;

Settings::Settings() {}

Settings settings;



// ============================================================
//  BEGIN — load stored values, run the conversion function once
// ============================================================

void Settings::begin(SettingRow * table, SettingsChangedHandler on_change) {

	Serial.println("\n-- settings.begin()");

	_table     = table;
	_on_change = on_change;
	_count     = 0;

	if (_table == nullptr) {
		Serial.println("\txx settings: no table provided");
		return;
	}

	// Walk the table.  A null key ends it; SETTINGS_MAX_ROWS is the
	// runaway guard for a forgotten { nullptr, nullptr } row.
	_prefs.begin(SETTINGS_NAMESPACE, false);

	int i = 0;
	for (; i < SETTINGS_MAX_ROWS; i++) {

		if (_table[i].key == nullptr) break;

		if (_table[i].buffer == nullptr) {
			Serial.print("\txx settings: '");
			Serial.print(_table[i].key);
			Serial.println("' has no buffer — row skipped");
			_table[i].key = nullptr;	// disable the row so iteration / find / publish never see it
			continue;
		}

		if (!_key_is_valid(_table[i].key)) {	// _key_is_valid prints its own complaint
			_table[i].key = nullptr;
			continue;
		}

		// duplicate key?  first one wins, later ones are skipped.
		bool duplicate = false;
		for (int j = 0; j < i; j++) {
			if (_table[j].key != nullptr && strcmp(_table[j].key, _table[i].key) == 0) {
				Serial.print("\txx settings: duplicate key '");
				Serial.print(_table[i].key);
				Serial.println("' — row skipped");
				duplicate = true;
				break;
			}
		}
		if (duplicate) {
			_table[i].key = nullptr;
			continue;
		}

		// load stored value, or persist the compiled default (whatever
		// the user put in the buffer's initializer) on first boot.
		if (_prefs.isKey(_table[i].key)) {

			_prefs.getString(_table[i].key, _table[i].buffer, SETTINGS_BUF_SIZE);

			Serial.print("\t.. '");
			Serial.print(_table[i].key);
			Serial.print("' loaded from flash: ");
			Serial.println(_table[i].buffer);

		} else {

			_prefs.putString(_table[i].key, _table[i].buffer);

			Serial.print("\t.. '");
			Serial.print(_table[i].key);
			Serial.print("' using default: ");
			Serial.println(_table[i].buffer);
		}
	}

	_prefs.end();

	if (i == SETTINGS_MAX_ROWS) {
		Serial.println("\txx settings: hit SETTINGS_MAX_ROWS — is the { nullptr, nullptr } row missing from your table?");
	}

	_count = i;

	// Run the user's conversion function once, so their typed variables
	// are populated from the (possibly flash-loaded) buffers before
	// loop() ever runs.
	if (_on_change) _on_change();

	Serial.println("-- settings.begin() complete\n");
}



// ============================================================
//  SET — apply a remote update (called by the controller)
// ============================================================

bool Settings::set(const char * key, const char * value) {

	int index = _find(key);

	if (index < 0) {
		Serial.print("\txx settings: unknown key '");
		Serial.print(key);
		Serial.println("' — update refused");
		return false;
	}

	if (strlen(value) >= SETTINGS_BUF_SIZE) {
		Serial.print("\txx settings: value for '");
		Serial.print(key);
		Serial.print("' exceeds ");
		Serial.print(SETTINGS_BUF_SIZE - 1);
		Serial.println(" chars — update refused");
		return false;
	}

	strcpy(_table[index].buffer, value);

	_prefs.begin(SETTINGS_NAMESPACE, false);
	_prefs.putString(key, value);
	_prefs.end();

	Serial.print("\t.. setting updated '");
	Serial.print(key);
	Serial.print("' = ");
	Serial.println(value);

	if (_on_change) _on_change();

	return true;
}



// ============================================================
//  CONVERSIONS — for use inside the user's conversion function
// ============================================================

/*
	The plain flavors mirror atof/atoi: garbage in, zero out, but with a
	failure report so the person at the dashboard can see what happened.

	The fallback flavors return your fallback instead of zero — the
	friendlier choice for values where zero is dangerous.

	A parse "fails" when the buffer doesn't begin with a number.  Trailing
	junk ("2.5abc") parses as 2.5 — same as atof.
*/

float Settings::convert_to_float(const char * buffer) {
	return convert_to_float(buffer, 0.0f);
}

float Settings::convert_to_float(const char * buffer, float fallback) {
	char * end;
	float value = strtof(buffer, &end);
	if (end == buffer) {						// nothing numeric at all
		_report_failure(buffer, "not a number");
		return fallback;
	}
	return value;
}

int Settings::convert_to_int(const char * buffer) {
	return convert_to_int(buffer, 0);
}

int Settings::convert_to_int(const char * buffer, int fallback) {
	char * end;
	long value = strtol(buffer, &end, 10);
	if (end == buffer) {
		_report_failure(buffer, "not a number");
		return fallback;
	}
	return (int)value;
}

bool Settings::convert_to_bool(const char * buffer) {
	return (
		strcmp(buffer, "1")    == 0 ||
		strcmp(buffer, "true") == 0 ||
		strcmp(buffer, "on")   == 0
	);
}



// ============================================================
//  FAILURE REPORTING
// ============================================================

void Settings::set_failure_handler(SettingsFailureHandler h) {
	_on_failure = h;
}


// Build a human-readable line and hand it to whatever the controller
// injected.  Settings never touches MQTT itself.
void Settings::_report_failure(const char * buffer, const char * detail) {

	const char * key = _key_for_buffer(buffer);		// "?" if the buffer isn't in the table

	char text[96];
	snprintf(text, sizeof(text), "setting '%s': %s ('%s')", key, detail, buffer);

	Serial.print("\txx ");
	Serial.println(text);

	if (_on_failure) _on_failure(text);
}


// Reverse lookup: which row owns this buffer?  Lets the conversion
// helpers name the setting in failure messages without the user ever
// passing a key.
const char * Settings::_key_for_buffer(const char * buffer) {
	for (int i = 0; i < _count; i++) {
		if (_table[i].key != nullptr && _table[i].buffer == buffer) return _table[i].key;
	}
	return "?";
}



// ============================================================
//  UTILITY
// ============================================================

// Number of table rows walked at begin().  Disabled rows (bad key,
// duplicate, missing buffer) are still counted but report an empty key —
// iterate with key_at() and skip empty keys.
int Settings::count() {
	return _count;
}


const char * Settings::key_at(int index) {
	if (index < 0 || index >= _count || _table[index].key == nullptr) return "";
	return _table[index].key;
}


const char * Settings::value_at(int index) {
	if (index < 0 || index >= _count || _table[index].buffer == nullptr) return "";
	return _table[index].buffer;
}


bool Settings::has(const char * key) {
	return _find(key) >= 0;
}


void Settings::print() {

	Serial.println("\n\t== Settings ==");

	for (int i = 0; i < _count; i++) {
		if (_table[i].key == nullptr || _table[i].buffer == nullptr) continue;
		Serial.print("\t   ");
		Serial.print(_table[i].key);
		Serial.print(" = ");
		Serial.println(_table[i].buffer);
	}

	Serial.println("\t=================\n");
}



// ============================================================
//  PRIVATE
// ============================================================

int Settings::_find(const char * key) {
	for (int i = 0; i < _count; i++) {
		if (_table[i].key != nullptr && strcmp(_table[i].key, key) == 0) return i;
	}
	return -1;
}


// Keys ride in MQTT topics and land in NVS, so both impose rules:
//   - 15 chars max (NVS)
//   - no '/', '+', '#' (MQTT topic structure), no spaces
// Complaints are loud and happen at boot, while the user is watching serial.
bool Settings::_key_is_valid(const char * key) {

	if (strlen(key) > SETTINGS_MAX_KEY_LEN - 1) {
		Serial.print("\txx settings: key '");
		Serial.print(key);
		Serial.print("' exceeds ");
		Serial.print(SETTINGS_MAX_KEY_LEN - 1);
		Serial.println(" chars — row skipped");
		return false;
	}

	if (strpbrk(key, "/+# ") != nullptr) {
		Serial.print("\txx settings: key '");
		Serial.print(key);
		Serial.println("' contains an illegal character (/ + # or space) — row skipped");
		return false;
	}

	return true;
}