#include <cstdint>
#include <iostream>

#include "config_table.h"

#define _STRINGIFY(s) #s
#define STRINGIFY(s) _STRINGIFY(s)

// File where configuration changes are stored
#define CONFIG_FILE "example_cfg_file.cfg"

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

void print_config(const ConfigTable_t& cfg_table) {
    for(size_t i = 0; i < cfg_table.count; i++) {
        const auto& e = cfg_table.entries[i];
        std::cout << e.key << ": ";
        switch(e.type) {
            default:
            case CONFIG_UINT32:
            case CONFIG_INT32:
            case CONFIG_BOOL:
                std::cout << *(int32_t*)(e.value);
            break;
            case CONFIG_FLOAT:
                std::cout << *(float*)(e.value);
            break;
            case CONFIG_STRING:
                std::cout << (char*)e.value;
            break;
        }
        std::cout << ", var_size: " << e.size << std::endl;
    }
}

int main() {
    // Start your program by loading a possibly existing configuration file
    config_loadFromFile(&config_table, CONFIG_FILE);
    print_config(config_table);
    // Increment the execution counter
    cfg.execution_counter++;
    // and call saveToFile to persist the new counter value
    config_saveToFile(&config_table, CONFIG_FILE);
    return 0;
}
