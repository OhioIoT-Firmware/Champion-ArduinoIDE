

#include "provisioner/provisioner.h"
#include "Arduino.h"
#include "wifi_tools/serial_mask.h"

void Provisioner::_set_other_routes() {

    _web_server.on("/favicon.ico", [this]() { _web_server.send(200); });

    _web_server.onNotFound([this]() {
        // uri() is the path.  If a core build also hands back the query
        // string, the Wi-Fi password from the provisioning form stays masked.
        Serial.print("\txx 'route not found' for request: ");
        String uri = _web_server.uri();
        char scrubbed[160];
        if (mask_text_secrets(uri.c_str(), scrubbed, sizeof(scrubbed))) Serial.println(scrubbed);
        else Serial.println(uri);
        _web_server.send(204);
    });

}
