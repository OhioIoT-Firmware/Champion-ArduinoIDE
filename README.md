# OhioIoT — Champion SDK for Arduino IDE

The **Champion** tier of the OhioIoT device firmware SDK for ESP32.

This code base is designed to help you connect your device to the OhioIoT cloud-based MQTT broker.

This code is subject to ongoing development. If you have any feedback or questions, please send a note from the contact page on the website: [ohioiot.com/contact](https://ohioiot.com/contact).


## Quick Start

1. In Arduino IDE, add ESP32 board support if you do not already have it: **File -> Preferences**, set Additional Board Manager URLs to `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, then **Tools -> Board -> Boards Manager**, install **esp32** by Espressif Systems. This SDK targets the 2.x core (2.0.17). Prefer that version over 3.x if both are offered.
2. **Tools -> Manage Libraries**, install **PubSubClient** by Nick O'Leary (2.8 or later).
3. On GitHub, use the green **Code** button -> Download ZIP. (The unzipped folder is named like `Minimalist-ArduinoIDE-main`. That is expected.)
4. Arduino IDE: **Sketch -> Include Library -> Add .ZIP Library**, and choose that ZIP. If the IDE rejects the folder name, copy the unzipped folder into `Documents/Arduino/libraries/` (you can rename it to OhioIoT-Champion) and restart the IDE.
5. Open **File -> Examples -> OhioIoT-Champion -> Basic**. Do not start from a blank sketch.
6. Fill in the four WiFi/MQTT values at the top of the sketch, then Upload. Open the Serial Monitor at 115200 baud and wait for `mqtt connected`.

The four values at the top of the sketch:

```c
#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"
#define MQTT_USER  "your-mqtt-user"      // found in the Settings tab of the OhioIoT app
#define MQTT_PASS  "your-mqtt-password"  // found in the Settings tab of the OhioIoT app
```

The SDK connects to `mqtt.ohioiot.com` on port **8883** with TLS. The CA certificate is built into the SDK, so you don't download a cert or create a `ca_cert.h` file.

Port **1883** is only for a local, unsecured broker on your own network; it won't work with the OhioIoT broker.

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

You do not need to edit the controller for this to work. It is plain, readable source. Open it any time to see how the pieces fit together, or tweak it to change how the framework behaves. Nothing here is a black box.

*Find video tutorials on making IoT systems at the [YouTube channel](https://www.youtube.com/@ohioiot).*

*OhioIoT is an IoT platform designed for [small-scale IoT projects](https://ohioiot.com).*

