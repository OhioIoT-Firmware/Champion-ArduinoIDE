





#pragma once

/*

	the purpose of this library is to 

*/


class Device_ID {

	public:

		Device_ID();

		void get_or_set(char *);

	private:


		void _create_code(char *);

};

extern Device_ID device_id;


