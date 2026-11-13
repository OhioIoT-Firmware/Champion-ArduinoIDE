

#pragma once

#ifdef ALLOW_INSECURE_MQTT
    #include <WiFiClient.h>
#else
    #include <WiFiClientSecure.h>       // TLS
#endif

#include <PubSubClient.h>
#include <stddef.h>

#define MQTT_RETRY_INTERVAL 3000    // first wait after a failed connect, in ms. later waits double (see mqtt.cpp)
#define MQTT_CLIENT_ID_LEN 24

// Expanded topics (username + device id + the rest) are built into this.
// Anything longer is dropped instead of overflowing the buffer.
#define MQTT_TOPIC_BUF 180
// PubSubClient's own packet buffer is 256 bytes. Anything larger — the topic,
// the message, and about 7 bytes of MQTT header — is dropped, and the library
// does not say so. 1024 leaves roughly 1000 bytes for the topic plus the body,
// which covers a normal JSON payload (a few dozen short fields) after a
// realistic topic. The extra RAM is under a kilobyte; an ESP32 has a few
// hundred kilobytes of heap, and WiFi plus TLS already use much more than this.
//
// One knob. A build flag wins over the number below:
//   PlatformIO:  -D MQTT_BUFFER_SIZE=2048
//   Arduino IDE: the same -D line in build_opt.h next to the sketch,
//                or change the number here.
// It has to be set while this module is compiled. A #define in the sketch
// alone does not reach mqtt.cpp.
#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 1024
#endif

using MessageHandler    = void (*)(char *, char *);   // user's raw handler (terminal)
using StdMessageHandler = bool (*)(char *, char *);   // framework handler; returns true if it claimed the message

class Mqtt {

    public:

        Mqtt();

        // connect
        void setup(const char *, int, const char *, const char *, const char *, const char *);

        void maintain();

        void report_disconnect();

        bool is_connected = false;

        // publish
        void publish(const char *, const char *);
        void publish(const char *, float);
        void publish(const char *, int);
		void publish(const char *, bool);
        void publish(const char *);

        void publish_namespaces();

        // subscribe — four sources, all nullptr-terminated arrays of string
        // literals.  nothing is copied; topics are (re)built and subscribed
        // at every connect inside _subscribe_to_all().
        //
        //   set_subscriptions         — user's app-specific topics (main.cpp)
        //   set_std_subscriptions     — framework's standard topics (controller)
        //   set_command_namespaces    — group names; each expands to
        //                               ~/{group}/command/+  and  ~/{group}/command/+/+
        //   set_settings_namespaces   — group names; each expands to
        //                               ~/{group}/settings/+
        void set_subscriptions(const char **);
        void set_std_subscriptions(const char **);
        void set_command_namespaces(const char **);
        void set_settings_namespaces(const char **);

        // handle
        //   set_callback     — user's raw handler (main.cpp). void; terminal.
        //   set_std_callback — framework's router (controller). returns bool;
        //                      if it returns false, the raw user handler runs next.
        void set_callback(MessageHandler);
        void set_std_callback(StdMessageHandler);

    private:

        #ifdef ALLOW_INSECURE_MQTT
            WiFiClient _wifi_client;
        #else
            WiFiClientSecure _wifi_client;
        #endif
                
        
        PubSubClient _mqtt_client;

        // Writes the expanded topic into output. Returns false (and leaves
        // output empty) when the topic does not fit.
        bool _build_topic(const char * topic, char * output, size_t output_len);
        
        // connect — exponential backoff between attempts. the wait is
        // counted with millis() so loop() keeps running between tries.
        unsigned long _retry_timer = 0;
        unsigned long _retry_base_ms = MQTT_RETRY_INTERVAL;   // next wait before jitter
        unsigned long _retry_wait_ms = 0;                     // jittered wait we're counting
        bool _retry_waiting = false;
        bool _is_first_connect = true;

        char _device_id[MQTT_CLIENT_ID_LEN] = {};
        char _username[33] = {};
        char _password[33] = {};
        void _schedule_retry();
        void _reset_retry();

        char _device_id[MQTT_CLIENT_ID_LEN];
        char _username[33];
        char _password[33];

        // publish
        void _publish(const char *, const char *);

        // subscribe — pointer arrays only; the strings live in flash.
        const char ** _subscription_list     = nullptr;   // user's list
        const char ** _std_subscription_list = nullptr;   // framework's list
        const char ** _command_namespaces    = nullptr;   // group names
        const char ** _settings_namespaces   = nullptr;   // group names

        void _subscribe_to_all();
        void _subscribe_one(const char * topic);

        // handle
        MessageHandler    _stored_handler = nullptr;   // user's raw handler
        StdMessageHandler _std_handler    = nullptr;   // framework router
        static void _callback_wrapper(char *, byte *, unsigned int);

};

extern Mqtt mqtt;