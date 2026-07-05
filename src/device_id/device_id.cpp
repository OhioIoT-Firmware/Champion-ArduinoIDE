

#include "device_id/device_id.h"

#include "Preferences.h"
static Preferences _prefs;

#include "Arduino.h"
#include "esp_random.h"		// hardware RNG — see _create_code()


Device_ID::Device_ID() {}

Device_ID device_id;



void Device_ID::get_or_set(char * buffer) {

	if ( !_prefs.begin("id", false) ) {
		Serial.println("\t\tfailed to open the 'id' namespace");
	} else {
		

		if ( _prefs.isKey("deviceID") ) {

			strcpy( buffer, _prefs.getString("deviceID").c_str() );

			// #ifdef SHOW_LOGS

				Serial.print("\texisting deviceID: ");
				Serial.println(buffer);

			// #endif


		} else {
			
			char code_holder[DEVICE_ID_LEN];
			_create_code(code_holder);
			strcpy( buffer, code_holder );
			_prefs.putString("deviceID", (const char *) buffer);

			Serial.print("\t\tno 'deviceID' key existed.  id created: ");
			Serial.println(buffer);

		}

		_prefs.end();
	}

}





void Device_ID::_create_code(char * buffer) {

	const char * character_options = "abcdefghijklmnopqrstuvwxyz0123456789";

	for (int i = 0; i < DEVICE_ID_LEN - 1; i++) {
		buffer[i] = character_options[esp_random() % 36];
	}

	buffer[DEVICE_ID_LEN-1] = 0;

}