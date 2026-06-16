

#include "json/json.h"
#include "Arduino.h"

JSON::JSON() {}

JSON json;

// ---- bool: unquoted (true / false) ----

void JSON::first_key(char * buffer, const char * key, bool value) {
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, value ? "true" : "false");
}

void JSON::add_key(char * buffer, const char * key, bool value) {
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, value ? "true" : "false");
}

void JSON::last_key(char * buffer, const char * key, bool value) {
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, value ? "true" : "false");
	strcat(buffer, "}");
}

void JSON::one_key(char * buffer, const char * key, bool value) {
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, value ? "true" : "false");
	strcat(buffer, "}");
}


// ---- float: unquoted (23.50) ----

void JSON::first_key(char * buffer, const char * key, float value) {
	char tmp[32];
	dtostrf(value, 0, 2, tmp);
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, tmp);
}

void JSON::add_key(char * buffer, const char * key, float value) {
	char tmp[32];
	dtostrf(value, 0, 2, tmp);
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, tmp);
}

void JSON::last_key(char * buffer, const char * key, float value) {
	char tmp[32];
	dtostrf(value, 0, 2, tmp);
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, tmp);
	strcat(buffer, "}");
}

void JSON::one_key(char * buffer, const char * key, float value) {
	char tmp[32];
	dtostrf(value, 0, 2, tmp);
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":");
	strcat(buffer, tmp);
	strcat(buffer, "}");
}


// ---- const char*: quoted ("hello") ----

void JSON::first_key(char * buffer, const char * key, const char * value) {
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":\"");
	strcat(buffer, value);
	strcat(buffer, "\"");
}

void JSON::add_key(char * buffer, const char * key, const char * value) {
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":\"");
	strcat(buffer, value);
	strcat(buffer, "\"");
}

void JSON::last_key(char * buffer, const char * key, const char * value) {
	strcat(buffer, ",\"");
	strcat(buffer, key);
	strcat(buffer, "\":\"");
	strcat(buffer, value);
	strcat(buffer, "\"}");
}

void JSON::one_key(char * buffer, const char * key, const char * value) {
	strcat(buffer, "{\"");
	strcat(buffer, key);
	strcat(buffer, "\":\"");
	strcat(buffer, value);
	strcat(buffer, "\"}");
}