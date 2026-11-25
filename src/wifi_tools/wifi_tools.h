

#pragma once

#include <WiFi.h>

#define RECONNECT_INTERVAL 10000   // ms between retries
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
        // ignore the return value if you don't care -- the same answer is also
        // published on reconnect_attempted below, which is what the controller
        // reads when it counts retries.
        // fixed 10s wait between retries
        bool reconnect();

        void log_events();
        void log_status();

        bool is_connected = false;

        // mirrors the return of the most recent reconnect() call, so a caller
        // that ignored the return value can still ask after the fact.
        bool reconnect_attempted = false;

    private:

        bool _should_reconnect = false;
        bool _first_disconnect = true;   // the core retries the first disconnect after boot once
        bool _event_logging_enabled = false;

        unsigned long _reconnect_timer = 0;
        unsigned long _status_timer;

        char _ssid[WIFI_SSID_LEN] = "";
        char _pass[WIFI_PASS_LEN] = "";

        static void _event_handler(WiFiEvent_t, WiFiEventInfo_t);
        void _log_event(WiFiEvent_t, WiFiEventInfo_t);

};

extern WiFi_Tools wifi_tools;