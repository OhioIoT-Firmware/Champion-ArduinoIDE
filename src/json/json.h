#pragma once

#include <stddef.h>
#include <stdint.h>


/*
	JSON builds small status objects by appending to a char buffer.

	Pass the buffer and its size. If the next field does not fit, it is
	left out and a note is printed on Serial — the buffer is never
	written past its end, and it always stays null-terminated.

	When the buffer is a real array, the size is filled in for you:

	    char buf[128] = "";
	    json.first_key(buf, "temp", 21.5);

	When you only have a char*, pass the size yourself:

	    json.first_key(ptr, length, "temp", 21.5);
*/

class JSON {

	public:

        JSON();

		void first_key(char * buffer, size_t buffer_size, const char * key, bool value);
		void add_key(char * buffer, size_t buffer_size, const char * key, bool value);
		void last_key(char * buffer, size_t buffer_size, const char * key, bool value);
		void one_key(char * buffer, size_t buffer_size, const char * key, bool value);

		void first_key(char * buffer, size_t buffer_size, const char * key, float value);
		void add_key(char * buffer, size_t buffer_size, const char * key, float value);
		void last_key(char * buffer, size_t buffer_size, const char * key, float value);
		void one_key(char * buffer, size_t buffer_size, const char * key, float value);

		void first_key(char * buffer, size_t buffer_size, const char * key, int value);
		void add_key(char * buffer, size_t buffer_size, const char * key, int value);
		void last_key(char * buffer, size_t buffer_size, const char * key, int value);
		void one_key(char * buffer, size_t buffer_size, const char * key, int value);

		void first_key(char * buffer, size_t buffer_size, const char * key, const char * value);
		void add_key(char * buffer, size_t buffer_size, const char * key, const char * value);
		void last_key(char * buffer, size_t buffer_size, const char * key, const char * value);
		void one_key(char * buffer, size_t buffer_size, const char * key, const char * value);

		// array wrappers, the compiler supplies sizeof(buffer)
		template <size_t N> void first_key(char (&buffer)[N], const char * key, bool value)              { first_key(buffer, N, key, value); }
		template <size_t N> void add_key(char (&buffer)[N], const char * key, bool value)                { add_key(buffer, N, key, value); }
		template <size_t N> void last_key(char (&buffer)[N], const char * key, bool value)               { last_key(buffer, N, key, value); }
		template <size_t N> void one_key(char (&buffer)[N], const char * key, bool value)                { one_key(buffer, N, key, value); }

		template <size_t N> void first_key(char (&buffer)[N], const char * key, float value)             { first_key(buffer, N, key, value); }
		template <size_t N> void add_key(char (&buffer)[N], const char * key, float value)               { add_key(buffer, N, key, value); }
		template <size_t N> void last_key(char (&buffer)[N], const char * key, float value)              { last_key(buffer, N, key, value); }
		template <size_t N> void one_key(char (&buffer)[N], const char * key, float value)               { one_key(buffer, N, key, value); }

		template <size_t N> void first_key(char (&buffer)[N], const char * key, int value)               { first_key(buffer, N, key, value); }
		template <size_t N> void add_key(char (&buffer)[N], const char * key, int value)                 { add_key(buffer, N, key, value); }
		template <size_t N> void last_key(char (&buffer)[N], const char * key, int value)                { last_key(buffer, N, key, value); }
		template <size_t N> void one_key(char (&buffer)[N], const char * key, int value)                 { one_key(buffer, N, key, value); }

		template <size_t N> void first_key(char (&buffer)[N], const char * key, const char * value)      { first_key(buffer, N, key, value); }
		template <size_t N> void add_key(char (&buffer)[N], const char * key, const char * value)        { add_key(buffer, N, key, value); }
		template <size_t N> void last_key(char (&buffer)[N], const char * key, const char * value)       { last_key(buffer, N, key, value); }
		template <size_t N> void one_key(char (&buffer)[N], const char * key, const char * value)        { one_key(buffer, N, key, value); }

	private:

        uint8_t _decimals;

};

extern JSON json;
