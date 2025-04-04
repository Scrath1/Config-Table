#include "config_table.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#define KV_SEP_CHAR ':'
// If set to 1 will remove string delimiters (") from strings values parsed in parseKVStr
#define REMOVE_STRING_DELIMITERS (1)

CfgRet_t config_defaultLoadFunc(ConfigTable_t* cfg, const char* filename);
CfgRet_t config_defaultSaveFunc(const ConfigTable_t* cfg, const char* filename);

loadFromFileFunc loadFromFileFunction = config_defaultLoadFunc;
saveToFileFunc saveToFileFunction = config_defaultSaveFunc;

int32_t config_getIdxFromKey(const ConfigTable_t* cfg, const char* key) {
    if(cfg == NULL) return CFG_RC_ERROR_NULLPTR;
    for(int32_t i = 0; i < cfg->count; i++) {
        const ConfigEntry_t entry = cfg->entries[i];
        if(strcmp(key, entry.key) == 0) return i;
    }
    return -1;
}

CfgRet_t config_getByKey(const ConfigTable_t* cfg, const char* key, ConfigEntry_t* const entry) {
    if(cfg == NULL || key == NULL) return CFG_RC_ERROR_NULLPTR;
    const int32_t idx = config_getIdxFromKey(cfg, key);
    if(idx < 0) return CFG_RC_ERROR_UNKNOWN_KEY;

    return config_getByIdx(cfg, idx, entry);
}
CfgRet_t config_getByIdx(const ConfigTable_t* cfg, uint32_t idx, ConfigEntry_t* const entry) {
    if(cfg == NULL || entry == NULL) return CFG_RC_ERROR_NULLPTR;
    if(idx >= cfg->count) return CFG_RC_ERROR_RANGE;

    *entry = (cfg->entries[idx]);

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
    if(entry->perm == CFG_PERM_RO || entry->perm == CFG_PERM_SECRET_RO) return CFG_RC_ERROR_READ_ONLY;
    if(size > entry->size) return CFG_RC_ERROR_TOO_LARGE;
    memcpy(entry->value, value, size);
    // Fill remaining memory space with 0 to clear out possible leftover data
    memset(entry->value + size, 0, entry->size - size);
    return CFG_RC_SUCCESS;
}

/**
 * Type specific getter and setter functions
 * ===================================================================
 */

CfgRet_t config_getUint32ByKey(const ConfigTable_t* cfg, const char* key, uint32_t* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByKey(cfg, key, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_UINT32) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((uint32_t*)entry.value);
    return CFG_RC_SUCCESS;
}
CfgRet_t config_getUint32ByIdx(const ConfigTable_t* cfg, uint32_t idx, uint32_t* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByIdx(cfg, idx, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_UINT32) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((uint32_t*)entry.value);
    return CFG_RC_SUCCESS;
}

CfgRet_t config_getInt32ByKey(const ConfigTable_t* cfg, const char* key, int32_t* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByKey(cfg, key, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_INT32) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((int32_t*)entry.value);
    return CFG_RC_SUCCESS;
}
CfgRet_t config_getInt32ByIdx(const ConfigTable_t* cfg, uint32_t idx, int32_t* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByIdx(cfg, idx, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_INT32) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((int32_t*)entry.value);
    return CFG_RC_SUCCESS;
}

CfgRet_t config_getFloatByKey(const ConfigTable_t* cfg, const char* key, float* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByKey(cfg, key, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_FLOAT) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((float*)entry.value);
    return CFG_RC_SUCCESS;
}
CfgRet_t config_getFloatByIdx(const ConfigTable_t* cfg, uint32_t idx, float* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByIdx(cfg, idx, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_FLOAT) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((float*)entry.value);
    return CFG_RC_SUCCESS;
}

