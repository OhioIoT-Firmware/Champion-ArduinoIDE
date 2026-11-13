





#pragma once

#ifndef DEVICE_ID_LEN
    #define DEVICE_ID_LEN 9
#endif


class Device_ID {

	public:

		Device_ID();

		// buffer must be at least DEVICE_ID_LEN bytes. A stored id that is
		// longer than that is truncated and a warning is printed.
		void get_or_set(char * buffer);

	private:


		void _create_code(char *);

};

extern Device_ID device_id;


