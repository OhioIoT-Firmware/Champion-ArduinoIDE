

#pragma once

#include "wifi_tools/wifi_tools.h"

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

#ifndef AP_SSID
	#define AP_SSID "DEVICE PROVISIONING"
#endif


class Provisioner {
    public:
        Provisioner();

        void set_pin(int, unsigned long = 3000);
        bool pin_is_pulled();

        bool creds_already_exist(char *, char *);
        void run_provisioner(char * ssid, char * pass);
        void store_creds(char *, char *);

        void clear_creds();

    private:
        WebServer _web_server{80};
        DNSServer _dns_server;
        bool _provisioning_complete = false;
        char _ssid[WIFI_SSID_LEN];
        char _pass[WIFI_PASS_LEN];
        void _start_provisioner();
        void _wait_for_completion();
        
        void _set_base_routes();
        void _set_portal_routes();
        void _set_other_routes();

        int _provisioning_pin;
        unsigned long _provisioning_timeout;
        unsigned long _pin_timer;
        bool _timer_is_on = false;


};

extern Provisioner provisioner;