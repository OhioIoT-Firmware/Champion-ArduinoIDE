

#include "metrics/metrics.h"
#include "Arduino.h"

#include <WiFiClientSecure.h>


Metrics::Metrics() {}

Metrics metrics;
// TODO:  do i want to instantiate here?  if the user wants it, shoudl that be a separate instance?
// TODO:  ask AI to analyze whether or not it makes sense to expose all of these instances for clarity

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