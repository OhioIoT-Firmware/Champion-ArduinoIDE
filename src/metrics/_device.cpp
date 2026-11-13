

#include "metrics/metrics.h"

#include "Arduino.h"
#include <esp_system.h>


// check once when Metrics fires up, and then not again


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


void Metrics::_check_device(void) {

	_copy_text(chip_model, sizeof(chip_model), ESP.getChipModel(), "chip model");

	itoa(ESP.getChipRevision(), chip_revision, 10);
	itoa(ESP.getChipCores(), chip_cores, 10);
	itoa(ESP.getCpuFreqMHz(), cpu_freq, 10);
	itoa(ESP.getFlashChipSize() / (1024 * 1024), flash_chip_size, 10);

	// why did the chip last reboot — only valid for THIS boot session.
	// strip the ESP_RST_ prefix so the cloud sees just "PANIC", "BROWNOUT", etc.
	const char * reason = "UNKNOWN";
	switch (esp_reset_reason()) {
		case ESP_RST_POWERON:   reason = "POWERON";   break;
		case ESP_RST_EXT:       reason = "EXT";       break;
		case ESP_RST_SW:        reason = "SW";        break;
		case ESP_RST_PANIC:     reason = "PANIC";     break;
		case ESP_RST_INT_WDT:   reason = "INT_WDT";   break;
		case ESP_RST_TASK_WDT:  reason = "TASK_WDT";  break;
		case ESP_RST_WDT:       reason = "WDT";       break;
		case ESP_RST_DEEPSLEEP: reason = "DEEPSLEEP"; break;
		case ESP_RST_BROWNOUT:  reason = "BROWNOUT";  break;
		case ESP_RST_SDIO:      reason = "SDIO";      break;
		default:                reason = "UNKNOWN";   break;
	}
	_copy_text(reset_reason, sizeof(reset_reason), reason, "reset reason");

	// Serial.printf("\t   - chip model: %s\n", chip_model);
	// Serial.printf("\t   - chip revision: %s\n", chip_revision);
	// Serial.printf("\t   - chip cores: %s\n", chip_cores);
	// Serial.printf("\t   - CPU frequency: %s MHz\n", cpu_freq);
	// Serial.printf("\t   - flash Size: %s MB\n", flash_chip_size);
	// Serial.printf("\t   - reset reason: %s\n", reset_reason);

}