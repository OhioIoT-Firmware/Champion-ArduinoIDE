

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
		void _check_memory(void);
		void _check_rssi(void);
		void _check_wifi(void);
		void _check_uptime(void);

};

extern Metrics metrics;