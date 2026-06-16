
// NOTE TO AI:  this entire module is @champion.  the entire file should be excluded unless we're building the champion version.


/*
	OTA *reporting* lives here, in the controller — not in the OTA module.

	The OTA module is a pure, transport-agnostic state machine: it downloads,
	flashes, and inspects partitions, but does not touch MQTT and does not
	reboot (see ota.h).  Everything in this file is policy and messaging
	convention layered on top of that state machine:

	  - which topics progress/outcomes publish under (~/~/response/ota/...)
	  - how chatty progress reporting is (OTA_PROGRESS_INCREMENT)
	  - when to reboot after a successful download
	  - boot-time accepted/rolled_back reporting

	Split into its own translation unit purely to keep controller.cpp short;
	same class, same boundary.
*/

#include "Arduino.h"

#include "_controller/controller.h"

#include "mqtt/mqtt.h"
#include "ota/ota.h"


// How often to publish OTA progress, in percentage increments.
// Override with a build flag in platformio if you want something different.
#ifndef OTA_PROGRESS_INCREMENT
    #define OTA_PROGRESS_INCREMENT 1
#endif


void Controller::_handle_ota_reporting() {

	// Outbound OTA messages publish under ~/~/response/ota/{tag}.  This
	// matches the broader convention ~/~/response/{domain}/...  used by
	// command responses (see messages.cpp), and keeps OTA traffic out of
	// any inbound subscription pattern so the broker can't echo our own
	// progress back into the message dispatcher.

	static int        last_published_pct = -1;
	static OTA::State last_state         = OTA::IDLE;

	OTA::State state = ota.get_state();

	// progress publishing — fire when we've crossed a percent boundary
	// (see OTA_PROGRESS_INCREMENT) or hit 100% exactly.
	if (state == OTA::IN_PROGRESS) {
		int pct         = ota.get_percent();
		int bucket      = pct / OTA_PROGRESS_INCREMENT;
		int last_bucket = (last_published_pct < 0)
		                    ? -1
		                    : last_published_pct / OTA_PROGRESS_INCREMENT;
		if (bucket != last_bucket || pct == 100) {
			mqtt.publish("~/~/response/ota/progress", pct);
			last_published_pct = pct;
		}
	}

	// state-transition handling
	if (state != last_state) {

		if (state == OTA::SUCCESS) {
			// payload carries the group name (not "ok") so the cloud can
			// open the per-device 'pending' lifecycle for the right group.
			mqtt.publish("~/~/response/ota/complete", ota.get_ota_group());
			// give MQTT a few cycles to actually flush over WiFi before reboot
			for (int i = 0; i < 5; i++) {
				mqtt.maintain();
				delay(100);
			}
			ESP.restart();   // does not return
		}
		else if (state == OTA::FAILED) {
			mqtt.publish("~/~/response/ota/failed", "error");
			ota.acknowledge();          // resets ota state to IDLE
			last_published_pct = -1;    // ready for a future attempt
		}

		last_state = state;
	}
}


void Controller::_report_boot_ota_outcome() {

	// Runs once, on the first connected loop after boot.
	//
	// We only have a story to tell if an OTA was pending across this boot
	// (the NVS flag set by ota.set_ota_pending() right before the post-
	// download reboot). If it isn't pending, this was an ordinary boot —
	// nothing to report.
	//
	// ota.was_rolled_back() does the partition inspection; we just publish
	// the result. accepted -> we booted the new image fine. rolled_back ->
	// the bootloader reverted us to the previous image.

	if (_boot_outcome_reported) return;
	_boot_outcome_reported = true;

	if (!ota.ota_was_pending_on_boot()) {
		// no OTA in flight across this boot — nothing to report
		return;
	}

	// snapshot the group before we clear anything
	char group[64];
	strlcpy(group, ota.get_ota_group(), sizeof(group));

	if (!ota.was_rolled_back()) {
		Serial.println("\t.. OTA accepted — reporting and marking firmware valid");
		mqtt.publish("~/~/response/ota/accepted", group);
		ota.mark_firmware_as_valid();   // clears ota_pending, cancels rollback
	} else {
		Serial.println("\t.. OTA rolled back — reporting (running the previous image)");
		mqtt.publish("~/~/response/ota/rolled_back", group);
		// pending flag is stale on the old image; clear it so we don't
		// mistake a future boot for a rollback.
		ota.mark_firmware_as_valid();
	}

	// flush before moving on; QoS 0, so give WiFi a moment
	for (int i = 0; i < 3; i++) { mqtt.maintain(); delay(50); }
}
