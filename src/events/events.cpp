

#include "events/events.h"

#include "Arduino.h"

#include "Preferences.h"
static Preferences _prefs; 

Events::Events() {}

Events events;


void Events::load_all() {

	if (!_prefs.begin("events", false)) {
		Serial.println("\tfailed to open events");
		return;
	}

	// missing key just returns 0 — no preemptive write, no default seeding.
	starts          = _prefs.getUInt("starts",          0);
	wifi_brown_outs = _prefs.getUInt("wifi_brown_outs", 0);
	wifi_retries    = _prefs.getUInt("wifi_retries",    0);
	wifi_drops      = _prefs.getUInt("wifi_drops",      0);
	mqtt_reconnects    = _prefs.getUInt("mqtt_reconnects",    0);
	mqtt_drops      = _prefs.getUInt("mqtt_drops",      0);

	_prefs.end();

	// Serial.printf("\tevents loaded: starts=%u brown_outs=%u wifi_retries=%u wifi_drops=%u mqtt_reconnects=%u mqtt_drops=%u\n",
	// 	starts, wifi_brown_outs, wifi_retries, wifi_drops, mqtt_reconnects, mqtt_drops);
}


void Events::increment(const char * key) {
	unsigned int count = 0;
	bool got_one = false;

	_prefs.begin("events");

	if (strcmp(key, "starts") == 0)          { count = ++starts;          got_one = true; }
	if (strcmp(key, "wifi_brown_outs") == 0) { count = ++wifi_brown_outs; got_one = true; }
	if (strcmp(key, "wifi_retries") == 0)    { count = ++wifi_retries;    got_one = true; }
	if (strcmp(key, "wifi_drops") == 0)      { count = ++wifi_drops;      got_one = true; }
	if (strcmp(key, "mqtt_reconnects") == 0)    { count = ++mqtt_reconnects;    got_one = true; }
	if (strcmp(key, "mqtt_drops") == 0)      { count = ++mqtt_drops;      got_one = true; }

	if (got_one) _prefs.putUInt(key, count);
	_prefs.end();
}


void Events::increment_ram(const char * key) {

	// RAM-only bump for events that can fire every few seconds (wifi retries
	// while offline, drops on a flapping AP).  A device stuck offline
	// overnight would otherwise putUInt the same NVS key thousands of times —
	// wear leveling or not, that's needless flash wear.  the pending value is
	// written by flush() on the next clean reconnect (or before a restart).
	// trade-off: if the device reboots before ever reconnecting, the pending
	// counts are lost.  acceptable — these are diagnostics, not billing.

	if (strcmp(key, "wifi_retries") == 0) { ++wifi_retries; _wifi_retries_dirty = true; return; }
	if (strcmp(key, "wifi_drops")   == 0) { ++wifi_drops;   _wifi_drops_dirty   = true; return; }

	// key isn't one of the high-frequency ones — fall back to the
	// write-through path so callers can't silently lose an event.
	increment(key);
}


void Events::flush() {

	if (!_wifi_retries_dirty && !_wifi_drops_dirty) return;		// nothing pending — no NVS touch at all

	_prefs.begin("events");
	if (_wifi_retries_dirty) { _prefs.putUInt("wifi_retries", wifi_retries); _wifi_retries_dirty = false; }
	if (_wifi_drops_dirty)   { _prefs.putUInt("wifi_drops",   wifi_drops);   _wifi_drops_dirty   = false; }
	_prefs.end();
}



void Events::reset_all() {

	Serial.println("\tresetting events");

	starts          = 0;
	wifi_brown_outs = 0;
	wifi_retries    = 0;
	wifi_drops      = 0;
	mqtt_reconnects    = 0;
	mqtt_drops      = 0;

	_prefs.begin("events");
	_prefs.clear();          // wipes namespace; next increment recreates keys
	_prefs.end();

	// drop any pending RAM-only increments too — otherwise a later flush()
	// would write stale pre-reset counts right back into NVS.
	_wifi_retries_dirty = false;
	_wifi_drops_dirty   = false;
}