

#include "mqtt/mqtt.h"
#include "Arduino.h"


// Build a namespace topic. Returns false (and prints a warning) when the
// name is too long to fit, so we never subscribe to a chopped topic.
static bool _topic_fits(char * topic, size_t topic_len, const char * fmt, const char * name) {
    int wrote = snprintf(topic, topic_len, fmt, name ? name : "");
    if (wrote < 0 || (size_t)wrote >= topic_len) {
        Serial.print("\txx namespace topic too long — skipped: ");
        Serial.println(name ? name : "");
        return false;
    }
    return true;
}


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
    char topic_build[MQTT_TOPIC_BUF];
    if (!_build_topic(topic, topic_build, sizeof(topic_build))) return;

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
            if (_topic_fits(topic, sizeof(topic), "~/%s/command/+", _command_namespaces[i])) {
                _subscribe_one(topic);
            }
            if (_topic_fits(topic, sizeof(topic), "~/%s/command/+/+", _command_namespaces[i])) {
                _subscribe_one(topic);
            }
        }
    }

    // 4. settings namespaces — one topic per group:  ~/{group}/settings/+
    if (_settings_namespaces != nullptr) {
        for (int i=0; _settings_namespaces[i] != nullptr; i++) {
            if (_topic_fits(topic, sizeof(topic), "~/%s/settings/+", _settings_namespaces[i])) {
                _subscribe_one(topic);
            }
        }
    }

    Serial.println();   // for aesthetics
}