CfgRet_t config_getStringByKey(const ConfigTable_t* cfg, const char* key, char* str, uint32_t str_size) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByKey(cfg, key, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;


    // Check for possible type mismatch
    if(entry.type != CONFIG_STRING) return CFG_RC_ERROR_TYPE_MISMATCH;
    // Size check
    uint32_t stored_str_size = strlen(entry.value) + 1;
    if(stored_str_size > str_size) return CFG_RC_ERROR_TOO_LARGE;
    // Copy string
    strncpy(str, (char*)entry.value, str_size);
    return CFG_RC_SUCCESS;
}
CfgRet_t config_getStringByIdx(const ConfigTable_t* cfg, uint32_t idx, char* str, uint32_t str_size) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByIdx(cfg, idx, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_STRING) return CFG_RC_ERROR_TYPE_MISMATCH;
    // size check
    uint32_t stored_str_size = strlen(entry.value) + 1;
    if(stored_str_size > str_size) return CFG_RC_ERROR_TOO_LARGE;
    // copy string
    strncpy(str, (char*)entry.value, str_size);
    return CFG_RC_SUCCESS;
}

CfgRet_t config_getBoolByKey(const ConfigTable_t* cfg, const char* key, bool* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByKey(cfg, key, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_BOOL) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((bool*)entry.value);
    return CFG_RC_SUCCESS;
}
CfgRet_t config_getBoolByIdx(const ConfigTable_t* cfg, uint32_t idx, bool* value) {
    ConfigEntry_t entry;
    CfgRet_t ret = config_getByIdx(cfg, idx, &entry);
    if(CFG_RC_SUCCESS != ret) return ret;

    // Check for possible type mismatch
    if(entry.type != CONFIG_BOOL) return CFG_RC_ERROR_TYPE_MISMATCH;
    *value = *((bool*)entry.value);
    return CFG_RC_SUCCESS;
}

/**
 * Storage and parsing
 * ===================================================================
 */

