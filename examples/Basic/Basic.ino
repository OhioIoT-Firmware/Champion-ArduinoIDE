#include <OhioIoT-Champion.h>


#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"
#define MQTT_USER  "your-mqtt-user"			// found in the Settings tab of the OhioIoT app
#define MQTT_PASS  "your-mqtt-password"		// found in the Settings tab of the OhioIoT app



// REGULAR SUBSCRIPTIONS AND MESSAGE HANDLING Part 1 ----------------------------------
//
// Topics this device listens to.  Two shorthands save you typing:
//
//     ~/        becomes   your-mqtt-user/
//     ~/~/      becomes   your-mqtt-user/this-device-id/
//
// So "~/~/inbox" below arrives as   your-mqtt-user/a1b2c3d4e/inbox
//
// Rename these, add your own, or leave just the nullptr if you don't need any.

static const char * subscription_list[] = {
	"~/~/inbox",			// anything sent to this one device
	"~/~/config",			// a second topic, to show the list takes several
	nullptr					// keep this
};


// Runs for every message that arrives on a topic above.
// `topic` is the full expanded topic; `payload` is the message body.
void messageHandler(char * topic, char * payload) {
	Serial.println(payload);	// replace this with something more interesting when you are ready
}



// STRUCTURED COMMANDS Part 1 ---------------------------------------------------------
//
// A namespace groups related commands.  Each name below subscribes to
//
//     ~/{namespace}/command/{command-name}
//
// So "pump" catches   your-mqtt-user/pump/command/start
//
// Name them after the things your device controls.

static const char * command_namespaces[] = {
	"pump",
	"lights",
	nullptr					// keep this
};


// Runs for every structured command that arrives.
// `sub_topic` is the part after the namespace, e.g. "command/start".
bool commandHandler(const char * sub_topic, const char * payload, char * error) {
	Serial.println(payload);	// replace this with something more interesting when you are ready
	return true;	// return false if the command couldn't be processed
					// write a string into the error buffer if you want to return a error message
}



//	SETTINGS Part 1 -------------------------------------------------------------------
//
// Values you can change from the app without reflashing.  One buffer per
// setting; the initializer is the default until the app sends something else.
// Rename these to suit your device.

char _threshold[SETTINGS_BUF_SIZE]  = "2.5";
char _sleep_secs[SETTINGS_BUF_SIZE] = "30";
char _mode[SETTINGS_BUF_SIZE]       = "auto";

float threshold;
int   sleep_secs;

SettingRow settings_table[] = {
	{ "threshold",  _threshold  },
	{ "sleep_secs", _sleep_secs },
	{ "mode",       _mode       },
	{ nullptr,      nullptr     },		// keep this
};

// Runs at boot and again every time a setting changes in the app.
void settingsChanged() {
	threshold  = settings.convert_to_float(_threshold, 2.5);
	sleep_secs = settings.convert_to_int(_sleep_secs, 30);
	// _mode needs no conversion — use the buffer directly wherever you need it
}



void setup() {

	Serial.begin(115200);
	Serial.println("\n\n\n+++++++++++  DEVICE BOOT  ++++++++++++++++++++++++++++++++++\n");

	controller.setup(WIFI_SSID, WIFI_PASS, MQTT_USER, MQTT_PASS);
	// controller.setup(MQTT_USER, MQTT_PASS);		// auto-launches the provisioner

	// REGULAR SUBSCRIPTIONS AND MESSAGE HANDLING Part 2 -----------------------------------
	mqtt.set_subscriptions(subscription_list);
	mqtt.set_callback(messageHandler);

	// STRUCTURED COMMANDS Part 2 ----------------------------------------------------------
	mqtt.set_command_namespaces(command_namespaces);
	messages.set_command_handler(commandHandler);
	
	//	SETTINGS Part 2 --------------------------------------------------------------------
	settings.begin(settings_table, settingsChanged);

	// provisioner.set_pin(X);		// un-comment if you want to assign a pin for provisioning (delete this line if not)
	

	// your own setup code goes here

}




void loop() {

	controller.loop();

	// Keep this loop non-blocking — no delay().  The controller needs to run
	// often to hold the WiFi and MQTT connections up.  For periodic work,
	// compare millis() against a timestamp you saved last time round.

	if (mqtt.is_connected) {

		// your own code here — publish readings, read sensors, and so on
		
	}

}
