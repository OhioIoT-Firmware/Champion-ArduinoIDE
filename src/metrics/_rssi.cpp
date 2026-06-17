

#include "metrics/metrics.h"
#include "Arduino.h"

// #include <WiFiClientSecure.h>
#include <WiFi.h>


// RSSI is slow-moving signal-strength data.  We sample it once per heartbeat
// (via metrics.check_dynamic in monitor) and ship the raw value.  Any
// smoothing belongs cloud-side, where the window can be tuned without
// reflashing devices.

void Metrics::_check_rssi(void) {

	int32_t spot = WiFi.RSSI();
	itoa(spot, spot_rssi, 10);

}