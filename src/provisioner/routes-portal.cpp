

#include "provisioner/provisioner.h"
#include "Arduino.h"
#include "provisioner/html.h"

void Provisioner::_set_portal_routes() {

    // Android routes

    _web_server.on("/generate_204", [this]() {
        Serial.println("\t.. got an Android request");
        _web_server.send(200, "text/html", CONFIG_PAGE);
    });

    _web_server.on("/gen_204", [this]() {
        Serial.println("\t.. got an Android request");
        _web_server.send(200, "text/html", CONFIG_PAGE);
    });

    // iOS routes

    _web_server.on("/hotspot-detect.html", [this]() {
        Serial.println("\t.. got an iOS request");
        _web_server.send(200, "text/html", CONFIG_PAGE);
    });

}