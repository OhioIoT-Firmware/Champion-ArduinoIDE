

#include <OhioIoT-Champion.h>


#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"
#define MQTT_USER  "your-mqtt-user"		// found in the Settings tab of the OhioIoT app
#define MQTT_PASS  "your-mqtt-password"		// found in the Settings tab of the OhioIoT app



// REGULAR SUBSCRIPTIONS AND MESSAGE HANDLING Part 1 ----------------------------------

static const char * subscription_list[] = {
	"~/~/cypress",
	nullptr				// keep this
};


void messageHandler(char * topic, char * payload) {
	Serial.println(payload);	// replace this with something more interesting when you are ready
}



// STRUCTURED COMMANDS Part 1 ---------------------------------------------------------

static const char * command_namespaces[] = {
	"arborvitae",
	"dogwood",
	nullptr			// keep this
};


bool commandHandler(const char * sub_topic, const char * payload, char * error) {
	Serial.println(payload);	// replace this with something more interesting when you are ready
	return true;	// return false if the command couldn't be processed
					// write a string into the error buffer if you want to return a error message
}



//	SETTINGS Part 1 -------------------------------------------------------------------

char _threshold[SETTINGS_BUF_SIZE]  = "2.5";
char _sleep_secs[SETTINGS_BUF_SIZE] = "30";
char _mode[SETTINGS_BUF_SIZE]	   = "auto";

float threshold;
int   sleep_secs;

SettingRow settings_table[] = {
	{ "threshold",  _threshold  },
	{ "sleep_secs", _sleep_secs },
	{ "mode",       _mode       },
	{ nullptr,      nullptr     },
};

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
	
}




void loop() {

	controller.loop();

	if (mqtt.is_connected) {

		// do something
		
	}

}

