

/*
	NOTE: these SDKs deliberately ignore error handling.  someone else's code is hard enough to read, let alone recognizing key logic
	from boilerplate error handling.  Error handling is an important part of firmware, but I left it out here so you can focus on getting 
	your arms around the system logic.  Once you are comfortable, wrap your code in all of the error handling that you feel is appropriate.
*/

#include "Arduino.h"

#include "_controller/controller.h"

#include "_certificates/ca_cert.h"
#include "device_id/device_id.h"
#include "events/events.h"
#include "json/json.h"
#include "messages/messages.h"
#include "metrics/metrics.h"
#include "monitor/monitor.h"
#include "mqtt/mqtt.h"
#include "ota/ota.h"
#include "provisioner/provisioner.h"
#include "settings/settings.h"
#include "wifi_tools/wifi_tools.h"



/* 
To override the CA_CERT to point your device to a different MQTT broker, create your own CA cert variable and define CA_CERT to point to that variable name: 
	const char YOUR_CERT[] = "-----BEGIN CERTIFICATE-----\n"
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n"
		"-----END CERTIFICATE-----\n";

	#define CA_CERT YOUR_CERT
*/
#ifndef CA_CERT
    #define CA_CERT OHIOIOT_CA_CERT
#endif


// Override the following with build flags in platformio if you want something different

#ifndef MQTT_HOST
    #define MQTT_HOST "test-mqtt.ohioiot.com"
#endif

#ifndef MQTT_PORT
    #define MQTT_PORT 8883
#endif

char wifi_ssid[WIFI_SSID_LEN];	// are these LEN defined anywhere?
char wifi_pass[WIFI_PASS_LEN];


Controller::Controller() {}

Controller controller;


// Standard subscription list — installed automatically whenever the
// controller is in use.  These guarantee every device is reachable by its
// device_id and through the broadcast channel, no matter what the user
// configures in main.cpp.
//
//   ~/~/...           — per-device addressing (using this device's id)
//   ~/broadcast/...   — fleet-wide broadcasts (idempotent / notification)
//
// Group-level addressing (~/{group}/...) is opt-in via main.cpp:
//   messages.add_command_namespace("lights");
//   messages.add_settings_namespace("kitchen");
//
// OTA is per-device only.  The cloud addresses each device individually
// at ~/~/ota/{group_name}, so there is no group OTA subscription here.
//
// NOTE: device-side responses go under ~/~/response/...  Nothing here
// subscribes under that prefix, so the broker can't echo our own responses
// back into our message dispatcher.  Keep it that way.
//
// NOTE: settings echoes go OUT on ~/~/settings (three segments).  The
// inbound subscription is ~/~/settings/+ (four segments), so the broker
// can't feed our own echoes back into the settings route either.
static const char * std_subscription_list[] = {
	"~/~/settings/+",		// champion only
	"~/~/command/+",		// scaler and champion
	"~/~/ota/+",			// champion only — start an OTA
	"~/~/ota_abort",		// champion only — cancel an in-flight OTA
	"~/~/ota_accept",		// champion only — accept pending firmware
	"~/~/ota_rollback",		// champion only — revert to previous image
	"~/~/clear_counters",	// scaler and champion
	"~/~/restart",			// scaler and champion
	"~/broadcast/+",		// delete??
	nullptr
};


// Publish one setting as a single-key JSON object on the settings report
// topic.  This is what feeds the twin (and therefore the device dashboard):
// the boot snapshot uses it for every row, and every remote update — applied
// OR refused — triggers one echo so the dashboard always shows the value the
// device actually holds.
static void _publish_one_setting(const char * key, const char * value) {
	char buffer[128] = "";
	json.one_key(buffer, key, value);
	mqtt.publish("~/~/settings", buffer);
}


void Controller::setup(const char * mqtt_user, const char * mqtt_pass) {

	// if the user doesn't provide the wifi_ssid and wifi_pass (maybe they don't know where the device is going)
	// then we trigger the provisioner on first boot.
    if (!provisioner.creds_already_exist(wifi_ssid, wifi_pass)) {
		provisioner.run_provisioner(wifi_ssid, wifi_pass);
		provisioner.store_creds(wifi_ssid, wifi_pass);
	}

	setup(wifi_ssid, wifi_pass, mqtt_user, mqtt_pass);

}



