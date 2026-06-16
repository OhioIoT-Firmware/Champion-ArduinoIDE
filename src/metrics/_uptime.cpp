

#include "metrics/metrics.h"

#include "Arduino.h"


// uptime in SECONDS since this boot.
//
// raw millis() rolls over at ~49.7 days (uint32 ms).  we divide by 1000
// before storing, which pushes rollover out to ~136 years on the same
// uint32 — effectively never for a deployed device.


void Metrics::_check_uptime(void) {

	unsigned long seconds = millis() / 1000UL;
	ultoa(seconds, uptime_s, 10);

}