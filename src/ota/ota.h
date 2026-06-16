

#pragma once

#ifndef ROLLBACK_THRESHOLD
	#define ROLLBACK_THRESHOLD 600000
#endif

#ifndef OTA_TIMEOUT
	#define OTA_TIMEOUT 300000
#endif


#ifndef OTA_URL
    #define OTA_URL "https://test-ota.ohioiot.com"
#endif

#include <esp_err.h>

// forward declare so we don't drag lwip into every translation unit
typedef struct esp_http_client_event esp_http_client_event_t;


/*
    Non-blocking OTA download manager.

    Lifecycle:
      1. update_firmware()  — kicks off the download (esp_https_ota_begin) and returns.
      2. check()            — call every loop iteration.  advances the download by
                              one chunk, finalizes the partition on completion, and
                              transitions state to SUCCESS or FAILED.
      3. acknowledge()      — caller invokes after handling SUCCESS/FAILED to reset
                              the state machine back to IDLE.

    This module does not touch MQTT, does not reboot, and does not know what
    transport (if any) is reporting progress to the outside world.  Observers
    poll get_state() and get_percent() and decide what to do.
*/

class OTA {

    public:
        OTA();

        enum State { IDLE, IN_PROGRESS, SUCCESS, FAILED };

        // ota.cpp
        void initialize();
        void set_ca_cert(const char * cert);   // inject OTA-server root CA; call once in setup, before update_firmware
        // Two modes for auth_key:
        //   app-driven    — caller passes auth_key (e.g. from MQTT payload)
        //   device-driven — caller omits it; device uses its own AUTH_KEY
        //                   #defined in credentials.h
        bool  update_firmware(const char * group_name, const char * auth_key = nullptr);
        void  check();             // call from main loop every iteration
        void  acknowledge();       // call after handling SUCCESS/FAILED

        State get_state()   { return _state; }
        int   get_percent() { return _percent; }

        // _rollback.cpp
        void set_ota_pending();
        void mark_firmware_as_valid();
        void rollback_and_reboot();
        void rollback_if_not_verified_within_time();

        // _rollback.cpp — group persistence + boot-time outcome reporting.
        // The group name is captured when an update starts and persisted to
        // NVS so it survives the reboot; on the next boot we report whether
        // the update was accepted or rolled back, tagged with that group.
        void  save_ota_group(const char * group_name);
        const char * get_ota_group();          // returns "" if none stored
        bool  ota_was_pending_on_boot();        // true if we rebooted into a pending update

        // True when the most recent update was rolled back: the bootloader
        // marked the other (non-running) partition ABORTED/INVALID and booted
        // the previous image. Pure partition inspection, no transport.
        bool  was_rolled_back();


    private:

        const char *_ota_server_certificate;

        // _log_partition.cpp
        void _log_ota_status();

        // _rollback.cpp
        unsigned long _rollback_timer;
        unsigned long _rollback_threshold = ROLLBACK_THRESHOLD;
        bool _need_to_check_rollback_once;
        bool _update_is_pending_validation();
        void _mark_firmware_as_valid_if_appropriate();
        bool _update_firmware_http(char* url);
        bool _update_firmware_https(char* url);

        // _http_events.cpp
        static esp_err_t _http_event_handler(esp_http_client_event_t * evt);

        // ota.cpp — download state
        State  _state      = IDLE;
        int    _percent    = 0;
        void * _ota_handle = nullptr;   // actually esp_https_ota_handle_t

        // group name for the in-flight / just-completed update (read back
        // from NVS on boot for outcome reporting)
        char   _ota_group[64] = {0};

};

extern OTA ota;