

#pragma once


class Metrics {

	/*
		Metrics gathers system values and stores them as char arrays.
		Makers reads those values and sends them via MQTT.
	*/

	public:
	
		Metrics();
		
		void check_all();
		void check_dynamic();

		// device  (static — set once at boot)
		char chip_model[20];
		char chip_revision[20];
		char chip_cores[20];
		char cpu_freq[20];
		char flash_chip_size[20];
		char reset_reason[16];

		// firmware identity — sha256 of the running image, as 64 hex chars.
		// computed by the build tooling and appended to the image, so it's
		// present in both Arduino IDE and PlatformIO with no build flags.
		// changes on every recompile (compile time is baked into the image).
		char fw_hash[65];

		// human-readable build timestamp from the app descriptor (compile
		// date + time, e.g. "Jun 29 2026 14:33:02").  Reliably populated in
		// both Arduino IDE and PlatformIO (these come from compiler macros,
		// unlike name/version which are placeholders in Arduino builds).
		char fw_built[24];

		// memory
		char free_heap_kb[10];
		char min_free_heap_kb[10];

		// rssi
		char spot_rssi[10];

		// wifi
		char local_IP[20];
		char SSID[20];
		char dns_IP[20];
		char mac_address[20];

		// runtime
		char uptime_s[12];


	private:

		void _check_device(void);
		void _check_firmware(void);
		void _check_memory(void);
		void _check_rssi(void);
		void _check_wifi(void);
		void _check_uptime(void);

};

extern Metrics metrics;