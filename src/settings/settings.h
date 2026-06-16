#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>
#include <stddef.h>

#define SETTINGS_MAX_BINDINGS   30
#define SETTINGS_MAX_KEY_LEN    16      // NVS limit: 15 chars + null
#define SETTINGS_MAX_STR_LEN    64
#define SETTINGS_NAMESPACE      "settings"


enum SettingType { SETTING_INT, SETTING_FLOAT, SETTING_BOOL, SETTING_STRING };


class Settings {

    public:

        Settings();

        // --- registration (call in setup, before load) ---

        void bind_int    (const char* key, int32_t* ptr);
        void bind_float  (const char* key, float*   ptr);
        void bind_bool   (const char* key, bool*    ptr);
        void bind_string (const char* key, char*    ptr, size_t size);

        // --- lifecycle ---

        void load();                                        // call once after all binds
        bool set(const char* key, const char* value);       // called by your controller on MQTT update
        bool clear(const char* key);                        // reset one key to compiled default, remove from NVS
        void clear_all();                                   // reset everything to compiled defaults, wipe NVS namespace

        // --- utility ---

        bool  has(const char* key);
        void  get_as_string(const char* key, char* buffer, size_t buffer_size);
        int   count();
        void  print();


    private:

        struct Binding {
            char            key[SETTINGS_MAX_KEY_LEN];
            SettingType     type;
            void*           ptr;
            size_t          size;           // buffer size for strings
            char            default_str[SETTINGS_MAX_STR_LEN];  // compiled default, captured at bind time
        };

        Binding _bindings[SETTINGS_MAX_BINDINGS];
        int     _count = 0;

        int     _find(const char* key);
        void    _add(const char* key, SettingType type, void* ptr, size_t size);
        void    _apply(int index, const char* str_value);
        void    _to_string(int index, char* buffer, size_t buffer_size);
        void    _capture_default(int index);

};


extern Settings settings;

#endif
