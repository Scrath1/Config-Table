# Config-Table
Config Table is a library written in C for persisting runtime configuration options
on embedded systems. By default the library is written for compatibility with desktop filesystems.
Depending on the filesystem used, the save and load functions have to be overwritten with
user-defined functions. Look below for an example for LittleFS.

## Installation
Simply add the contents of the `include` and `src` directories to your project.

## Usage
For a barebones example of how you can use this library look at the [basic-example.cpp](examples/basic-example.cpp)
file. The recommended way of using this however is in addition to a user defined configuration
struct as seen in the [example_with_config_struct.cpp](examples/example_with_config_struct.cpp) file
to allow easy and structured access to configuration values.

```C++
// You can define your configuration structure with default values like this
struct Configuration {
    struct {
        char ssid[128] = "";
        char password[128] = "";
    } wifi;
    uint32_t baud_rate = 9600;
    uint32_t execution_counter = 0;
};
// Followed by an instantiation of your struct for actual use
Configuration cfg;

// And then link the config entries to that instantiated struct
ConfigEntry_t config_entries[] = {
    {STRINGIFY(cfg.wifi.ssid), CONFIG_STRING, &cfg.wifi.ssid, sizeof(cfg.wifi.ssid)},
    {STRINGIFY(cfg.wifi.password), CONFIG_STRING, &cfg.wifi.password, sizeof(cfg.wifi.password)},
    {STRINGIFY(cfg.baud_rate), CONFIG_UINT32, &cfg.baud_rate, sizeof(cfg.baud_rate)},
    {STRINGIFY(cfg.execution_counter), CONFIG_UINT32, &cfg.execution_counter, sizeof(cfg.execution_counter)}
};

// Finally make the actual config table which also stores the number of config entries you have
ConfigTable_t config_table{
    .entries = config_entries,
    .count = sizeof(config_entries) / sizeof(config_entries[0])
};
```

## Example load and save functions for LittleFS
The following functions are examples for usage with the embedded filesystem LittleFS.
They are identical to the default load and save functions beside their usage of LittleFS
for opening and reading/writing the files. Please note that LittleFS requires absolute
file paths, e.g. `/config.txt` instead of `config.txt` as the given filename.
```C++
CfgRet_t littleFSSaveToFile(const ConfigTable_t* cfg, const char* filename) {
    if(cfg == NULL || filename == NULL) return CFG_RC_ERROR_NULLPTR;
    fs::File file = LittleFS.open(filename, FILE_WRITE);
    if(!file) return CFG_RC_ERROR;
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
        if(ret > sizeof(line))
            line_length_error = true;
        else if(ret < 0)
            encoding_error = true;
        else {
            // write to file
            file.print(line);
        }
    }
    // close file
    file.close();

    if(encoding_error) return CFG_RC_ERROR_INVALID;
    if(line_length_error) return CFG_RC_ERROR_INCOMPLETE;
    return CFG_RC_SUCCESS;
}
```
```C++
CfgRet_t littleFSloadFromFile(ConfigTable_t* cfg, const char* filename) {
    if(cfg == NULL || filename == NULL) return CFG_RC_ERROR_NULLPTR;
    fs::File file = LittleFS.open(filename, FILE_READ);
    if(!file) return CFG_RC_ERROR;

    String lineStr = "";
    char line[FILE_MAX_LINE_LEN] = "";
    bool parsing_error_occurred = false;
    // Read each line
    while(file.available()) {
        lineStr = file.readStringUntil('\n');
        uint32_t line_len = lineStr.length();
        if(line_len == 0) continue;

        // copy into c string for compatibility with library functions
        strncpy(line, lineStr.c_str(), lineStr.length() + 1);
        if(CFG_RC_SUCCESS != config_parseKVStr(cfg, line, line_len + 1)) {
            // If any line could not be matched to a valid entry, set a flag
            parsing_error_occurred = true;
        }
        // reset line
        line[0] = '\0';
    }
    file.close();

    if(parsing_error_occurred) return CFG_RC_ERROR_INCOMPLETE;
    return CFG_RC_SUCCESS;
}
```