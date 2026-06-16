

#include "metrics/metrics.h"
#include "Arduino.h"

#include <WiFiClientSecure.h>



void ipToString(IPAddress ip, char* buffer, size_t bufferSize) {
    snprintf(buffer, bufferSize, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}


void Metrics::_check_wifi(void) {

    if (WiFi.status() != WL_CONNECTED) return; 

	ipToString(WiFi.localIP(), local_IP, sizeof(local_IP));
	ipToString(WiFi.dnsIP(), dns_IP, sizeof(dns_IP));

	strncpy(SSID, WiFi.SSID().c_str(), sizeof(SSID) - 1);
	SSID[sizeof(SSID) - 1] = '\0';

	strncpy(mac_address, WiFi.macAddress().c_str(), sizeof(mac_address) - 1);
	mac_address[sizeof(mac_address) - 1] = '\0';

	// Serial.print("\t   - IP: ");      Serial.println(local_IP);
	// Serial.print("\t   - SSID: ");    Serial.println(SSID);
	// Serial.print("\t   - dnsIP: ");   Serial.println(dns_IP);
	// Serial.print("\t   - mac: ");     Serial.println(mac_address);

}