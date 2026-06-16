

#pragma once

#include <stdint.h> 


class Controller {

	public:

		Controller();

		void setup(const char *, const char *);		// if we're using provisioner, we don't need to provide wifi credentials
		void setup(const char *, const char *, const char *, const char *);				

		void loop();

	private:

		// Watches OTA state and reports progress / completion over MQTT.
		// Called once per loop(), keeps loop() readable.
		void _handle_ota_reporting();

		// On the first connected loop after boot, report whether a just-
		// applied OTA was accepted (we booted the new image) or rolled back
		// (we booted the old image). Tagged with the persisted group name.
		void _report_boot_ota_outcome();
		bool _boot_outcome_reported = false;

};


extern Controller controller;
