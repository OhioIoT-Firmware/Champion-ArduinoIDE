

#include "provisioner/provisioner.h"
#include "Arduino.h"


void Provisioner::set_pin(int pin, unsigned long timer) {

    
    _provisioning_pin = pin;
    _provisioning_timeout = timer;
    pinMode(pin, INPUT_PULLUP);


}

bool Provisioner::pin_is_pulled() {
    
    if (_provisioning_pin < 0) {
        return false;
    }

    if (!digitalRead(_provisioning_pin)) {

        if (!_timer_is_on) {
            _timer_is_on = true;
            _pin_timer = millis();
        }

        if (millis() - _pin_timer > _provisioning_timeout) {
            _timer_is_on = false;
            return true;
        }

    } else {
        _timer_is_on = false;
    }

    return false;

}