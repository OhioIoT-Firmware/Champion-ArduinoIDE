





#pragma once

#ifndef DEVICE_ID_LEN
    #define DEVICE_ID_LEN 9
#endif


class Device_ID {

	public:

		Device_ID();

		void get_or_set(char *);

	private:


		void _create_code(char *);

};

extern Device_ID device_id;


