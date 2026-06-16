

#pragma once

#include <stdint.h>

class JSON {

	public:
	
        JSON();

		void first_key(char *, const char *, bool);
		void add_key(char *, const char *, bool);
		void last_key(char *, const char *, bool);
		void one_key(char *, const char *, bool);
		
		void first_key(char *, const char *, float);
		void add_key(char *, const char *, float);
		void last_key(char *, const char *, float);
		void one_key(char *, const char *, float);
		
		void first_key(char *, const char *, int);
		void add_key(char *, const char *, int);
		void last_key(char *, const char *, int);
		void one_key(char *, const char *, int);
		
		void first_key(char *, const char *, const char *);
		void add_key(char *, const char *, const char *);
		void last_key(char *, const char *, const char *);
		void one_key(char *, const char *, const char *);

	private:

        uint8_t _decimals;

};

extern JSON json;





