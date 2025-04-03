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

#ifndef FILE_MAX_LINE_LEN
    // Defines the maximum character length for the buffer
    // when printing a key-value config pair to a file
    #define FILE_MAX_LINE_LEN (256)
#endif

typedef enum {
    CFG_RC_ERROR_READ_ONLY = -9,      // The setting is read-only
    CFG_RC_ERROR_INCOMPLETE = -8,     // Operation was partially successful
    CFG_RC_ERROR_INVALID = -7,        // Invalid state detected
    CFG_RC_ERROR_FORMAT = -6,         // Error in string formatting detected
    CFG_RC_ERROR_TYPE_MISMATCH = -5,  // Requested type was incorrect for the value
    CFG_RC_ERROR_RANGE = -4,          // Given value was out of expected/valid range
    CFG_RC_ERROR_NULLPTR = -3,        // Unexpected nullptr
    CFG_RC_ERROR_TOO_LARGE = -2,      // The given value does not fit into the allocated memory
    CFG_RC_ERROR_UNKNOWN_KEY = -1,    // Unknown setting key
    CFG_RC_ERROR = 0,                 // Unspecified error
    CFG_RC_SUCCESS = 1,               // Success
} CfgRet_t;

/**
 * Flag for setting some permission context information for individual
 * settings. There is no way for the library to actually enforce this
 * beyond blocking the setter functions for read-only entries but
 * these permissions can be used to provide some additional context
 * for example while printing the configuration to the console to
 * exclude passwords.
 */
typedef enum {
    CFG_PERM_RW = 0, // read/write permissions
    CFG_PERM_RO = 1, // read-only
    CFG_PERM_SECRET_RW = 2, // read/write but entries with this flag should be
        // ignored for example when printing the settings somewhere
    CFG_PERM_SECRET_RO = 3, // read-only but entries with this flag should be
    // ignored for example when printing the settings somewhere
} CfgPermissions_t;

typedef enum { CONFIG_NONE = 0, CONFIG_UINT32, CONFIG_INT32, CONFIG_FLOAT, CONFIG_STRING, CONFIG_BOOL } ConfigType_t;

typedef struct {
    const char* key;
    ConfigType_t type;
    void* value;
    uint32_t size;
    CfgPermissions_t perm;
} ConfigEntry_t;

typedef struct {
    ConfigEntry_t* entries;
    uint32_t count;
} ConfigTable_t;

/**
 * Function pointer definition for overwriting the save function
 */
typedef CfgRet_t (*saveToFileFunc)(const ConfigTable_t* cfg, const char* filename);

/**
 * Function pointer definition for overwriting the load function
 */
typedef CfgRet_t (*loadFromFileFunc)(ConfigTable_t* cfg, const char* filename);

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
 * @warning This function returns a pointer to the configuration entry, not a copy.
 *  Changes to that pointer will result in changes to the actual stored configuration
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key string
 * @param entry [OUT] Pointer to the corresponding configuration entry
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 */
CfgRet_t config_getByKey(const ConfigTable_t* cfg, const char* key, ConfigEntry_t* const entry);

/**
 * Returns a configuration entry for the given index
 * @warning This function returns a pointer to the configuration entry, not a copy.
 *  Changes to that pointer will result in changes to the actual stored configuration
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param entry [OUT] Pointer to the corresponding configuration entry
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 */
CfgRet_t config_getByIdx(const ConfigTable_t* cfg, uint32_t idx, ConfigEntry_t* const entry);

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
 * @return CFG_RC_ERROR_READ_ONLY if the setting to change is read-only
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
 * @return CFG_RC_ERROR_READ_ONLY if the setting to change is read-only
 */
CfgRet_t config_setByIdx(ConfigTable_t* cfg, uint32_t idx, const void* value, uint32_t size);

/**
 * Type specific getter and setter functions
 * ===================================================================
 */

/**
 * Returns the uint32 value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key string
 * @param value [OUT] Pointer to a uint32_t where the config value should be stored
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
 * @param value [OUT] Pointer to a uint32_t where the config value should be stored
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
 * @param key [IN] Configuration key string
 * @param value [OUT] Pointer to a int32_t where the config value should be stored
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
 * @param value [OUT] Pointer to a int32_t where the config value should be stored
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
 * @param key [IN] Configuration key string
 * @param value [OUT] Pointer to a float where the config value should be stored
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
 * @param value [OUT] Pointer to a float where the config value should be stored
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
 * @param key [IN] Configuration key string
 * @param str [INOUT] string buffer of size str_size
 * @param str_size [IN] Maximum size of str string
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or key are NULL
 * @return CFG_RC_ERROR_UNKNOWN_KEY if no matching key was found
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 * @return CFG_RC_ERROR_TOO_LARGE if the stored string does not fit into the provided str parameter.
 *  In that case, no data will be written to str
 */
