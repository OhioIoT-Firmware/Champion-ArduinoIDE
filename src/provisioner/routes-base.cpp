

#include "provisioner/provisioner.h"
#include "Arduino.h"
#include "provisioner/html.h"

void Provisioner::_set_base_routes() {

    _web_server.on("/", [this]() {
        Serial.println("\t.. base route hit");
        _web_server.send(200, "text/html", CONFIG_PAGE);
    });

    _web_server.on("/save", [this]() {

        Serial.println("\t.. save route hit");

        strlcpy(_ssid, _web_server.arg("ssid").c_str(), WIFI_SSID_LEN);
        strlcpy(_pass, _web_server.arg("pass").c_str(), WIFI_PASS_LEN);

        Serial.print("\n\tnew SSID: "); Serial.println(_ssid);
        Serial.print("\tnew PASS: ");   Serial.println(_pass);

        if (strlen(_ssid) < 4 || strlen(_pass) < 4) {
            Serial.println("\t.. error - SSID or password failed validation");
            _web_server.send(200, "text/html", INSTRUCTIONS_PAGE);
        } else {
            _provisioning_complete = true;
            _web_server.send(200, "text/html", SAVED_PAGE);
        }

    });

}

