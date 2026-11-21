

#include "metrics/metrics.h"

#include "Arduino.h"
#include <esp_ota_ops.h>
#include <esp_partition.h>


// check once when Metrics fires up, and then not again.
//
// This is the sha256 the bootloader appends to the app image — a hash of the
// running bytes, not of the .bin file (the file has extra padding + the digest
// itself, so `sha256sum firmware.bin` will NOT match this; reconcile on the
// server by chopping the last 32 bytes off the .bin, or via `esptool image_info`).
//
// It's read straight from the running partition, so it works the same in both
// Arduino IDE and PlatformIO with zero build flags, and it changes on every
// recompile because the compile timestamp is baked into the image.


void Metrics::_check_firmware(void) {

	// human-readable build timestamp — compile date + time baked into the
	// image by the compiler, so it's present regardless of IDE.  Gathered
	// first so it's always set, even if the hash read below fails.
	// core 2.0.17 names this esp_ota_get_app_description
	const esp_app_desc_t * desc = esp_ota_get_app_description();
	int built_len = snprintf(fw_built, sizeof(fw_built), "%s %s", desc->date, desc->time);
	if (built_len < 0 || (size_t)built_len >= sizeof(fw_built)) {
		Serial.println("\txx fw_built timestamp truncated");
	}

	uint8_t raw[32] = {0};

	esp_err_t err = esp_partition_get_sha256(esp_ota_get_running_partition(), raw);

	if (err != ESP_OK) {
		strlcpy(fw_hash, "unknown", sizeof(fw_hash));
		Serial.printf("\t   - fw_hash: unavailable (%s)\n", esp_err_to_name(err));
		return;
	}

	// format the 32 raw bytes as 64 lowercase hex chars
	fw_hash[0] = '\0';
	for (int i = 0; i < 32; i++) {
		size_t offset = (size_t)i * 2;
		int wrote = snprintf(&fw_hash[offset], sizeof(fw_hash) - offset, "%02x", raw[i]);
		if (wrote != 2) {
			Serial.println("\txx fw_hash truncated");
			break;
		}
	}
	fw_hash[64] = '\0';

}