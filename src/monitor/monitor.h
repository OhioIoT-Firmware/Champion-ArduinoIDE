

#pragma once

#include "Arduino.h"


class Monitor {

	public:
	
		Monitor();

		void setup(int);

		void send_all();
		void send_heartbeat();

		// boot packet — static device facts, sent once per power cycle
		void send_boot_packet();
		void send_identity();
		void send_session();
		void send_network();

		// heartbeat payloads — only what actually changes
		void send_counters();
		void send_runtime();

		bool boot_packet_not_sent = true;


	private:

		int _interval;
		uint8_t _heartbeat_counter = 0;
		unsigned long _heartbeat_timer = 0;

};

extern Monitor monitor;