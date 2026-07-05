

#include "metrics/metrics.h"
#include "Arduino.h"

#include <WiFiClientSecure.h>


Metrics::Metrics() {}

Metrics metrics;

void Metrics::check_all() {
	_check_device();
	_check_firmware();
	_check_rssi();
	_check_memory();
	_check_wifi();
	_check_uptime();
}


void Metrics::check_dynamic() {

	_check_rssi();
	_check_memory();
	// _check_wifi();
	_check_uptime();

}