

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
		void increment(const char *);		// bumps RAM value AND writes NVS immediately — for rare, edge-triggered events
		void increment_ram(const char *);	// bumps RAM value only — for high-frequency events (e.g. wifi retries while offline)
		void flush();						// writes any RAM-only increments to NVS; no-op if nothing is pending
		void reset_all();

		unsigned int starts;
		unsigned int wifi_brown_outs;
		unsigned int wifi_retries;
		unsigned int wifi_drops;
		unsigned int mqtt_reconnects;
		unsigned int mqtt_drops;

	private:

		// keys with RAM-only increments that haven't hit NVS yet.
		// set by increment_ram(), cleared by flush() / reset_all().
		bool _wifi_retries_dirty = false;
		bool _wifi_drops_dirty   = false;
};

extern Events events;