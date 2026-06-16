# OhioIoT — Champion (ArduinoIDE)

The **Champion** tier of the OhioIoT device firmware SDK for ESP32.

The full SDK: OTA updates, provisioning, power management, and settings on top of Scaler.

## Quick start

1. Download this repo as a ZIP (green **Code** button -> Download ZIP).
2. Arduino IDE: **Sketch -> Include Library -> Add .ZIP Library**, choose the ZIP.
3. Open **File -> Examples -> OhioIoT-Champion -> Basic**.
4. Copy `credentials.template` to `credentials.h`, fill it in, then Upload. credentials.h is git-ignored so your secrets stay private.

## What's included

- _certificates
- device_id
- wifi_tools
- mqtt
- events
- messages
- metrics
- monitor
- json
- ota
- power
- provisioner
- settings

## The controller

You don't need to touch it — but the controller in `src/_controller/` is plain,
readable source. Open it any time to see how the pieces fit together, or tweak it
to change how the framework behaves. Nothing here is a black box.

## Feedback

This SDK is built to be read. If something's unclear or could be better, open an
issue — feedback is welcome.

## need to add a not that pubsubclient is required
