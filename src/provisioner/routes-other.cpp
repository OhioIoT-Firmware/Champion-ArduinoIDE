

#include "provisioner/provisioner.h"
#include "Arduino.h"

void Provisioner::_set_other_routes() {

    _web_server.on("/favicon.ico", [this]() { _web_server.send(200); });

    _web_server.onNotFound([this]() {
        Serial.print("\txx 'route not found' for request: ");
        Serial.println(_web_server.uri());
        _web_server.send(204);
    });

}
