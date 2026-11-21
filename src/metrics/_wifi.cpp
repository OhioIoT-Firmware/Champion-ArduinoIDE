

#include "metrics/metrics.h"
#include "Arduino.h"

#include <WiFi.h>



void ipToString(IPAddress ip, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}


static void _copy_text(char * dest, size_t dest_len, const char * src, const char * label) {
	if (dest == nullptr || dest_len == 0) return;
	if (src == nullptr) src = "";
	size_t copied = strlcpy(dest, src, dest_len);
	if (copied >= dest_len) {
		Serial.print("\txx ");
		Serial.print(label);
		Serial.print(" truncated to ");
		Serial.print(dest_len - 1);
		Serial.println(" characters");
	}
}


void Metrics::_check_wifi(void) {

    if (WiFi.status() != WL_CONNECTED) return; 

	ipToString(WiFi.localIP(), local_IP, sizeof(local_IP));
	ipToString(WiFi.dnsIP(), dns_IP, sizeof(dns_IP));

	_copy_text(SSID, sizeof(SSID), WiFi.SSID().c_str(), "SSID");
	_copy_text(mac_address, sizeof(mac_address), WiFi.macAddress().c_str(), "mac address");

}