void Controller::setup(const char * wifi_ssid, const char * wifi_pass, const char * mqtt_user, const char * mqtt_pass) {

    char deviceID[9];
    device_id.get_or_set(deviceID);

	wifi_tools.begin(wifi_ssid, wifi_pass);

	Serial.print("\tusing MQTT host: ");	Serial.println(MQTT_HOST);
	Serial.print("\tusing MQTT port: ");	Serial.println(MQTT_PORT);

	mqtt.setup(MQTT_HOST, MQTT_PORT, mqtt_user, mqtt_pass, deviceID, CA_CERT); 

	ota.set_ca_cert(CA_CERT);   // controller owns the cert; inject same root CA into OTA
	ota.initialize();           // start the post-boot rollback clock (see ota.cpp)

	monitor.setup(8000);

	// brownout = power problem during last session.  reset_reason was
	// captured in monitor.setup -> metrics.check_all, so we can check it now.
	if (strcmp(metrics.reset_reason, "BROWNOUT") == 0) {
		events.increment("wifi_brown_outs");
	}

	// install the framework's standard subscription list.  user's own
	// subscriptions and any add_command_namespace calls go on top of this.
	mqtt.set_std_subscriptions(std_subscription_list);

	// code is "" when the action was sent untracked (today's existing
	// behavior, unchanged) — only publish a response when a tracking code
	// was actually supplied.  Topic is .../response/clear_counters/{code}/success,
	// deliberately not .../response/command/..., so it can never be picked
	// up by the generic command-response listener (or vice versa).
	messages.set_clear_counter_handler([](const char* code) {
		events.reset_all();   // wipes everything including starts and brown_outs
		monitor.refresh_counters();

		// if (code[0]) {
		// 	char response_topic[100] = "~/~/response/clear_counters/";
		// 	strcat(response_topic, code);
		// 	strcat(response_topic, "/success");
		// 	mqtt.publish(response_topic, "");
		// }
	});

	// Restart the chip.  If a tracking code was supplied, ack it first —
	// "received" only, never "success": the device reboots immediately
	// after, so there's no point at which a "success" could still be sent.
	// The short delay gives that publish a chance to actually flush over
	// the wire before ESP.restart() yanks everything down hard. No need
	// to disconnect mqtt cleanly either way — we publish at QoS 0 and the
	// broker will time the session out on its own. The "starts" counter
	// will also increment on the next boot via monitor.setup, which is a
	// second, independent confirmation the cloud can check.
	messages.set_restart_handler([](const char* code) {

		// if (code[0]) {
		// 	char response_topic[100] = "~/~/response/restart/";
		// 	strcat(response_topic, code);
		// 	mqtt.publish(response_topic, "");	// 5-segment topic -> implied status "received"
		// 	delay(200);							// let the ack actually go out before we reboot
		// }

		Serial.println("\n\t### RESTARTING ###\n");
		delay(100);
		ESP.restart();
	});

	// provisioner.set_pin(15, 2000);		// uncomment if you are assigning a provisioning pin

	// SETTINGS ---------------------------------------------------------
	//
	// Remote update, quarterbacked.  The messages router hands us the key
	// (from the topic ~/~/settings/{key}) and the raw value (the payload).
	// We ask the settings module to apply it — it refuses unknown keys and
	// oversized values — and then, applied or refused, we echo the value
	// the device NOW HOLDS.  The echo is what updates the twin: on success
	// the dashboard's pending pulse clears on the new value; on refusal
	// the input snaps back to the old one.  The dashboard never lies.
	//
	// Note the module boundaries: settings knows nothing about MQTT, and
	// messages knows nothing about settings.  This lambda is the only
	// place the two meet.
	messages.set_settings_handler([](const char* key, const char* value) {

		settings.set(key, value);		// may refuse — that's fine, we echo either way

		if (settings.has(key)) {
			int index = -1;
			for (int i = 0; i < settings.count(); i++) {
				if (strcmp(settings.key_at(i), key) == 0) { index = i; break; }
			}
			if (index >= 0) _publish_one_setting(key, settings.value_at(index));
		}
		// unknown key: nothing held, nothing to echo — the refusal was
		// already logged on serial by settings.set().

		return true;	// settings topics are framework-owned; always claimed
	});

	// Conversion failures ("threshold: not a number ('abc')") get posted
	// to the app's Messages inbox via the device message channel.  Settings
	// itself never touches MQTT — it just calls whatever we inject here.
	// Throttled so a bad value sitting in a hot loop can't flood the inbox.
	settings.set_failure_handler([](const char* text) {
		static unsigned long last_report = 0;
		if (millis() - last_report < 3000) return;
		last_report = millis();
		mqtt.publish("~/~/messages/post", text);
	});

	// OTA handler signature:
	//   group_name comes from the topic (~/~/ota/{group_name})
	//   auth_key   comes from the payload (raw string, not JSON)
	messages.set_ota_handler([](const char* group_name, const char* auth_key) {
		ota.update_firmware(group_name, auth_key);
	});

	// Cancel an in-flight download (manual abort from the messenger).
	messages.set_ota_abort_handler([]() {
		ota.abort();
	});

	// Manually accept pending firmware.  This is the *required* path when
	// REQUIRE_MANUAL_FIRMWARE_ACCEPT is set; in the default build it's harmless
	// (the image is usually already accepted on MQTT connect).
	messages.set_ota_accept_handler([]() {
		ota.mark_firmware_as_valid();
		mqtt.publish("~/~/response/ota/accepted", ota.get_ota_group());
	});

	// Manually roll back to the previous image.  Reboots immediately; the next
	// boot reports rolled_back via _report_boot_ota_outcome.
	messages.set_ota_rollback_handler([]() {
		ota.rollback_and_reboot();
	});

	mqtt.set_std_callback(messages.main_handler);	// router gets first look; user's raw handler runs for anything it doesn't claim

}


