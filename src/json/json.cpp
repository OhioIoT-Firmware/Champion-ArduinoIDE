#include "json/json.h"
#include "Arduino.h"

JSON::JSON() {}

JSON json;


// Where the next field sits in the object.
enum JsonPlace { JSON_FIRST, JSON_NEXT, JSON_LAST, JSON_ONLY };


static void _warn_left_out(const char * key) {
	Serial.print("\txx json field '");
	Serial.print(key && key[0] ? key : "?");
	Serial.println("' does not fit in the buffer — left out");
}


// Append one field, or leave the buffer unchanged if it does not fit.
// A closing '}' is added when a final field is dropped, so earlier fields
// stay a valid object instead of an unclosed string.
static void _append_field(char * buffer, size_t capacity, JsonPlace place,
                          const char * key, const char * value, bool quote) {

	if (buffer == nullptr || capacity == 0) return;
	if (key == nullptr) key = "";
	if (value == nullptr) value = "";

	size_t used = 0;
	while (used < capacity && buffer[used] != '\0') used++;
	if (used == capacity) {
		buffer[capacity - 1] = '\0';
		_warn_left_out(key);
		return;
	}

	const char * lead = (place == JSON_FIRST || place == JSON_ONLY) ? "{\"" : ",\"";
	const char * mid  = quote ? "\":\"" : "\":";
	const char * tail;
	if (quote) {
		tail = (place == JSON_LAST || place == JSON_ONLY) ? "\"}" : "\"";
	} else {
		tail = (place == JSON_LAST || place == JSON_ONLY) ? "}" : "";
	}

	const char * parts[5] = { lead, key, mid, value, tail };
	size_t need = 0;
	for (int i = 0; i < 5; i++) need += strlen(parts[i]);

	bool closing = (place == JSON_LAST || place == JSON_ONLY);
	if (used + need >= capacity) {
		_warn_left_out(key);
		if (closing && used > 0 && buffer[0] == '{' && buffer[used - 1] != '}' && used + 1 < capacity) {
			buffer[used] = '}';
			buffer[used + 1] = '\0';
		}
		return;
	}

	for (int i = 0; i < 5; i++) {
		size_t part_len = strlen(parts[i]);
		if (part_len == 0) continue;
		strlcpy(buffer + used, parts[i], capacity - used);
		used += part_len;
	}
}


// ---- bool: unquoted (true / false) ----

void JSON::first_key(char * buffer, size_t buffer_size, const char * key, bool value) {
	_append_field(buffer, buffer_size, JSON_FIRST, key, value ? "true" : "false", false);
}

void JSON::add_key(char * buffer, size_t buffer_size, const char * key, bool value) {
	_append_field(buffer, buffer_size, JSON_NEXT, key, value ? "true" : "false", false);
}

void JSON::last_key(char * buffer, size_t buffer_size, const char * key, bool value) {
	_append_field(buffer, buffer_size, JSON_LAST, key, value ? "true" : "false", false);
}

void JSON::one_key(char * buffer, size_t buffer_size, const char * key, bool value) {
	_append_field(buffer, buffer_size, JSON_ONLY, key, value ? "true" : "false", false);
}


// ---- float: unquoted (23.50) ----
// 64 bytes holds the longest float printed with two decimal places.

void JSON::first_key(char * buffer, size_t buffer_size, const char * key, float value) {
	char tmp[64];
	dtostrf(value, 0, 2, tmp);
	_append_field(buffer, buffer_size, JSON_FIRST, key, tmp, false);
}

void JSON::add_key(char * buffer, size_t buffer_size, const char * key, float value) {
	char tmp[64];
	dtostrf(value, 0, 2, tmp);
	_append_field(buffer, buffer_size, JSON_NEXT, key, tmp, false);
}

void JSON::last_key(char * buffer, size_t buffer_size, const char * key, float value) {
	char tmp[64];
	dtostrf(value, 0, 2, tmp);
	_append_field(buffer, buffer_size, JSON_LAST, key, tmp, false);
}

void JSON::one_key(char * buffer, size_t buffer_size, const char * key, float value) {
	char tmp[64];
	dtostrf(value, 0, 2, tmp);
	_append_field(buffer, buffer_size, JSON_ONLY, key, tmp, false);
}


// ---- int: unquoted (42) ----

void JSON::first_key(char * buffer, size_t buffer_size, const char * key, int value) {
	char tmp[16];
	itoa(value, tmp, 10);
	_append_field(buffer, buffer_size, JSON_FIRST, key, tmp, false);
}

void JSON::add_key(char * buffer, size_t buffer_size, const char * key, int value) {
	char tmp[16];
	itoa(value, tmp, 10);
	_append_field(buffer, buffer_size, JSON_NEXT, key, tmp, false);
}

void JSON::last_key(char * buffer, size_t buffer_size, const char * key, int value) {
	char tmp[16];
	itoa(value, tmp, 10);
	_append_field(buffer, buffer_size, JSON_LAST, key, tmp, false);
}

void JSON::one_key(char * buffer, size_t buffer_size, const char * key, int value) {
	char tmp[16];
	itoa(value, tmp, 10);
	_append_field(buffer, buffer_size, JSON_ONLY, key, tmp, false);
}


// ---- const char*: quoted ("hello") ----

void JSON::first_key(char * buffer, size_t buffer_size, const char * key, const char * value) {
	_append_field(buffer, buffer_size, JSON_FIRST, key, value, true);
}

void JSON::add_key(char * buffer, size_t buffer_size, const char * key, const char * value) {
	_append_field(buffer, buffer_size, JSON_NEXT, key, value, true);
}

void JSON::last_key(char * buffer, size_t buffer_size, const char * key, const char * value) {
	_append_field(buffer, buffer_size, JSON_LAST, key, value, true);
}

void JSON::one_key(char * buffer, size_t buffer_size, const char * key, const char * value) {
	_append_field(buffer, buffer_size, JSON_ONLY, key, value, true);
}
