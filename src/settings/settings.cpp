

#include "settings/settings.h"
#include "Arduino.h"
#include "Preferences.h"

static Preferences _prefs;

Settings::Settings() {}

Settings settings;



// ============================================================
//  REGISTRATION
// ============================================================

void Settings::bind_int(const char* key, int32_t* ptr) {
    _add(key, SETTING_INT, (void*)ptr, sizeof(int32_t));
}

void Settings::bind_float(const char* key, float* ptr) {
    _add(key, SETTING_FLOAT, (void*)ptr, sizeof(float));
}

void Settings::bind_bool(const char* key, bool* ptr) {
    _add(key, SETTING_BOOL, (void*)ptr, sizeof(bool));
}

void Settings::bind_string(const char* key, char* ptr, size_t size) {
    _add(key, SETTING_STRING, (void*)ptr, size);
}


void Settings::_add(const char* key, SettingType type, void* ptr, size_t size) {

    if (_count >= SETTINGS_MAX_BINDINGS) {
        Serial.print("\txx settings: max bindings reached (");
        Serial.print(SETTINGS_MAX_BINDINGS);
        Serial.println(")");
        return;
    }

    if (strlen(key) > SETTINGS_MAX_KEY_LEN - 1) {
        Serial.print("\txx settings: key '");
        Serial.print(key);
        Serial.print("' exceeds ");
        Serial.print(SETTINGS_MAX_KEY_LEN - 1);
        Serial.println(" chars — will be truncated");
    }

    Binding& b = _bindings[_count];

    strncpy(b.key, key, SETTINGS_MAX_KEY_LEN - 1);
    b.key[SETTINGS_MAX_KEY_LEN - 1] = '\0';
    b.type = type;
    b.ptr  = ptr;
    b.size = size;

    _capture_default(_count);

    _count++;
}


void Settings::_capture_default(int index) {
    _to_string(index, _bindings[index].default_str, SETTINGS_MAX_STR_LEN);
}



// ============================================================
//  LOAD
// ============================================================

void Settings::load() {

    Serial.println("\n-- settings.load()");

    _prefs.begin(SETTINGS_NAMESPACE, false);

    for (int i = 0; i < _count; i++) {

        if (_prefs.isKey(_bindings[i].key)) {

            char nvs_value[SETTINGS_MAX_STR_LEN];
            _prefs.getString(_bindings[i].key, nvs_value, SETTINGS_MAX_STR_LEN);
            _apply(i, nvs_value);

            Serial.print("\t.. '");
            Serial.print(_bindings[i].key);
            Serial.print("' loaded from NVS: ");
            Serial.println(nvs_value);

        } else {

            _prefs.putString(_bindings[i].key, _bindings[i].default_str);

            Serial.print("\t.. '");
            Serial.print(_bindings[i].key);
            Serial.print("' using default: ");
            Serial.println(_bindings[i].default_str);
        }
    }

    _prefs.end();

    Serial.println("-- settings.load() complete\n");
}



// ============================================================
//  SET
// ============================================================

bool Settings::set(const char* key, const char* value) {

    int index = _find(key);
    if (index < 0) return false;

    _apply(index, value);

    Serial.print("\t.. setting updated '");
    Serial.print(key);
    Serial.print("' = ");
    Serial.println(value);

    return true;
}



// ============================================================
//  CLEAR
// ============================================================

bool Settings::clear(const char* key) {

    int index = _find(key);
    if (index < 0) return false;

    _apply(index, _bindings[index].default_str);

    Serial.print("\t.. setting '");
    Serial.print(key);
    Serial.print("' reset to default: ");
    Serial.println(_bindings[index].default_str);

    return true;
}


void Settings::clear_all() {

    Serial.println("\t.. clearing all settings to defaults");

    _prefs.begin(SETTINGS_NAMESPACE, false);
    _prefs.clear();
    _prefs.end();

    for (int i = 0; i < _count; i++) {
        _apply(i, _bindings[i].default_str);
    }
}



// ============================================================
//  UTILITY
// ============================================================

bool Settings::has(const char* key) {
    return _find(key) >= 0;
}


void Settings::get_as_string(const char* key, char* buffer, size_t buffer_size) {
    int index = _find(key);
    if (index < 0) {
        buffer[0] = '\0';
        return;
    }
    _to_string(index, buffer, buffer_size);
}


int Settings::count() {
    return _count;
}


void Settings::print() {

    Serial.println("\n\t== Settings ==");

    for (int i = 0; i < _count; i++) {

        char current[SETTINGS_MAX_STR_LEN];
        _to_string(i, current, SETTINGS_MAX_STR_LEN);

        const char* type_label;
        switch (_bindings[i].type) {
            case SETTING_INT:    type_label = "int";    break;
            case SETTING_FLOAT:  type_label = "float";  break;
            case SETTING_BOOL:   type_label = "bool";   break;
            case SETTING_STRING: type_label = "string"; break;
        }

        Serial.print("\t   ");
        Serial.print(_bindings[i].key);
        Serial.print(" = ");
        Serial.print(current);
        Serial.print("  (");
        Serial.print(type_label);
        Serial.print(", default: ");
        Serial.print(_bindings[i].default_str);
        Serial.println(")");
    }

    Serial.println("\t=================\n");
}



// ============================================================
//  PRIVATE
// ============================================================

void Settings::_apply(int index, const char* str_value) {

    Binding& b = _bindings[index];

    switch (b.type) {

        case SETTING_INT:
            *(int32_t*)b.ptr = atoi(str_value);
            break;

        case SETTING_FLOAT:
            *(float*)b.ptr = atof(str_value);
            break;

        case SETTING_BOOL:
            *(bool*)b.ptr = (
                strcmp(str_value, "1")    == 0 ||
                strcmp(str_value, "true") == 0 ||
                strcmp(str_value, "on")   == 0
            );
            break;

        case SETTING_STRING:
            strncpy((char*)b.ptr, str_value, b.size - 1);
            ((char*)b.ptr)[b.size - 1] = '\0';
            break;
    }

    _prefs.begin(SETTINGS_NAMESPACE, false);
    _prefs.putString(b.key, str_value);
    _prefs.end();
}


void Settings::_to_string(int index, char* buffer, size_t buffer_size) {

    Binding& b = _bindings[index];

    switch (b.type) {
        case SETTING_INT:
            snprintf(buffer, buffer_size, "%d", *(int32_t*)b.ptr);
            break;
        case SETTING_FLOAT:
            snprintf(buffer, buffer_size, "%.4f", *(float*)b.ptr);
            break;
        case SETTING_BOOL:
            snprintf(buffer, buffer_size, "%s", *(bool*)b.ptr ? "true" : "false");
            break;
        case SETTING_STRING:
            strncpy(buffer, (char*)b.ptr, buffer_size - 1);
            buffer[buffer_size - 1] = '\0';
            break;
    }
}


int Settings::_find(const char* key) {
    for (int i = 0; i < _count; i++) {
        if (strcmp(_bindings[i].key, key) == 0) return i;
    }
    return -1;
}