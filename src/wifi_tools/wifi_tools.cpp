

#include "wifi_tools/wifi_tools.h"
#include "Arduino.h"

WiFi_Tools::WiFi_Tools() {}

WiFi_Tools wifi_tools;

void WiFi_Tools::begin(const char * ssid, const char * pass) {
	WiFi.setAutoReconnect(false);
	WiFi.persistent(false);
	WiFi.onEvent(_event_handler);
	Serial.println("\n\twifi connecting...");
	_reconnect_timer = millis();
	_is_connecting = true;                            
	WiFi.begin(ssid, pass);
}

bool WiFi_Tools::reconnect() {

	if (_is_connecting) return false;                 

	if ((millis() - _reconnect_timer > RECONNECT_INTERVAL) && _should_reconnect) {
		Serial.println("\n\treconnecting...\n");
		_is_connecting = true;                        
		WiFi.reconnect();
		_reconnect_timer = millis();
		return true;
	}
	return false;
}

void WiFi_Tools::_event_handler(WiFiEvent_t event, WiFiEventInfo_t info) {

	if (wifi_tools._event_logging_enabled) wifi_tools._log_event(event, info);	
	
	if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
		// state update only — keep this handler tiny.  the controller
		// observes is_connected on each loop tick and does any bookkeeping
		// (logging, counter increments) on the main task, where heap work
		// is safe.  the WiFi event task is not the place for NVS writes.
		wifi_tools.is_connected = false;
		wifi_tools._is_connecting = false;            
		bool user_disconnected = info.wifi_sta_disconnected.reason == WIFI_REASON_ASSOC_LEAVE;
		wifi_tools._should_reconnect = !(user_disconnected || wifi_tools._first_disconnect);
		wifi_tools._first_disconnect = false;
	}

	if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
		if (!wifi_tools.is_connected) Serial.println("\twifi connected...\n");
		wifi_tools.is_connected = true;
		wifi_tools._is_connecting = false;            
	}

}


void WiFi_Tools::log_events() {
	_event_logging_enabled = true;
}