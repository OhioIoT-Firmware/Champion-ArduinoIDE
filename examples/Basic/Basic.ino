

#include <OhioIoT.h>

#include "credentials.h"





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
	"soohee",
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
    return true;
}



// TODO:  user can also add an optional settings namespace, but settings isn't quite worked out.


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


