

#include <OhioIoT-Champion.h>



#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"
#define MQTT_USER  "your-mqtt-user"
#define MQTT_PASS  "your-mqtt-password"




/* 
	1A) If you want any direct subscriptions, put them here.  You want direct subscriptions for messages that aren't coming
	from the app.  In-app commands are automatically subscribed to in Scaler and Champion.
*/
static const char * subscription_list[] = {
	"~/~/whatever",
    nullptr
};

/*
	2A)  If you are subscribing to custom topics, you need a function that gets called when those messages come in.  This works
	in every tier.  Define a function here, in the global space, and inject it with mqtt.set_callback() below.
*/
void messageHandler(char * topic, char * payload) {
    Serial.println("\n\tWE GOT A GENERAL MESSAGE:");
    Serial.print("\ttopic: ");
    Serial.print(topic);
    Serial.print("\t\tpayload: ");
    Serial.println(payload);
}


/* 
	3A)  Command namespaces.  Same pattern as above, but these are group
	names, not full topics.  Each entry opts this device into commands
	addressed to that group — the framework expands each one to
	~/{group}/command/+ and ~/{group}/command/+/+ at connect time.
*/
static const char * command_namespaces[] = {
	"trump",
	nullptr
};

/*
	4A)  If you want to take advantage of the command builder, create a separate function here (in the global space) and inject
	that below with message.set_command_handler().  You prefer this option when you wan to react to individual commands.  The
	messages firmware will subscribe automatically to the structured topics created by the app.  then, the messages module will 
	receive those messages, parse the topic, and forward the command to you.
	see "url" for more.
*/
bool commandHandler(const char * topic, const char * payload, char * error) {
    Serial.println("\n\tWE GOT A COMMAND:");
    Serial.print("\ttopic: ");
    Serial.print(topic);
    Serial.print("\t\tpayload: ");
    Serial.println(payload);
	if (strcmp(topic, "trump") == 0) Serial.print("SUCCESS");
    return true;
}


/*
	5A)  Settings.  Values stored on the device and editable from the device
	dashboard.  Three parts:

	  i.   One buffer per setting.  The initializer is the compiled default.
	       Buffer names are yours — the cloud never sees them.  Anything you
	       use as a number also gets a typed variable.

	  ii.  A table mapping each dashboard key to its buffer.  Keys: 15 chars
	       max, no '/', '+', '#', or spaces.  End with the nullptr row, same
	       as your subscription lists.

	  iii. A conversion function — re-derives every typed variable from its
	       buffer.  No keys, no ifs; one line per numeric setting.  The
	       framework calls it once at boot and again after every update.
	       Strings need no line at all: the buffer IS the setting.

	The framework does the rest: loads stored values from flash (or persists
	your defaults on first boot), shows the settings on the dashboard, applies
	edits, persists them, re-runs your function, and echoes the held value
	back so the dashboard always shows the truth.  A value that isn't a
	number lands in the buffer honestly — your fallback protects the typed
	variable, and the failure is posted to your app's Messages inbox.
*/
char _threshold[SETTINGS_BUF_SIZE]  = "2.5";
char _sleep_secs[SETTINGS_BUF_SIZE] = "30";
char _mode[SETTINGS_BUF_SIZE]       = "auto";

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

    // 1B)
	mqtt.set_subscriptions(subscription_list);

	/*
		2B)  Inject your function here.  Works in every tier — this is the raw
		message hook.  In Scaler/Champion the framework router runs first and
		only un-routed messages reach this handler.
	*/
    mqtt.set_callback(messageHandler);

	/*
		3B) opt into command groups.  these are declared in the array at 1A-2,
		alongside the direct subscriptions — same technique for both.
	*/
    mqtt.set_command_namespaces(command_namespaces);

	// 4B) 
	messages.set_command_handler(commandHandler);


	/*
		5B)  Hand over the table and your conversion function.  Stored values
		load from flash and your function runs once, right here — so your
		typed variables are correct before loop() ever starts.
	*/
	settings.begin(settings_table, settingsChanged);

    // power.save();

	// provisioner.set_pin(X);

}




void loop() {

	controller.loop();

    if (mqtt.is_connected) {

		// do something
		
    }

	// power.sleep();

}





// TODO: Device ID randomness. _create_code() uses random(36) with no randomSeed(). On the ESP32 Arduino core random() is backed by the hardware RNG (esp_random()), so this is almost certainly fine — but given that a collision in an 8-char ID would be catastrophic in a multi-tenant system, it's worth a one-line confirmation that your core version routes random() to the HW RNG and not a deterministic newlib PRNG. If you ever have doubt, seed from esp_random() explicitly before the loo

// TODO: events.increment() writes NVS on every wifi retry. wifi_retries increments each time reconnect() fires, which is every RECONNECT_INTERVAL (10 s) while offline — and each increment is a putUInt. A device stuck offline overnight writes the same key thousands of times. NVS has wear leveling, but this is the kind of thing that quietly kills flash on a long-deployed fleet. Consider accumulating retries/drops in RAM and flushing to NVS only on a clean reconnect (or on a timer), rather than on every attempt.

// TODO:  do we need to lock the partition scheme as well?