CfgRet_t config_parseKVStr(ConfigTable_t* cfg, char* str, uint32_t len) {
    if(cfg == NULL || str == NULL) return CFG_RC_ERROR_NULLPTR;
    // First step, try to parse a key.
    // Find the index of the key-value separator
    char* value_str = strchr(str, KV_SEP_CHAR);
    if(value_str == NULL) return CFG_RC_ERROR_FORMAT; // separator char not found
    uint32_t sep_idx = (uint32_t)(value_str-str);
    // advance by one to omit the separator char from value string
    value_str++;

    // trim leading whitespace
    char* key_str = str;
    while(isspace(key_str[0])) key_str++;

    // Next look for a matching key
    // If no entry is found, cfg_entry_idx will stay at the value of cfg->count,
    // else it will be smaller
    uint32_t cfg_entry_idx = cfg->count;
    for(uint32_t i = 0; i < cfg->count; i++) {
        const char* cfg_key = cfg->entries[i].key;
        if(strncmp(cfg_key, key_str, sep_idx) == 0) {
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
    uint32_t value_str_size = len - sep_idx - 1;
    // Advance value string to get rid of possible whitespace
    while(isspace(value_str[0])) {
        value_str++;
        value_str_size--;
    }
    // trim trailing whitespace of value string
    while(isspace(value_str[value_str_size-2])) {
        value_str[value_str_size-2] = '\0';
        value_str_size--;
    }
    switch(entry->type) {
        default:
        case CONFIG_NONE:
            return CFG_RC_ERROR_INVALID;
        case CONFIG_UINT32: {
                if(value_str[0] == '-') return CFG_RC_ERROR;
                const uint32_t tmp = strtoull(value_str, NULL, 10);
                if(errno == ERANGE) {
                    errno = 0;
                    return CFG_RC_ERROR;
                }
                return config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
            }
        case CONFIG_INT32: {
                const int32_t tmp = strtol(value_str, NULL, 10);
                if(errno == ERANGE) {
                    errno = 0;
                    return CFG_RC_ERROR;
                }
                return config_setByIdx(cfg, cfg_entry_idx, &tmp, sizeof(tmp));
            }
        case CONFIG_FLOAT: {
                const float tmp = strtof(value_str, NULL);
                if(errno == ERANGE) {
                    errno = 0;
                    return CFG_RC_ERROR;
                }
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

CfgRet_t config_defaultLoadFunc(ConfigTable_t* cfg, const char* filename){
    if(cfg == NULL || filename == NULL) return CFG_RC_ERROR_NULLPTR;
    FILE* file_ptr = NULL;
    file_ptr = fopen(filename, "r");
    if(file_ptr == NULL) return CFG_RC_ERROR;

    char line[FILE_MAX_LINE_LEN] = "";
    bool parsing_error_occurred = false;
    // Read each line
    while(NULL != fgets(line, sizeof(line), file_ptr)) {
        uint32_t line_len = strlen(line);
        if(CFG_RC_SUCCESS != config_parseKVStr(cfg, line, line_len + 1)) {
            // If any line could not be matched to a valid entry, set a flag
            parsing_error_occurred = true;
        }
        // reset line
        line[0] = '\0';
    }
    fclose(file_ptr);

    if(parsing_error_occurred) return CFG_RC_ERROR_INCOMPLETE;
    return CFG_RC_SUCCESS;
}

CfgRet_t config_loadFromFile(ConfigTable_t* cfg, const char* filename) {
    if(loadFromFileFunction != NULL) return loadFromFileFunction(cfg, filename);
    else return CFG_RC_ERROR_INVALID;
}

CfgRet_t config_defaultSaveFunc(const ConfigTable_t* cfg, const char* filename){
    if(cfg == NULL || filename == NULL) return CFG_RC_ERROR_NULLPTR;
    FILE* file_ptr = NULL;
    file_ptr = fopen(filename, "w+");
    if(file_ptr == NULL) return CFG_RC_ERROR;

    char line[FILE_MAX_LINE_LEN] = "";
    // iterate over all config entries
    bool line_length_error = false;
    bool encoding_error = false;
    for(uint32_t i = 0; i < cfg->count; i++) {
        const ConfigEntry_t e = cfg->entries[i];
        int32_t ret;
        switch(e.type) {
            default:
            case CONFIG_NONE:
                    continue;
            case CONFIG_BOOL:
                ret = snprintf(line, sizeof(line), "%s: %u\n", e.key, *(bool*)e.value);
                break;
            case CONFIG_UINT32:
                ret = snprintf(line, sizeof(line), "%s: %lu\n", e.key, *(uint32_t*)e.value);
                break;
            case CONFIG_INT32:
                ret = snprintf(line, sizeof(line), "%s: %i\n", e.key, *(int32_t*)e.value);
                break;
            case CONFIG_FLOAT:
                ret = snprintf(line, sizeof(line), "%s: %f\n", e.key, *(float*)e.value);
                break;
            case CONFIG_STRING:
                ret = snprintf(line, sizeof(line), "%s: %s\n", e.key, e.value);
                break;
        }
        // Check if snprintf was successful
        if(ret > sizeof(line)) line_length_error = true;
        else if(ret < 0) encoding_error = true;
        else {
            // write to file
            fprintf(file_ptr, line);
        }
    }
    // close file
    fclose(file_ptr);

    if(encoding_error) return CFG_RC_ERROR_INVALID;
    if(line_length_error) return CFG_RC_ERROR_INCOMPLETE;
    return CFG_RC_SUCCESS;
}

CfgRet_t config_saveToFile(const ConfigTable_t* cfg, const char* filename) {
    if(saveToFileFunction != NULL) return saveToFileFunction(cfg, filename);
    else return CFG_RC_ERROR_INVALID;
}

void config_setSaveLoadFunctions(saveToFileFunc saveFunc, loadFromFileFunc loadFunc){
    if(saveFunc == NULL) saveToFileFunction = config_defaultSaveFunc;
    else saveToFileFunction = saveFunc;
    if(loadFunc == NULL) loadFromFileFunction = config_defaultLoadFunc;
    else loadFromFileFunction = loadFunc;
}