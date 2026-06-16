

#include "ota/ota.h"

#include "Arduino.h"
#include "esp_http_client.h"


esp_err_t OTA::_http_event_handler(esp_http_client_event_t * evt) {
    static int total_bytes    = 0;
    static int bytes_received = 0;
    static int last_percent   = -1;

    switch (evt->event_id) {
        case HTTP_EVENT_ERROR:
            Serial.println("xxx http event error");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            Serial.println(">> connected ..");
            // reset counters at the start of each download
            total_bytes    = 0;
            bytes_received = 0;
            last_percent   = -1;
            ota._percent   = 0;
            break;

        case HTTP_EVENT_HEADER_SENT:
            Serial.println(">> header sent ..");
            break;

        case HTTP_EVENT_ON_HEADER:
            Serial.printf(">> header: %s: %s\n", evt->header_key, evt->header_value);
            if (strcasecmp(evt->header_key, "Content-Length") == 0) {
                total_bytes = atoi(evt->header_value);
            }
            break;

        case HTTP_EVENT_ON_DATA:
            bytes_received += evt->data_len;
            if (total_bytes > 0) {
                int percent = (bytes_received * 100) / total_bytes;
                if (percent != last_percent) {
                    Serial.printf("\r\t.. downloading: %3d%%  (%d / %d bytes)",
                                  percent, bytes_received, total_bytes);
                    if (percent == 100) Serial.println();
                    last_percent  = percent;
                    ota._percent  = percent;   // main loop reads this for MQTT
                }
            } else {
                // fallback if server didn't send Content-Length
                Serial.printf("\r\t.. downloaded %d bytes", bytes_received);
            }
            break;

        case HTTP_EVENT_ON_FINISH:
            Serial.println("\n>> finished ..");
            break;

        case HTTP_EVENT_DISCONNECTED:
            Serial.println(">> disconnected ..");
            break;

        default:
            break;
    }
    return ESP_OK;
}