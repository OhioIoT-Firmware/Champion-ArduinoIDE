

#include "wifi_tools/wifi_tools.h"
#include "Arduino.h"

WiFi_Tools::WiFi_Tools() {}

WiFi_Tools wifi_tools;

void WiFi_Tools::begin(const char * ssid, const char * pass) {
	// Off on purpose — core 3.x would retry in a tight loop. See reconnect().
	WiFi.setAutoReconnect(false);
	WiFi.persistent(false);
	WiFi.onEvent(_event_handler);

	strlcpy(_ssid, ssid, WIFI_SSID_LEN);
	strlcpy(_pass, pass, WIFI_PASS_LEN);
	_backoff_ms = RECONNECT_INTERVAL;
	_should_reconnect = false;

	Serial.println("\n\twifi connecting...");
	WiFi.begin(_ssid, _pass);
}

bool WiFi_Tools::reconnect() {

	reconnect_attempted = false;

	if (!_should_reconnect) return false;
	if (millis() - _reconnect_timer < _backoff_ms) return false;

	Serial.println("\n\treconnecting...\n");

	// WiFi.reconnect() is not the same on every core. On 3.x it does nothing
	// while a join is still in progress ("sta is connecting"), and ASSOC_FAIL
	// is a dead end the core will not retry. disconnect() then begin() starts
	// a fresh join on both 2.x and 3.x. The core already retries the first
	// disconnect after boot; we take over after that. The 10ms pause just
	// lets the cancel finish — the wait between tries is the timer above
	// (2s, then 4, 8, 16... up to 60s), so loop() stays free the rest of the time.
	_should_reconnect = false;
	WiFi.disconnect(false);
	delay(10);
	_should_reconnect = false;   // the cancel's own disconnect event can re-arm this

	if (_backoff_ms < RECONNECT_MAX) {
		_backoff_ms *= 2;
		if (_backoff_ms > RECONNECT_MAX) _backoff_ms = RECONNECT_MAX;
	}

	WiFi.begin(_ssid, _pass);

	reconnect_attempted = true;
	return true;
}

void WiFi_Tools::_event_handler(WiFiEvent_t event, WiFiEventInfo_t info) {

	if (wifi_tools._event_logging_enabled) wifi_tools._log_event(event, info);	
	
	if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
		// state update only — keep this handler tiny.  the controller
		// observes is_connected on each loop tick and does any bookkeeping
		// (logging, counter increments) on the main task, where heap work
		// is safe.  the WiFi event task is not the place for NVS writes.
		wifi_tools.is_connected = false;

		// ASSOC_LEAVE means we (or the user) disconnected on purpose.
		if (info.wifi_sta_disconnected.reason == WIFI_REASON_ASSOC_LEAVE) {
			wifi_tools._should_reconnect = false;
			return;
		}

		// The core retries the first disconnect after boot once, by itself.
		// Let that one finish. After it, arm the timer in reconnect().
		if (wifi_tools._first_disconnect) {
			wifi_tools._first_disconnect = false;
			return;
		}

		if (!wifi_tools._should_reconnect) {
			wifi_tools._reconnect_timer = millis();
			wifi_tools._should_reconnect = true;
		}
	}

	if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
		if (!wifi_tools.is_connected) Serial.println("\twifi connected...\n");
		wifi_tools.is_connected = true;
		wifi_tools._should_reconnect = false;
		wifi_tools._backoff_ms = RECONNECT_INTERVAL;
	}

}


void WiFi_Tools::log_events() {
	_event_logging_enabled = true;
}