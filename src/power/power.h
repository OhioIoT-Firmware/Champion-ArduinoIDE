
#pragma once

#include <stdint.h>


class Power {

	public:
	
		Power();
		
		void save();

		void sleep(uint32_t);
	
};

extern Power power;
