#ifndef CONFIG_TABLE_H
#define CONFIG_TABLE_H
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
// Ideas:
// Callbacks on changes
// Persistent storage support
// Default values
// JSON/CLI Adapter
// RW Permissions

typedef enum {
    CFG_RC_ERROR_TYPE_MISMATCH = -5,  // Requested type was incorrect for the value
    CFG_RC_ERROR_RANGE = -4,          // Given value was out of expected/valid range
    CFG_RC_ERROR_NULLPTR = -3,        // Unexpected nullptr
    CFG_RC_ERROR_TOO_LARGE = -2,      // The given value does not fit into the allocated memory
    CFG_RC_ERROR_UNKNOWN_KEY = -1,    // Unknown setting key
    CFG_RC_ERROR = 0,                 // Unspecified error
    CFG_RC_SUCCESS = 1,               // Success
} CfgRet_t;

typedef enum { CONFIG_NONE = 0, CONFIG_UINT32, CONFIG_INT32, CONFIG_FLOAT, CONFIG_STRING, CONFIG_BOOL } ConfigType_t;

typedef struct {
    const char* key;
    ConfigType_t type;
    void* value;
    uint32_t size;
} ConfigEntry_t;

typedef struct {
    ConfigEntry_t* entries;
    uint32_t count;
} ConfigTable_t;

/**
 * Searches for the given key in the config table and returns the corresponding
 * index if it exists.
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key
 * @return Index of configuration entry matching the key or -1 if no matching key was found
 */
int32_t config_getIdxFromKey(const ConfigTable_t* cfg, const char* key);

/**
 * Returns a configuration entry for the given key
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key string
 * @param entry [OUT] Pointer to the corresponding configuration entry
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 */
CfgRet_t config_getByKey(const ConfigTable_t* cfg, const char* key, ConfigEntry_t** const entry);

/**
 * Returns a configuration entry for the given index
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param entry [OUT] Pointer to the corresponding configuration entry
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 */
CfgRet_t config_getByIdx(const ConfigTable_t* cfg, uint32_t idx, ConfigEntry_t** const entry);

/**
 * Sets a configuration entry value based on the given key
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key string
 * @param value [IN] Value to write to the configuration entry
 * @param size [IN] size of value in bytes
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TOO_LARGE if the given value does not
 *  fit into the allocated memory for the configuration value
 */
CfgRet_t config_setByKey(ConfigTable_t* cfg, const char* key, const void* value, uint32_t size);

/**
 * Sets a configuration entry value based on the given index
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [IN] Value to write to the configuration entry
 * @param size [IN] size of value in bytes
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 * @return CFG_RC_ERROR_TOO_LARGE if the given value does not
 *  fit into the allocated memory for the configuration value
 */
CfgRet_t config_setByIdx(ConfigTable_t* cfg, uint32_t idx, const void* value, uint32_t size);

/**
 *
 * @param cfg [INOUT] Configuration table
 * @param str [IN] String of format "key: value" which will be attempted
 *  to be parsed into a configuration entry with a matching key.
 *  This string may be modified during parsing
 * @param len [IN] size of str string including null-terminator
 * @return
 */
CfgRet_t config_parseKVStr(ConfigTable_t* cfg, char* str, uint32_t len);

/**
 * Type specific getter and setter functions
 * ===================================================================
 */

/**
 * Returns the uint32 value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getUint32ByKey(const ConfigTable_t* cfg, const char* key, uint32_t* value);
/**
 * Returns the uint32 value for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getUint32ByIdx(const ConfigTable_t* cfg, uint32_t idx, uint32_t* value);

/**
 * Returns the int32 value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getInt32ByKey(const ConfigTable_t* cfg, const char* key, int32_t* value);
/**
 * Returns the int32 value for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getInt32ByIdx(const ConfigTable_t* cfg, uint32_t idx, int32_t* value);

/**
 * Returns the float value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getFloatByKey(const ConfigTable_t* cfg, const char* key, float* value);
/**
 * Returns the float value for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getFloatByIdx(const ConfigTable_t* cfg, uint32_t idx, float* value);

/**
 * Returns the string for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param str [OUT] Pointer to the string with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getStringByKey(const ConfigTable_t* cfg, const char* key, char** str);
/**
 * Returns the string for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param str [OUT] Pointer to the string with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getStringByIdx(const ConfigTable_t* cfg, uint32_t idx, char** str);

/**
 * Returns the bool value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getBoolByKey(const ConfigTable_t* cfg, const char* key, bool* value);
/**
 * Returns the bool value for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param value [OUT] Pointer to value with correct typecast applied
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getBoolByIdx(const ConfigTable_t* cfg, uint32_t idx, bool* value);

#ifdef __cplusplus
}
#endif
#endif  // CONFIG_TABLE_H
