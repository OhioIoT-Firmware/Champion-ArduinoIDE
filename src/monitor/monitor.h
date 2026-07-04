

#pragma once

#include "Arduino.h"

#ifndef STATIC_REFRESH_EVERY
	#define STATIC_REFRESH_EVERY 30
#endif


class Monitor {

	public:
	
		Monitor();

		void setup(int);

		// void send_all();
		void send_heartbeat();
		void refresh_counters();

		// boot packet — static device facts
		// void send_boot_packet();
		void send_identity();
		void send_firmware();
		void send_session();
		void send_network();

		// heartbeat payloads — only what actually changes
		void send_counters();
		void send_runtime();

		// bool boot_packet_sent = false;


	private:

		int _interval;
		uint8_t _heartbeat_counter = 0;
		unsigned long _heartbeat_timer = 0;

};

extern Monitor monitor;