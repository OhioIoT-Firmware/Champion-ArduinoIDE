

#include "monitor/monitor.h"
#include "Arduino.h"
#include "mqtt/mqtt.h"

#include "json/json.h"
#include "events/events.h"
#include "metrics/metrics.h"


Monitor::Monitor() {}

Monitor monitor;

void Monitor::setup(int interval) {
	events.load_all();
	events.increment("starts");
	// metrics.check_all();
	_interval = interval;
}


// Ships a complete state snapshot — boot packet plus a full sweep of
// dynamic data.  Called at first connect (gated externally by the
// controller via boot_packet_sent) and again whenever we want a fresh
// full-state publish, e.g. after clearing event counters.
//
// No internal gate on send_boot_packet — every call to send_all ships
// everything.  The first-boot-only behavior comes from the controller's
// loop choosing when to call this.
// void Monitor::send_all() {

// 	// _heartbeat_timer = millis();

// 	metrics.check_all();

// 	send_identity();
// 	send_session();
// 	send_network();
// 	send_counters();
// 	send_runtime();

// 	// boot_packet_sent = true;

// }


// Heartbeat only ships what can have changed.  Static stuff (chip info,
// MAC, reset reason, starts, brown_outs) was already sent in the boot packet.
//
// Defensive: if the boot packet never went out (e.g. mqtt wasn't
// connected when send_all ran), ship it now on the first successful tick.
void Monitor::send_heartbeat() {

	if (millis() - _heartbeat_timer > _interval) {

		if (_heartbeat_counter == 0) {

			metrics.check_all();
			send_identity();
			send_firmware();
			send_session();
			send_network();
			send_counters();
			send_runtime();


		} else {

			metrics.check_dynamic();
			if (_heartbeat_counter % 2) {
				send_counters();
			} else {
				send_runtime();
			}
			
		}

		_heartbeat_counter++;
		if (_heartbeat_counter > STATIC_REFRESH_EVERY) _heartbeat_counter = 0; 

		_heartbeat_timer = millis();
	}

}


// ─────────────────────────────────────────────────────────────────────
//  BOOT PACKET — session-static facts, sent once per power cycle
// ─────────────────────────────────────────────────────────────────────

// void Monitor::send_boot_packet() {
// 	send_identity();
// 	send_session();
// 	boot_packet_sent = false;
// }

void Monitor::refresh_counters() {
	send_session();
	send_counters();
}


// hardware identity — facts about the silicon itself.  identical across
// every boot of this device, would only change if the hardware changed.
void Monitor::send_identity() {

	char buffer[160] = "";

	json.first_key(buffer, "chip_mdl",  metrics.chip_model);
	json.add_key(buffer,   "chip_rev",  metrics.chip_revision);
	json.add_key(buffer,   "cores",     metrics.chip_cores);
	json.add_key(buffer,   "cpu_freq",  metrics.cpu_freq);
	json.last_key(buffer,  "flsh_size", metrics.flash_chip_size);

	mqtt.publish("~/~/status", buffer);
}


// firmware identity — sha256 of the running image (64 hex chars).  Shipped on
// its own line because it's session-static (can't change without a reboot, and
// a reboot re-runs the boot packet) and long enough to deserve its own message.
void Monitor::send_firmware() {

	char buffer[128] = "";

	json.first_key(buffer, "fw_hash",    metrics.fw_hash);
	json.last_key(buffer,  "fw_bld_date", metrics.fw_built);

	mqtt.publish("~/~/status", buffer);
}


// session boot facts — vary per power cycle but frozen during the session.
//   reset_reason     why this boot started (POWERON, BROWNOUT, PANIC, ...)
//   starts           lifetime boot count
//   wifi_brown_outs  lifetime brownout count
//
// brownouts are only ever detected at boot via esp_reset_reason, never
// observed in-session, so they ride here.
void Monitor::send_session() {

	char buffer[126] = "";
	char number_holder[12];

	itoa(events.starts, number_holder, 10);
	json.first_key(buffer, "starts", number_holder);

	json.add_key(buffer, "reset_reason", metrics.reset_reason);

	itoa(events.wifi_brown_outs, number_holder, 10);
	json.last_key(buffer,  "wifi_brown_outs", number_holder);

	mqtt.publish("~/~/status", buffer);
}


// network identity — IP/SSID/DNS rarely change; MAC never does.
void Monitor::send_network() {

	char buffer[160] = "";

	json.first_key(buffer,   "SSID",        metrics.SSID);
	json.add_key(buffer, "local_IP",    metrics.local_IP);
	// json.add_key(buffer,   "dns_IP",      metrics.dns_IP);
	json.last_key(buffer,  "mac_address", metrics.mac_address);

	mqtt.publish("~/~/status", buffer);
}


// ─────────────────────────────────────────────────────────────────────
//  HEARTBEAT — dynamic only, two messages
// ─────────────────────────────────────────────────────────────────────

// the four counters that can actually change during a running session.
// starts and wifi_brown_outs live in the boot packet — they're frozen
// once we're up and ticking.
//
// spot_rssi rides along here as well, even though it's also in send_runtime.
// RSSI is the single most useful field for diagnosing flaky links, and the
// heartbeat alternates between counters and runtime — without this, rssi
// only lands every other tick.  The duplication on send_all (boot) is
// harmless; the win is rssi on every heartbeat.
void Monitor::send_counters() {

	char buffer[126] = "";
	char number_holder[8];

	itoa(events.wifi_retries, number_holder, 10);
	json.first_key(buffer, "wifi_retries", number_holder);
	itoa(events.wifi_drops,   number_holder, 10);
	json.add_key(buffer,   "wifi_drops",   number_holder);
	itoa(events.mqtt_reconnects, number_holder, 10);
	json.add_key(buffer,   "mqtt_reconnects", number_holder);
	itoa(events.mqtt_drops,   number_holder, 10);
	json.add_key(buffer,   "mqtt_drops",   number_holder);
	json.last_key(buffer,  "spot_rssi",    metrics.spot_rssi);

	mqtt.publish("~/~/status", buffer);
}


// memory + signal + uptime — the stuff actually worth watching over time.
// uptime_s pairs with the counters: lets you compute rates ("3 wifi drops
// in 4 hours" vs "3 wifi drops in 30 seconds" are very different signals).
//
// spot_rssi shipped raw — cloud handles any smoothing.
void Monitor::send_runtime() {

	char buffer[126] = "";

	json.first_key(buffer, "free_heap_kb",     metrics.free_heap_kb);
	json.add_key(buffer,   "min_free_heap_kb", metrics.min_free_heap_kb);
	json.add_key(buffer,   "spot_rssi",     metrics.spot_rssi);
	json.last_key(buffer,  "uptime",      metrics.uptime_s);

	mqtt.publish("~/~/status", buffer);
}