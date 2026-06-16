
#include "power/power.h"
#include "Arduino.h"

#include <WiFi.h>

#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_wifi.h"
#include "esp_idf_version.h"   // ESP_IDF_VERSION / ESP_IDF_VERSION_VAL (version guard below)


Power::Power() {}

Power power;


void Power::save() {

	/*
		Power::save() — low-power connected mode. Configures the ESP32 for efficient continuous operation rather 
		than sleep: the CPU is locked at 80 MHz (the minimum that supports WiFi), automatic light sleep is enabled 
		so that any idle time — most notably time spent inside delay() calls in loop() — is converted into actual 
		light sleep instead of busy-waiting, and the WiFi modem is set to minimum modem-sleep so the radio naps 
		between AP beacons. Crucially, all state is preserved: RAM, program execution, the WiFi association, and 
		the MQTT session all stay alive, so the device can receive or publish a message within milliseconds at 
		any time. Use this mode when the device must remain reachable and responsive — listening for commands, 
		maintaining an MQTT subscription, or reporting frequently — and structure the loop around generous delay() 
		calls so the chip spends most of its life lightly sleeping between moments of work. Call it once at startup 
		after connecting.
	*/


	int cpu_mhz = 80;
	
	setCpuFrequencyMhz(cpu_mhz);

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
	esp_pm_config_t       pm_config = {        // IDF 5.x  (core 3.x)
#else
	esp_pm_config_esp32_t pm_config = {        // IDF 4.x  (core 2.x)
#endif
		.max_freq_mhz       = cpu_mhz,
		.min_freq_mhz       = cpu_mhz,
		.light_sleep_enable = true
	};
	esp_pm_configure(&pm_config);

	WiFi.setSleep(WIFI_PS_MIN_MODEM);

}






void Power::sleep(uint32_t seconds) {

	/*
		Power::sleep(seconds) — deep sleep with timer wakeup. This is the hard shutdown: WiFi is cleanly disconnected 
		and powered off, an RTC timer is armed for the requested number of seconds, and the chip enters deep sleep, 
		cutting power to the CPU, RAM, and radio so consumption falls to the microamp range. The trade-off is that 
		nothing survives — waking is a full reboot starting back at setup(), all variables are lost unless explicitly 
		stored in RTC memory or flash, and both the WiFi connection and the MQTT session must be re-established from 
		scratch, which typically costs several seconds and a burst of power before the device is communicating again. 
		Use this when the device only needs to act at long intervals — say, waking every five or thirty minutes to 
		take a reading and publish it — and is unreachable in between; the reconnect cost is paid once per wake cycle 
		but the savings while asleep dwarf it. The function never returns.	
	*/	

    delay(100);                      // let any pending TX flush before we tear down

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);

    esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
    esp_deep_sleep_start();
}