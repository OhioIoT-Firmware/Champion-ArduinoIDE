# OhioIoT — Champion SDK for Arduino IDE

The **Champion** tier of the OhioIoT firmware SDK for ESP32.

Connecting is simple: add the library, enter your WiFi and OhioIoT credentials, and upload.  There is no device registration.  The SDK keeps WiFi connected (it retries every 10 seconds) and keeps MQTT connected (it retries with backoff).

Requirements: arduino-esp32 core **2.0.17** and PubSubClient 2.8 or later.  Core 3.x is not supported.

Docs for this tier: [ohioiot.com/docs/code-samples/champion](https://ohioiot.com/docs/code-samples/champion).  Using an AI coding tool?  Point it at this repo and [AGENTS.md](AGENTS.md).

This code is still in development.  Questions or feedback: [ohioiot.com/contact](https://ohioiot.com/contact).


## Quick Start

1. In Arduino IDE, add ESP32 board support if you do not already have it: **File -> Preferences**, set Additional Board Manager URLs to `https://espressif.github.io/arduino-esp32/package_esp32_index.json`, then **Tools -> Board -> Boards Manager**, install **esp32** by Espressif Systems, version **2.0.17**.  Core 3.x is not supported.
2. **Tools -> Manage Libraries**, install **PubSubClient** by Nick O'Leary (2.8 or later).
3. On GitHub, use the green **Code** button -> Download ZIP.  (The unzipped folder is named like `Minimalist-ArduinoIDE-main`.  That is expected.)
4. Arduino IDE: **Sketch -> Include Library -> Add .ZIP Library**, and choose that ZIP.  If the IDE rejects the folder name, copy the unzipped folder into `Documents/Arduino/libraries/` (you can rename it to OhioIoT-Champion) and restart the IDE.
5. Open **File -> Examples -> OhioIoT-Champion -> Basic**.  Do not start from a blank sketch.
6. Fill in the four WiFi/MQTT values at the top of the sketch, then Upload.  Open the Serial Monitor at 115200 baud and wait for `mqtt connected`.

The four values at the top of the sketch:

```c
#define WIFI_SSID  "your-wifi-name"
#define WIFI_PASS  "your-wifi-password"
#define MQTT_USER  "your-mqtt-user"      // found in the Settings tab of the OhioIoT app
#define MQTT_PASS  "your-mqtt-password"  // found in the Settings tab of the OhioIoT app
```

The SDK connects to `mqtt.ohioiot.com` on port **8883** with TLS.  The CA certificate is built into the SDK, so you don't download a cert or create a `ca_cert.h` file.

Port **1883** is only for a local, unsecured broker on your own network; it won't work with the OhioIoT broker.

## Your first message

Put this in the Basic example.  Leave the four values from Quick Start at the top of the sketch.

`~/~/x` becomes `{user}/{device_id}/x`.  Publishing to `~/~/numb/<graph>/<field>` makes an automatic graph.

The SDK never calls `Serial.begin()`.  This sketch calls it so the Serial Monitor shows the incoming message.

```cpp
#include <OhioIoT-Champion.h>


// inbox for this device
static const char * subscription_list[] = {
    "~/~/inbox",
    nullptr    // required end of the list
};


// runs when a message arrives
void on_message(char * topic, char * payload) {
    Serial.println(payload);
}


void setup() {
    Serial.begin(115200);

    controller.setup(WIFI_SSID, WIFI_PASS, MQTT_USER, MQTT_PASS);

    mqtt.set_subscriptions(subscription_list);
    mqtt.set_callback(on_message);
}


void loop() {
    controller.loop();

    if (mqtt.is_connected) {
        // publish every 5 seconds
        static unsigned long timer = 0;

        if (millis() - timer >= 5000) {
            timer = millis();
            float temperature = 21.5;    // replace with a sensor reading
            mqtt.publish("~/~/numb/temperature/temp1", temperature);
        }
    }
}
```

## Device ID

The device id is generated once, on first boot, from the hardware random number generator.  It is saved in NVS (Preferences) and reused after that.  It is 8 lowercase letters and digits.  It must be unique per device on the broker.

## What's Included

| Part | What it does |
| --- | --- |
| `_certificates` | holds the CA certificate for the OhioIoT broker |
| `controller` | runs the modules from setup and loop |
| `device_id` | creates the device id once and saves it in NVS |
| `wifi_tools` | connects to WiFi and retries every 10 seconds |
| `mqtt` | connects to the broker, publishes, and subscribes |
| `events` | counts boots, drops, and retries, saved in NVS |
| `messages` | routes incoming topics to handlers |
| `metrics` | reads chip, memory, signal, and uptime |
| `monitor` | publishes identity and a heartbeat |
| `json` | builds small JSON objects in a char buffer |
| `ota` | downloads firmware in the background and can roll back |
| `power` | low-power connected mode, or deep sleep on a timer |
| `provisioner` | WiFi setup page when no password is saved |
| `settings` | values the app can change, saved in NVS |

## Layout

The library is laid out like this.

```text
examples/
    Basic/
        Basic.ino
src/
    OhioIoT-Champion.h
    controller/
    _certificates/
    device_id/
    wifi_tools/
    mqtt/
    events/
    messages/
    metrics/
    monitor/
    json/
    ota/
    power/
    provisioner/
    settings/
```

## The Controller

The controller lives in `src/controller/`.  It quarterbacks the remaining modules so that they can remain relatively unaware of each other.

You do not need to edit the controller for this to work.  It is plain, readable source.  Open it any time to see how the pieces fit together, or tweak it to change how the framework behaves.  Nothing here is a black box.

*Find video tutorials on making IoT systems at the [YouTube channel](https://www.youtube.com/@ohioiot).*

*[OhioIoT](https://ohioiot.com) is an IoT platform.  Built around ESP32.  Works with Raspberry Pi or any MQTT device.*

