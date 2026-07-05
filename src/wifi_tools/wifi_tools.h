

#pragma once

#include <WiFi.h>

#define RECONNECT_INTERVAL 10000
#define STATUS_LOG_INTERVAL 1000

#ifndef WIFI_SSID_LEN
    #define WIFI_SSID_LEN 33   // 32-char SSID max + null terminator
#endif
#ifndef WIFI_PASS_LEN
    #define WIFI_PASS_LEN 64   // 63-char WPA2 passphrase max + null terminator
#endif


class WiFi_Tools {

    public:

        WiFi_Tools();

        void begin(const char *, const char *);

        // returns true if a reconnect attempt was actually made on this call
        // (the call is internally timer-gated so most invocations are no-ops).
        // ignore the return value if you don't care; the controller uses it
        // to count retries.
        bool reconnect();

        void log_events();
        void log_status();

        bool is_connected = false;

    private:

        bool _should_reconnect = true;
        bool _first_disconnect = true;
        bool _event_logging_enabled = false;
		bool _is_connecting = false;			

        unsigned long _reconnect_timer;
        unsigned long _status_timer;

        static void _event_handler(WiFiEvent_t, WiFiEventInfo_t);
        void _log_event(WiFiEvent_t, WiFiEventInfo_t);

};

extern WiFi_Tools wifi_tools;