void Controller::loop() {

	// do something that doesn't require MQTT

	if (wifi_tools.is_connected) {

		mqtt.maintain();

		if (mqtt.is_connected) {

			// if (!monitor.boot_packet_sent) monitor.send_all();	// we don't send this in setup, because mqtt is not connected yet
			monitor.send_heartbeat();

			_report_boot_ota_outcome();   // once-only; no-op after first run

			_publish_settings_snapshot(); // once-only; populates the dashboard's settings section

			// do something that requires MQTT

		}


	} else {

		if (wifi_tools.reconnect()) {
			events.increment("wifi_retries");
		}
		mqtt.report_disconnect();

	}

	static bool wifi_was_connected = false;
	static bool mqtt_was_connected = false;
	static bool mqtt_has_connected = false;   // first connect after boot isn't a reconnect

	if (wifi_was_connected && !wifi_tools.is_connected) {
		Serial.println("\n\twifi disconnected...\n");
		events.increment("wifi_drops");
	}

	// mqtt_drops      = falling edge of is_connected
	// mqtt_reconnects = rising edge, excluding the very first connect
	if (mqtt_was_connected && !mqtt.is_connected) {
		events.increment("mqtt_drops");
	}
	if (!mqtt_was_connected && mqtt.is_connected) {
		if (mqtt_has_connected) events.increment("mqtt_reconnects");
		mqtt_has_connected = true;
	}
	wifi_was_connected = wifi_tools.is_connected;
	mqtt_was_connected = mqtt.is_connected;
	
	if (provisioner.pin_is_pulled()) {
		Serial.println("pulled");
		mqtt.report_disconnect();
		provisioner.run_provisioner(wifi_ssid, wifi_pass);
		provisioner.store_creds(wifi_ssid, wifi_pass);
		wifi_tools.begin(wifi_ssid, wifi_pass);
	}

	// OTA is non-blocking: check() advances any active download by one chunk,
	// and _handle_ota_reporting watches the resulting state and bridges it
	// to MQTT.  Both are safe to call every loop iteration; they no-op when
	// nothing is happening.
	ota.check();

	_handle_ota_reporting();

	// Post-OTA safety net: if a freshly-booted image hasn't been accepted
	// within ROLLBACK_THRESHOLD (auto on MQTT connect, or via an explicit
	// accept command), revert to the previous image.  Runs unconditionally —
	// it MUST fire even when MQTT never connects, since "can't reach the
	// broker" is exactly the failure it's guarding against.  Internally a
	// one-shot: it evaluates once, when the threshold is first crossed.
	ota.rollback_if_not_verified_within_time();

}


// First-connect settings snapshot.  Publishes every registered setting as
// its own single-key message — the twin merges per-key, so many small
// messages behave identically to one big one and we never fight the MQTT
// client's payload buffer.  This is what makes the settings section appear
// on the device dashboard without the user doing anything.
//
// One-shot per boot, same pattern as _report_boot_ota_outcome.  If the user
// never registered a table, we say so once (silence should never be
// ambiguous) and go idle.
void Controller::_publish_settings_snapshot() {

	if (_settings_snapshot_sent) return;

	if (settings.count() == 0) {
		Serial.println("\t.. no settings table registered — settings features idle");
		_settings_snapshot_sent = true;
		return;
	}

	Serial.println("\t.. publishing settings snapshot");

	for (int i = 0; i < settings.count(); i++) {
		if (settings.key_at(i)[0] == '\0') continue;	// disabled row (bad key / duplicate)
		_publish_one_setting(settings.key_at(i), settings.value_at(i));
	}

	_settings_snapshot_sent = true;
}
