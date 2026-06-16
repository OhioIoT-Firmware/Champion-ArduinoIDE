

#include "mqtt/mqtt.h"
#include "Arduino.h"


/*
    All four subscription sources are nullptr-terminated arrays of string
    literals supplied by the caller.  We store only the array pointers —
    the strings themselves live in flash, so nothing is copied and there
    is no ceiling on how many entries an array may hold.

    Namespace topics are *built fresh* inside _subscribe_to_all() on every
    connect ("build on connect").  The assembled string lives in a stack
    temp just long enough to hand to the client, exactly like the literal
    topics — so no permanent storage is needed for constructed topics.
*/

void Mqtt::set_subscriptions(const char ** topics) {
    _subscription_list = topics;
}

void Mqtt::set_std_subscriptions(const char ** topics) {
    _std_subscription_list = topics;
}

void Mqtt::set_command_namespaces(const char ** groups) {
    _command_namespaces = groups;
}

void Mqtt::set_settings_namespaces(const char ** groups) {
    _settings_namespaces = groups;
}


// helper — apply the username/device_id template, log it, and subscribe.
void Mqtt::_subscribe_one(const char * topic) {
    char topic_build[100];
    _build_topic(topic, topic_build);

    Serial.print("\t.. subscribing to: ");
    Serial.println(topic_build);

    _mqtt_client.subscribe(topic_build);
}


void Mqtt::_subscribe_to_all() {

    // 1. framework standard subscriptions (set by controller).  always first
    //    so std subs go in even if the user's list isn't installed yet.
    if (_std_subscription_list != nullptr) {
        for (int i=0; _std_subscription_list[i] != nullptr; i++) {
            _subscribe_one(_std_subscription_list[i]);
        }
    }

    // 2. user's app-specific subscriptions (set in main.cpp).
    if (_subscription_list != nullptr) {
        for (int i=0; _subscription_list[i] != nullptr; i++) {
            _subscribe_one(_subscription_list[i]);
        }
    }

    // 3. command namespaces — each group expands to two topics:
    //      ~/{group}/command/{name}        (no tracking code)
    //      ~/{group}/command/{name}/{tag}  (with tracking code)
    char topic[80];
    if (_command_namespaces != nullptr) {
        for (int i=0; _command_namespaces[i] != nullptr; i++) {
            snprintf(topic, sizeof(topic), "~/%s/command/+",   _command_namespaces[i]);
            _subscribe_one(topic);
            snprintf(topic, sizeof(topic), "~/%s/command/+/+", _command_namespaces[i]);
            _subscribe_one(topic);
        }
    }

    // 4. settings namespaces — one topic per group:  ~/{group}/settings/+
    if (_settings_namespaces != nullptr) {
        for (int i=0; _settings_namespaces[i] != nullptr; i++) {
            snprintf(topic, sizeof(topic), "~/%s/settings/+", _settings_namespaces[i]);
            _subscribe_one(topic);
        }
    }

    Serial.println();   // for aesthetics
}