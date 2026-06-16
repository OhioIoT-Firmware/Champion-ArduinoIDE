

#include "metrics/metrics.h"

#include "Arduino.h"
#include <esp_system.h>


// check once when Metrics fires up, and then not again


void Metrics::_check_device(void) {

	strcpy(chip_model, ESP.getChipModel());

	itoa(ESP.getChipRevision(), chip_revision, 10);
	itoa(ESP.getChipCores(), chip_cores, 10);
	itoa(ESP.getCpuFreqMHz(), cpu_freq, 10);
	itoa(ESP.getFlashChipSize() / (1024 * 1024), flash_chip_size, 10);

	// why did the chip last reboot — only valid for THIS boot session.
	// strip the ESP_RST_ prefix so the cloud sees just "PANIC", "BROWNOUT", etc.
	switch (esp_reset_reason()) {
		case ESP_RST_POWERON:   strcpy(reset_reason, "POWERON");   break;
		case ESP_RST_EXT:       strcpy(reset_reason, "EXT");       break;
		case ESP_RST_SW:        strcpy(reset_reason, "SW");        break;
		case ESP_RST_PANIC:     strcpy(reset_reason, "PANIC");     break;
		case ESP_RST_INT_WDT:   strcpy(reset_reason, "INT_WDT");   break;
		case ESP_RST_TASK_WDT:  strcpy(reset_reason, "TASK_WDT");  break;
		case ESP_RST_WDT:       strcpy(reset_reason, "WDT");       break;
		case ESP_RST_DEEPSLEEP: strcpy(reset_reason, "DEEPSLEEP"); break;
		case ESP_RST_BROWNOUT:  strcpy(reset_reason, "BROWNOUT");  break;
		case ESP_RST_SDIO:      strcpy(reset_reason, "SDIO");      break;
		default:                strcpy(reset_reason, "UNKNOWN");   break;
	}

	// Serial.printf("\t   - chip model: %s\n", chip_model);
	// Serial.printf("\t   - chip revision: %s\n", chip_revision);
	// Serial.printf("\t   - chip cores: %s\n", chip_cores);
	// Serial.printf("\t   - CPU frequency: %s MHz\n", cpu_freq);
	// Serial.printf("\t   - flash Size: %s MB\n", flash_chip_size);
	// Serial.printf("\t   - reset reason: %s\n", reset_reason);

}