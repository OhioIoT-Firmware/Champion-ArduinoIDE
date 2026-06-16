

#pragma once


#include "Arduino.h"

/*
	tracks lifetime counts of system events (boots, wifi/mqtt drops & retries,
	brownouts).  values persist across reboots in NVS.

	0 means "never happened".  no default seeding — keys are created on first
	increment.
*/


class Events
{
	public:
		Events();

		void load_all();
		void increment(const char *);
		void reset_all();

		unsigned int starts;
		unsigned int wifi_brown_outs;
		unsigned int wifi_retries;
		unsigned int wifi_drops;
		unsigned int mqtt_reconnects;
		unsigned int mqtt_drops;
};

extern Events events;