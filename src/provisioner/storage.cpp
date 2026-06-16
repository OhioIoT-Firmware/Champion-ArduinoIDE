
#include "provisioner/provisioner.h"
#include "Arduino.h"

#include "Preferences.h"
static Preferences _prefs;




bool Provisioner::creds_already_exist(char * ssid, char * pass) {
    bool creds_exist = false;
    _prefs.begin("creds");
    if (_prefs.isKey("ssid")) {
        Serial.println("\n\tusing stored credentials");
        strlcpy(ssid, _prefs.getString("ssid").c_str(), WIFI_SSID_LEN);
        strlcpy(pass, _prefs.getString("pass").c_str(), WIFI_PASS_LEN);
        creds_exist = true;
    } else {
        Serial.println("\n\tusing credentials from credentials.h");
    }
    _prefs.end();
    return creds_exist;
}

        
void Provisioner::store_creds(char * ssid, char * pass) {
    _prefs.begin("creds");
    _prefs.putString("ssid", ssid);
    _prefs.putString("pass", pass);
    _prefs.end();
}


// needs to be called manually
void Provisioner::clear_creds() {
    _prefs.begin("creds");
    _prefs.clear();
    _prefs.end();
}