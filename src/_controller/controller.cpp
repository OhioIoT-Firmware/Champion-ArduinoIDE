

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
#include "messages/messages.h"
#include "metrics/metrics.h"
#include "monitor/monitor.h"
#include "mqtt/mqtt.h"
#include "ota/ota.h"
#include "provisioner/provisioner.h"
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
static const char * std_subscription_list[] = {
	"~/~/settings/+",		// champion only
	"~/~/command/+",		// scaler and champion
	"~/~/ota/+",			// champion only
	"~/~/clear_counters",	// scaler and champion
	"~/~/restart",			// scaler and champion
	"~/broadcast/+",		// delete??
	nullptr
};


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

	mqtt.setup(MQTT_HOST, MQTT_PORT, mqtt_user, mqtt_pass, deviceID, CA_CERT); 

	ota.set_ca_cert(CA_CERT);   // controller owns the cert; inject same root CA into OTA

	monitor.setup(8000);

	// brownout = power problem during last session.  reset_reason was
	// captured in monitor.setup -> metrics.check_all, so we can check it now.
	if (strcmp(metrics.reset_reason, "BROWNOUT") == 0) {
		events.increment("wifi_brown_outs");
	}

	// install the framework's standard subscription list.  user's own
	// subscriptions and any add_command_namespace calls go on top of this.
	mqtt.set_std_subscriptions(std_subscription_list);

	messages.set_clear_counter_handler([]() {
		events.reset_all();   // wipes everything including starts and brown_outs
		monitor.send_all();   // ship the freshly-cleared state immediately
	});

	// Restart the chip.  Brief delay first so the serial print has a
	// chance to flush; ESP.restart() yanks everything down hard.  No need
	// to disconnect mqtt cleanly — we publish at QoS 0 and the broker
	// will time the session out on its own.  The "starts" counter will
	// increment on the next boot via monitor.setup, which is all the
	// confirmation the cloud needs that the restart actually happened.
	messages.set_restart_handler([]() {
		Serial.println("\n\t### RESTARTING ###\n");
		delay(100);
		ESP.restart();
	});

	// provisioner.set_pin(15, 2000);		// uncomment if you are assigning a provisioning pin

	// OTA handler signature:
	//   group_name comes from the topic (~/~/ota/{group_name})
	//   auth_key   comes from the payload (raw string, not JSON)
	messages.set_ota_handler([](const char* group_name, const char* auth_key) {
		ota.update_firmware(group_name, auth_key);
	});

	mqtt.set_std_callback(messages.main_handler);	// router gets first look; user's raw handler runs for anything it doesn't claim

}


void Controller::loop() {

	// do something that doesn't require MQTT

	if (wifi_tools.is_connected) {

		mqtt.maintain();

		if (mqtt.is_connected) {

			if (monitor.boot_packet_not_sent) monitor.send_all();	// we don't send this in setup, because mqtt is not connected yet
			monitor.send_heartbeat();

			_report_boot_ota_outcome();   // once-only; no-op after first run

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

}
