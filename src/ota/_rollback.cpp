

#include "ota/ota.h"
#include "Arduino.h"

#include <esp_ota_ops.h>


#include "Preferences.h"
static Preferences _prefs;


// CALLED IN STD_RESPONSES
void OTA::set_ota_pending() {
	/*
		we just successfully did the update.  so, note it in nvs so the device
		knows to rollback if necessary after boot
	*/

    _prefs.begin("ota", false);  // Open namespace "ota" in RW mode
    _prefs.putBool("ota_pending", true);
    _prefs.end();

	// storage.set_flag("ota", "ota_pending");
}




// USED IN MQTT_TOOLS
// USED IN STANDARD_RESPONSES
void OTA::mark_firmware_as_valid() {
    _prefs.begin("ota", false);  // Open namespace "ota" in RW mode
    if (_prefs.isKey("ota_pending")) {
        if (_prefs.remove("ota_pending")) {
            Serial.println("\t'ota_pending' key removed successfully.");
        } else {
            Serial.println("\txx failed to remove 'ota_pending' key.");
        }
    } else {
        Serial.println("\t'ota_pending' key not found, nothing to remove.");
    }
    _prefs.end();

	// if (storage.get_flag("ota", "ota_pending")) {
	// 	storage.clear_flag("ota", "ota_pending");
	// } else {
	// 	Serial.println("\tno ota flags to clear");
	// }

    // NOTE: I have not yet verified that this function is doing anything!
    esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
    if (err != ESP_OK) {    Serial.print("\txx filed to mark app as valid: "); Serial.println(esp_err_to_name(err)); } 
    else {                  Serial.println("\tfirmware marked as valid");    }

}


void OTA::_mark_firmware_as_valid_if_appropriate() {

	/**
	 * 		if we' renot requiring manual acceptance, then we can mark it valid after mqtt connects.
	 * 
	 * 		this should be getting called in main after mqtt connects.
	 */

	#ifndef REQUIRE_MANUAL_FIRMWARE_ACCEPT
		Serial.println("\tREQUIRE_MANUAL_FIRMWARE_ACCEPT flag not set; accepting the firmware now; we have both Wifi and MQTT");
		mark_firmware_as_valid();
	#endif
}




// USED IN MQTT_TOOLS
// USED IN WIFI_TOOLS
void OTA::rollback_and_reboot() {
    Serial.println("\tSHOULD BE CALLING FOR ROLLBACK\n\n");

    // Re-arm the boot-report gate. The next boot's reporter only speaks if
    // ota_pending is set; a commanded rollback (manual, or one issued after
    // the image was already accepted) would otherwise have cleared it and
    // boot silently. Harmless when already set (the timeout path) — it just
    // keeps every rollback, however triggered, reporting on the next boot.
    set_ota_pending();

    esp_err_t err = esp_ota_mark_app_invalid_rollback_and_reboot();
    if (err != ESP_OK) {    Serial.print("\txx failed to mark app as invalid and reboot: "); Serial.println(esp_err_to_name(err)); }
    else {                  Serial.println("\t.. marked app as invalid and rebooting\n\n");    }
}




// CALLED IN STD_SCHEDULE
void OTA::rollback_if_not_verified_within_time() {

    
    if (_need_to_check_rollback_once) {
        if (millis()-_rollback_timer > _rollback_threshold) {

            _log_ota_status();

            bool needs_rollback = OTA::_update_is_pending_validation();          

            if (needs_rollback) {
                Serial.println("\t  xx running partition has not yet been marked as valid..");
                rollback_and_reboot();
				// OTA::rollback_and_reboot();
            } else {
                Serial.println("\t  .. either we haven't yet done ota, or the last ota has already been validated");
            }

            _need_to_check_rollback_once = false;

        }
    }
}





bool OTA::_update_is_pending_validation() {
    bool status = false;
    if (!_prefs.begin("ota", true)) {
        // Serial.println("Error: unable to begin _prefs. OTA library might not be initialized.");
        return false;
    }
    if (_prefs.isKey("ota_pending")) {
        status = _prefs.getBool("ota_pending", false);
		// TODO:  can't remember why this is here
    } else {
        // Serial.println("\t.. no OTA pending (normal boot)");
        return false;
    }
    _prefs.end();
    return status;
}




