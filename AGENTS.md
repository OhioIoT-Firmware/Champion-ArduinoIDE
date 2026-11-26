# AGENTS.md: OhioIoT Champion SDK for Arduino IDE

Notes for AI coding tools.  Read this before you review or change a sketch that uses this repo.

## What this is

ESP32 firmware SDK that connects a device to the OhioIoT MQTT broker (`mqtt.ohioiot.com`, port 8883, TLS, CA built in).  Connecting is simple: add the library, enter WiFi and OhioIoT credentials, and there is no device registration.  The SDK keeps WiFi (fixed 10 second retry) and MQTT (retry with backoff) connected.

This is the **Champion** tier.  Tiers stack: Minimalist, then Scaler, then Champion.  Modules in this tier: `controller`, `_certificates`, `device_id`, `wifi_tools`, `mqtt`, `events`, `messages`, `metrics`, `monitor`, `json`, `ota`, `power`, `provisioner`, `settings`.

## Files that matter

- `examples/Basic/Basic.ino`: the starting sketch, with the four WiFi and MQTT values at the top.
- `src/OhioIoT-Champion.h`: the one header a sketch includes.
- `src/controller/`: the controller.  It runs the modules from `setup()` and `loop()`.
- `src/<module>/`: one folder per module.
- `library.properties`: library name, version, and the PubSubClient dependency.

A user's own `.ino` lives in their sketch folder, not in this repo.  Read it against this library.

## How a sketch uses it

- Include: `#include <OhioIoT-Champion.h>` brings in `controller`, `mqtt`, and every module in this tier.
- In `setup()`: `controller.setup(WIFI_SSID, WIFI_PASS, MQTT_USER, MQTT_PASS);` then `mqtt.set_subscriptions(list);` and `mqtt.set_callback(handler);`.
- In `loop()`: call `controller.loop();` on every pass.  It keeps WiFi and MQTT up and delivers incoming messages.
- A subscription list ends with `nullptr`: `static const char * list[] = { "~/~/inbox", nullptr };`.
- Handler: `void handler(char * topic, char * payload)`.  `topic` arrives fully expanded.
- Topic shorthand: `~/` becomes `{mqtt_user}/`, and `~/~/` becomes `{mqtt_user}/{device_id}/` (this device).  `~/+/inbox` subscribes to `inbox` on all your devices.
- `mqtt.publish(topic, value)` takes text, int, float, or bool.  `~/~/numb/{graph}/{field}` makes an automatic graph.
- Publish only when `mqtt.is_connected` is true.  A publish while disconnected is dropped.
- The SDK never calls `Serial.begin()`.  The sketch does.

## Built-in message routes

The `messages` router sees each message first.  Anything it doesn't claim falls through to your handler.

- `~/~/command/{name}`, and `~/{namespace}/command/{name}[/{code}]` for each name in `mqtt.set_command_namespaces(list)` (nullptr-terminated).
- Command handler, set with `messages.set_command_handler(handler)`: `bool handler(const char * name, const char * payload, char * error)`.  Return true when the command is yours.  Leave `error` empty for success, or write the reason into it for failure.  The router then publishes `~/~/response/command/[{code}/]success` or `.../fail` with the error text.  Returning false means "not mine": no response, and the message falls through to your handler.
- `~/~/clear_counters` clears the event counters.  `~/~/restart` restarts the device.
- The monitor publishes device identity and a heartbeat to `~/~/status` on its own.

## Champion features

- Provisioner: `controller.setup(MQTT_USER, MQTT_PASS);` with no WiFi values opens a WiFi setup portal (access point `DEVICE PROVISIONING`) when no WiFi login is saved.  `provisioner.set_pin(pin);` reopens it when that pin is held low for 3 seconds.
- OTA: the app starts an update on `~/~/ota/{group}`.  The download runs in the background inside `controller.loop()`.  New firmware is accepted once it reaches MQTT, and it rolls back if that doesn't happen within 10 minutes.
- Settings: a `SettingRow` table ending in `{ nullptr, nullptr }`, passed to `settings.begin(table, on_change);`.  Values are saved in NVS and can be changed from the app.  Keys are 15 characters at most.
- Power: `power.save();` for low-power connected mode, or `power.sleep(seconds);` for deep sleep.  Waking from deep sleep restarts the sketch from `setup()`.

## Requirements

- arduino-esp32 core 2.0.17.  Core 3.x is not supported.
- PlatformIO: the official `espressif32` platform (core 2.x, tested on 6.13.0), not pioarduino.  Arduino IDE: esp32 boards package 2.0.17.
- PubSubClient 2.8 or later.

## Common mistakes to check

- Wrong core: 3.x, or the pioarduino platform.
- `controller.loop()` missing from `loop()`.
- `delay()` or other blocking code in `loop()`, so `controller.loop()` runs rarely and connections drop.  Time work with `millis()`.
- Publishing before `mqtt.is_connected` is true, including from `setup()`.  MQTT connects later, inside `controller.loop()`.
- A subscription list without `nullptr` at the end.
- `+` or `#` in a publish topic.  Wildcards are for subscriptions only.
- Real WiFi or MQTT credentials committed or pasted into a chat.  Use placeholders.
- Hard-coding a device id.  The SDK makes one on first boot (8 lowercase letters and digits) and keeps it in NVS.
- Using features this tier doesn't ship.  Champion ships every feature above.  Check that the sketch calls them as described here.

## More

OhioIoT: https://ohioiot.com.  Docs for this tier: https://ohioiot.com/docs/code-samples/champion
