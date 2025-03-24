#include "config_table.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define KV_SEP_CHAR ':'
// If set to 1 will remove string delimiters (") from strings values parsed in parseKVStr
#define REMOVE_STRING_DELIMITERS (1)

int32_t config_getIdxFromKey(const ConfigTable_t* cfg, const char* key) {
    if(cfg == NULL) return CFG_RC_ERROR_NULLPTR;
    for(int32_t i = 0; i < cfg->count; i++) {
        const ConfigEntry_t* entry_ptr = &(cfg->entries[i]);
        if(strcmp(key, entry_ptr->key) == 0) return i;
    }
    return -1;
}

CfgRet_t config_getByKey(const ConfigTable_t* cfg, const char* key, ConfigEntry_t** const entry) {
    if(cfg == NULL || key == NULL) return CFG_RC_ERROR_NULLPTR;
    const int32_t idx = config_getIdxFromKey(cfg, key);
    if(idx < 0) return CFG_RC_ERROR_UNKNOWN_KEY;

    return config_getByIdx(cfg, idx, entry);
}
CfgRet_t config_getByIdx(const ConfigTable_t* cfg, uint32_t idx, ConfigEntry_t** const entry) {
    if(cfg == NULL) return CFG_RC_ERROR_NULLPTR;
    if(idx >= cfg->count) return CFG_RC_ERROR_RANGE;

    *entry = &(cfg->entries[idx]);

    return CFG_RC_SUCCESS;
}
CfgRet_t config_setByKey(ConfigTable_t* cfg, const char* key, const void* value, uint32_t size) {
    if(cfg == NULL || value == NULL) return CFG_RC_ERROR_NULLPTR;
    const int32_t idx = config_getIdxFromKey(cfg, key);
    if(idx < 0) return CFG_RC_ERROR_UNKNOWN_KEY;

    return config_setByIdx(cfg, idx, value, size);
}
CfgRet_t config_setByIdx(ConfigTable_t* cfg, uint32_t idx, const void* value, uint32_t size) {
    if(cfg == NULL || value == NULL) return CFG_RC_ERROR_NULLPTR;
    if(idx >= cfg->count) return CFG_RC_ERROR_RANGE;
    ConfigEntry_t* entry = &(cfg->entries[idx]);
    if(size > entry->size) return CFG_RC_ERROR_TOO_LARGE;
    memcpy(entry->value, value, size);
    // Fill remaining memory space with 0 to clear out possible leftover data
    memset(entry->value + size, 0, entry->size - size);
    return CFG_RC_SUCCESS;
}

CfgRet_t config_parseKVStr(ConfigTable_t* cfg, char* str, uint32_t len) {
    if(cfg == NULL) return CFG_RC_ERROR_NULLPTR;
    // First step, try to parse a key.
    // Find the index of the key-value separator
    char* value_str = strchr(str, KV_SEP_CHAR);
    if(value_str == NULL) return CFG_RC_ERROR; // separator char not found
    uint32_t sep_idx = (uint32_t)(value_str-str);
    // advance by one to omit the separator char from value string
    value_str++;

    // Next look for a matching key
    // If no entry is found, cfg_entry_idx will stay at the value of cfg->count,
    // else it will be smaller
    uint32_t cfg_entry_idx = cfg->count;
    for(uint32_t i = 0; i < cfg->count; i++) {
        const char* cfg_key = cfg->entries[i].key;
        if(strncmp(cfg_key, str, sep_idx) == 0) {
            cfg_entry_idx = i;
            break;
        }
    }
    if(cfg_entry_idx == cfg->count){
        // Key does not exist in config
        return CFG_RC_ERROR_UNKNOWN_KEY;
    }
    ConfigEntry_t* entry = &(cfg->entries[cfg_entry_idx]);
    // Parse variable to correct type
    // 8 byte should be enough for any non-string type commonly used.
    // Strings do not need to be stored here
    uint32_t value_str_size = len - sep_idx - 1; // ToDo: Check! May be off by one?
    // Advance value string to get rid of possible whitespace
    while(isspace(value_str[0])) {
        value_str++;
        value_str_size--;
    }
    switch(entry->type) {
        default:
        case CONFIG_NONE:
            return CFG_RC_ERROR;
        case CONFIG_UINT32: {
                const uint32_t tmp = strtoull(value_str, NULL, 10);
                if(errno == ERANGE) return CFG_RC_ERROR;
                return config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
            }
        case CONFIG_INT32: {
                const int32_t tmp = strtol(value_str, NULL, 10);
                if(errno == ERANGE) return CFG_RC_ERROR;
                return config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
            }
        case CONFIG_FLOAT: {
                const float tmp = strtof(value_str, NULL);
                if(errno == ERANGE) return CFG_RC_ERROR;
                return config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
            }
        case CONFIG_STRING:
            if(value_str[0] == '"' && REMOVE_STRING_DELIMITERS) {
                value_str[value_str_size-2] = '\0';
                value_str++;
                value_str_size -= 2;
            }
            return config_setByIdx(cfg, cfg_entry_idx, value_str, value_str_size);
        case CONFIG_BOOL: {
                char bool_char = value_str[0];
                if(bool_char == 'T' || bool_char == 't' || bool_char == '1') {
                    uint8_t tmp = 1;
                    config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
                }
                else if(bool_char == 'F' || bool_char == 'f' || bool_char == '0') {
                    uint8_t tmp = 0;
                    config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
                }
                else return CFG_RC_ERROR;
            }
            break;
    }
    return CFG_RC_SUCCESS;
}