// ── Group persistence + boot-time outcome reporting ──────────────────────
//
// The group name is stored alongside the ota_pending flag in the same NVS
// namespace, captured when an update starts.  After the reboot we use it to
// tag the accepted/rolled_back report so the cloud knows which group's
// device state to update — without any server-side lookup.

void OTA::save_ota_group(const char * group_name) {
    if (!group_name) return;
    _prefs.begin("ota", false);
    _prefs.putString("ota_group", group_name);
    _prefs.end();
}


const char * OTA::get_ota_group() {
    _ota_group[0] = '\0';
    if (!_prefs.begin("ota", true)) return _ota_group;
    if (_prefs.isKey("ota_group")) {
        _prefs.getString("ota_group", _ota_group, sizeof(_ota_group));
    }
    _prefs.end();
    return _ota_group;
}


// True when we booted into a firmware that still has the pending flag set —
// i.e. an OTA happened and this boot is the validation boot.  This is the
// signal the controller uses on startup to decide whether to report
// 'accepted' (we're the new image, about to be marked valid) vs leave the
// rollback path to fire.  Distinct from _update_is_pending_validation() only
// in being public for the controller to read at boot.
bool OTA::ota_was_pending_on_boot() {
    return _update_is_pending_validation();
}


// Rollback detection by partition state. After a rollback the bootloader
// marks the bad image ABORTED (or INVALID) and boots the previous one, so an
// ABORTED/INVALID "next update" partition is the rollback fingerprint. No
// transport here — the controller decides what to publish based on this.
bool OTA::was_rolled_back() {

    const esp_partition_t * next = esp_ota_get_next_update_partition(NULL);
    if (!next) {
        Serial.println("\t[was_rolled_back] no next-update partition found -> false");
        return false;
    }
    Serial.printf("\t[was_rolled_back] next-update partition: %s (offset 0x%x)\n",
                  next->label, next->address);

    esp_ota_img_states_t next_state;
    esp_err_t err = esp_ota_get_state_partition(next, &next_state);
    if (err != ESP_OK) {
        Serial.printf("\t[was_rolled_back] could not read partition state: %s -> false\n",
                      esp_err_to_name(err));
        return false;
    }

    // translate the enum to something readable in the log
    const char * state_name = "UNKNOWN";
    switch (next_state) {
        case ESP_OTA_IMG_NEW:            state_name = "NEW";            break;
        case ESP_OTA_IMG_PENDING_VERIFY: state_name = "PENDING_VERIFY"; break;
        case ESP_OTA_IMG_VALID:          state_name = "VALID";          break;
        case ESP_OTA_IMG_INVALID:        state_name = "INVALID";        break;
        case ESP_OTA_IMG_ABORTED:        state_name = "ABORTED";        break;
        case ESP_OTA_IMG_UNDEFINED:      state_name = "UNDEFINED";      break;
        default: break;
    }
    Serial.printf("\t[was_rolled_back] next-update partition state: %s\n", state_name);

    bool rolled_back = (next_state == ESP_OTA_IMG_ABORTED ||
                        next_state == ESP_OTA_IMG_INVALID);

    Serial.printf("\t[was_rolled_back] verdict: %s\n",
                  rolled_back ? "ROLLED BACK" : "accepted (not rolled back)");

    return rolled_back;
}







// USED IN STANDARD_RESPONSES
// void OTA::mark_firmware_as_valid_and_delete() {
//     _clear_ota_status();
//     esp_err_t err = esp_ota_mark_app_valid_cancel_rollback();
//     if (err != ESP_OK) {    Serial.print("\txx filed to mark app as valid: "); Serial.println(esp_err_to_name(err)); } 
//     else {                  Serial.println("\t.. marked new firmware as valid");    }
//     esp_err_t err2 = esp_ota_erase_last_boot_app_partition();
//     if (err2 != ESP_OK) {    Serial.print("\txx filed to mark app as valid: "); Serial.println(esp_err_to_name(err2)); } 
//     else {                  Serial.println("\t.. marked new firmware as valid");    }
// }