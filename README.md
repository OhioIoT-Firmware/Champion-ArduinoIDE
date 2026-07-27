# OhioIoT — Champion SDK for Arduino IDE

The **Champion** tier of the OhioIoT device firmware SDK for ESP32.

This code base is designed to help you connect your device to the OhioIoT cloud-based MQTT broker.

This code is subject to ongoing development. If you have any feedback or questions, please send a note from the web page: [ohioiot.com/contact](https://ohioiot.com/contact).

```
// define your variables

void setup() {
    controller.setup(WIFI_SSID, WIFI_PASS, MQTT_USER, MQTT_PASS);
    // add your own code
}

void loop()  {
    controller.loop();
    // add your own code
}
```

## Quick Start

1. Download this repo as a ZIP (green **Code** button -> Download ZIP).
2. Arduino IDE: **Sketch -> Include Library -> Add .ZIP Library**, choose the ZIP.
3. Open **File -> Examples -> OhioIoT-Champion -> Basic**.
4. Fill in the four WiFi/MQTT values at the top of the sketch, then Upload.

The four values at the top of the sketch:

```c
#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"

// MQTT_USER and MQTT_PASS are in the Settings tab of the OhioIoT app
#define MQTT_USER  "your-mqtt-user"
#define MQTT_PASS  "your-mqtt-password"
```

## What's Included

- _certificates
- _controller
- device_id
- wifi_tools
- mqtt
- events
- messages
- metrics
- monitor
- json

**Added with this tier:**

- ota
- power
- provisioner
- settings

## The Controller

The controller quarterbacks the remaining modules so that they can remain relatively unaware of each other.
You do not need to edit this module for this to work. It is plain,
readable source. Open it any time to see how the pieces fit together, or tweak it
to change how the framework behaves. Nothing here is a black box.

Feedback is welcome.

*Find video tutorials on making IoT systems at the YouTube channel (<https://youtube.com/@ohioiot>).*

*OhioIoT is an IoT platform designed for small-scale IoT projects (<https://ohioiot.com>).*

