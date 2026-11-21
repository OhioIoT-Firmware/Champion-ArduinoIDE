



#include "ota/ota.h"

#include "Arduino.h"

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_log.h>




void OTA::_log_ota_status() {
    
    Serial.println("\n\tENTERING CHECK FOR INVALID FIRMWARE:");

    Serial.print("\tESP_OTA_IMG_NEW: "); Serial.println(ESP_OTA_IMG_NEW);
    Serial.print("\tESP_OTA_IMG_PENDING_VERIFY: "); Serial.println(ESP_OTA_IMG_PENDING_VERIFY);
    Serial.print("\tESP_OTA_IMG_VALID: "); Serial.println(ESP_OTA_IMG_VALID);
    Serial.print("\tESP_OTA_IMG_INVALID: "); Serial.println(ESP_OTA_IMG_INVALID);
    Serial.print("\tESP_OTA_IMG_ABORTED: "); Serial.println(ESP_OTA_IMG_ABORTED);
    


    // running partition

    const esp_partition_t *running_partition = esp_ota_get_running_partition();
    if (running_partition != NULL) {
        Serial.printf("\t  .. running partition: type=%d, subtype=%d, address=0x%x, size=%d\n", 
            running_partition->type, 
            running_partition->subtype, 
            running_partition->address, 
            running_partition->size);
    } else {
        Serial.println("\t  .. no running partition found");
    }
    

    // last invalid partition

    const esp_partition_t *last_invalid_partition = esp_ota_get_last_invalid_partition();
    if (last_invalid_partition != NULL) {
        Serial.printf("\t  .. last invalid partition: type=%d, subtype=%d, address=0x%x, size=%d\n", 
            last_invalid_partition->type, 
            last_invalid_partition->subtype, 
            last_invalid_partition->address, 
            last_invalid_partition->size);
    } else {
        Serial.println("\t  .. no last invalid partition found");
    }




    esp_ota_img_states_t ota_state;
    esp_err_t err = esp_ota_get_state_partition(running_partition, &ota_state);
    if (err != ESP_OK) {
        Serial.printf("\t  xx failed to get OTA state: %s\n", esp_err_to_name(err));
    } else {
        Serial.printf("\t  .. OTA state: %d", ota_state);
    }


    


    switch (ota_state) {
        case ESP_OTA_IMG_UNDEFINED:                                             // 0xFFFFFFFF
            Serial.println(" - OTA state is undefined");
            break;

        case ESP_OTA_IMG_NEW:                                                   // 1
            Serial.println(" - OTA image is new and not yet validated");
            break;

        case ESP_OTA_IMG_PENDING_VERIFY:                                        // 2
            Serial.println(" - OTA image is pending verification");
            break;

        case ESP_OTA_IMG_VALID:                                                 // 3
            Serial.println(" - OTA image is valid");
            break;

        case ESP_OTA_IMG_INVALID:                                               // 4
            Serial.println(" - OTA image is invalid");
            break;

        case ESP_OTA_IMG_ABORTED:                                               // 5
            Serial.println(" - OTA image update was aborted");
            break;

        default:
            Serial.println(" - xx unknown OTA state");
            break;
    }




}



