CfgRet_t config_getStringByKey(const ConfigTable_t* cfg, const char* key, char* str, uint32_t str_size);
/**
 * Returns the string for the given index if the type matches
 * @param cfg [IN] Configuration table
 * @param idx [IN] Index of the configuration entry in the config table
 * @param str [INOUT] string buffer of size str_size
 * @param str_size [IN] Maximum size of str string
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 * @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 * @return CFG_RC_ERROR_TOO_LARGE if the stored string does not fit into the provided str parameter.
 *  In that case, no data will be written to str
 */
CfgRet_t config_getStringByIdx(const ConfigTable_t* cfg, uint32_t idx, char* str, uint32_t str_size);

/**
 * Returns the bool value for the given key if the type matches
 * @param cfg [IN] Configuration table
 * @param key [IN] Configuration key string
 * @param value [OUT] Pointer to a bool where the config value should be stored
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
 * @param value [OUT] Pointer to a bool where the config value should be stored
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg is NULL
 * @return CFG_RC_ERROR_RANGE if the given index was larger than the
 *  number of entries in the configuration table
 *  @return CFG_RC_ERROR_TYPE_MISMATCH if the requested config entry has the wrong type
 */
CfgRet_t config_getBoolByIdx(const ConfigTable_t* cfg, uint32_t idx, bool* value);

/**
 * Storage and parsing
 * ===================================================================
 */

/**
 * Attempt to parse a key-value string into a configuration entry
 * with a matching key and type.
 * @param cfg [INOUT] Configuration table
 * @param str [IN] String of format "key: value" which will be attempted
 *  to be parsed into a configuration entry with a matching key.
 *  This string may be modified during parsing.
 *  Leading or trailing whitespace will be removed during parsing
 *
 * @note Parsing of booleans is done by checking the first character
 *  of the boolean value string for the characters 'T' 't' 'F' 'f' '1' '0'
 * @param len [IN] size of str string including null-terminator
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR if parsing of value failed
 * @return CFG_RC_ERROR_INVALID if the entry with a matching key has no type associated with it
 * @return CFG_RC_ERROR_NULLPTR if cfg or str is NULL
 * @return CFG_RC_ERROR_FORMAT if the separator between key and value was not found
 * @return CFG_RC_ERROR_UNKNOWN_KEY if the key provided in str was not found
 * @return any error caused by config_setByIdx
 */
CfgRet_t config_parseKVStr(ConfigTable_t* cfg, char* str, uint32_t len);

/**
 * Attempts to read configuration entries from a file
 * @param cfg [INOUT] Configuration table where matching key-value pairs will be stored
 * @param filename [IN] Name of the file to read for config values
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or filename are NULL
 * @return CFG_RC_ERROR if the file could not be opened
 * @return CFG_RC_ERROR_INCOMPLETE if there was an entry in the file which could not be matched
 *  to a configuration entry. Other entries have still been loaded.
 */
CfgRet_t config_loadFromFile(ConfigTable_t* cfg, const char* filename);
/**
 * Attempts to save configuration entries to a file
 *
 * @note This function can be overwritten with a custom implementation
 * @warning The contents of the target file will be overwritten if it already exists
 * @param cfg [IN] Configuration table
 * @param filename [IN] Name of the file where config entries should be stored
 * @return CFG_RC_SUCCESS on success
 * @return CFG_RC_ERROR_NULLPTR if cfg or filename are NULL
 * @return CFG_RC_ERROR if the file could not be opened
 * @return CFG_RC_ERROR_INCOMPLETE if any config entry could not be written to the
 *  file due to its size being too large for the internal string buffer.
 *  The internal string buffer is defined by FILE_MAX_LINE_LEN and can be
 *  overwritten using a compiler flag
 * @return CFG_RC_ERROR_INVALID if an encoding error has occurred while using
 *  snprintf.
 */
CfgRet_t config_saveToFile(const ConfigTable_t* cfg, const char* filename);

/**
 * Sets a new function for saving and loading configuration data to and from
 * files.
 * @param saveFunc [IN] Pointer to the new save function. Pass NULL to reset
 *  to default function
 * @param loadFunc [IN] Pointer to the new load function. Pass NULL to reset
 *  to default function
 */
void config_setSaveLoadFunctions(saveToFileFunc saveFunc, loadFromFileFunc loadFunc);

#ifdef __cplusplus
}
#endif
#endif  // CONFIG_TABLE_H
