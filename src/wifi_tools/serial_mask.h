#pragma once

// serial_mask.h
//
// Stand-ins for secrets that would otherwise be printed on the serial
// monitor.  The real value is never modified — callers print the stand-in
// and keep using the original for Wi-Fi, MQTT, and OTA.
//
//   empty            ->  "(not set)"
//   1 to 4 chars     ->  "(set)"          a tail would reveal most of it
//   5 to 7 chars     ->  "****" + last 2  e.g. ****ab
//   8 or more chars  ->  "****" + last 4  e.g. ****ab12
//
// Not part of the sketch API.  The modules that log a secret include this
// themselves.

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>


static inline void mask_secret_n(const char * secret, size_t len, char * out, size_t out_len) {
    if (out_len == 0) return;
    if (!secret || len == 0) {
        snprintf(out, out_len, "(not set)");
        return;
    }
    if (len <= 4) {
        snprintf(out, out_len, "(set)");
        return;
    }
    size_t tail = (len >= 8) ? 4 : 2;
    if (out_len < 4 + tail + 1) {
        snprintf(out, out_len, "(set)");
        return;
    }
    memcpy(out, "****", 4);
    memcpy(out + 4, secret + (len - tail), tail);
    out[4 + tail] = '\0';
}


static inline void mask_secret(const char * secret, char * out, size_t out_len) {
    mask_secret_n(secret, secret ? strlen(secret) : 0, out, out_len);
}


// A settings key, command name, or HTTP header is treated as a secret when
// one of its pieces (split on '_' or '-') is a password / token / key word.
// "sleep_secs" and "threshold" are not.  "api_token" and "X-Api-Key" are.
// Comparison ignores case.
static inline bool _secret_word(const char * s, size_t n) {
    static const char * words[] = {
        "pass", "password", "passwd", "pwd",
        "token", "secret",
        "auth", "authkey", "authorization",
        "apikey",
        "credential", "credentials",
        "cookie",
        "key",
        nullptr
    };
    if (n == 0 || n > 31) return false;
    char buf[32];
    for (size_t i = 0; i < n; i++) buf[i] = (char)tolower((unsigned char)s[i]);
    buf[n] = '\0';
    for (size_t w = 0; words[w] != nullptr; w++) {
        if (strcmp(buf, words[w]) == 0) return true;
    }
    return false;
}


static inline bool name_is_secret_n(const char * name, size_t n) {
    if (!name || n == 0) return false;
    size_t i = 0;
    while (i < n) {
        size_t start = i;
        while (i < n && name[i] != '_' && name[i] != '-') i++;
        if (_secret_word(name + start, i - start)) return true;
        if (i < n) i++;   // step over the separator
    }
    return false;
}


static inline bool name_is_secret(const char * name) {
    if (!name) return false;
    return name_is_secret_n(name, strlen(name));
}


// Replace secret query values (auth_key=, token=, password=, ...) inside a
// URL or any other text.  Returns false when nothing secret was found, so
// the caller can print the original line unchanged.  `out` is always
// NUL-terminated.
static inline bool _is_name_char(char c) {
    return isalnum((unsigned char)c) || c == '_' || c == '-';
}

static inline bool mask_text_secrets(const char * text, char * out, size_t out_len) {
    if (out_len == 0) return false;
    if (!text) { out[0] = '\0'; return false; }

    bool hid = false;
    size_t o = 0;
    const char * p = text;

    while (*p) {
        const char * eq = strchr(p, '=');
        if (!eq) {
            while (*p && o + 1 < out_len) out[o++] = *p++;
            break;
        }

        const char * name_end = eq;
        const char * name_start = eq;
        while (name_start > p && _is_name_char(name_start[-1])) name_start--;
        bool secret = name_is_secret_n(name_start, (size_t)(name_end - name_start));

        while (p <= eq && o + 1 < out_len) out[o++] = *p++;
        if (p <= eq) p = eq + 1;   // output full; still skip to the value

        if (!secret) continue;

        hid = true;
        const char * v = p;
        while (*p && *p != '&' && *p != '#' && *p != ';' && *p != '"' && *p != '\''
               && !isspace((unsigned char)*p)) {
            p++;
        }
        char masked[16];
        mask_secret_n(v, (size_t)(p - v), masked, sizeof(masked));
        for (size_t k = 0; masked[k] && o + 1 < out_len; k++) out[o++] = masked[k];
    }

    out[o] = '\0';
    return hid;
}


// Settings echoes are one JSON field: {"threshold":"2.5"}.
// When that name is a secret, write a masked copy for the serial log.
// Returns false when the line should be printed unchanged.
// `out` is always NUL-terminated when this returns true.
static inline bool mask_json_secret(const char * payload, char * out, size_t out_len) {
    if (!payload || out_len == 0) return false;
    if (payload[0] != '{' || payload[1] != '"') return false;

    const char * key_start = payload + 2;
    const char * key_end = strchr(key_start, '"');
    if (!key_end || key_end == key_start) return false;

    size_t key_len = (size_t)(key_end - key_start);
    if (!name_is_secret_n(key_start, key_len)) return false;

    // From here on the name is a secret, so never tell the caller to print
    // the raw payload — even if the value is not a plain quoted string.
    const char * val_start = nullptr;
    size_t val_len = 0;
    if (key_end[1] == ':' && key_end[2] == '"') {
        val_start = key_end + 3;
        const char * val_end = strchr(val_start, '"');
        if (val_end) val_len = (size_t)(val_end - val_start);
        else val_start = nullptr;
    }

    char masked[16];
    if (val_start) mask_secret_n(val_start, val_len, masked, sizeof(masked));
    else snprintf(masked, sizeof(masked), "(set)");

    int n = snprintf(out, out_len, "{\"%.*s\":\"%s\"}", (int)key_len, key_start, masked);
    if (n < 0 || (size_t)n >= out_len) {
        snprintf(out, out_len, "(set)");
    }
    return true;
}
