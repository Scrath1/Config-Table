#include <cstdint>
#include <iostream>

#include "config_table.h"

static uint32_t baud_rate = 115200;
static char ssid[128] = "HelloInternet";

ConfigEntry_t config_entries[] = {
    {"baud_rate", CONFIG_UINT32, &baud_rate, sizeof(baud_rate)},
    {"SSID", CONFIG_STRING, &ssid, sizeof(ssid)}
};

ConfigTable_t table{
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
    ConfigEntry_t entry;
    config_getByKey(&table, "baud_rate", &entry);
    print_config(table);
    uint32_t newVal = 9600;
    config_setByKey(&table, "baud_rate", &newVal, sizeof(newVal));
    char newSSID[] = "Foobar";
    config_setByKey(&table, "SSID", newSSID, sizeof(newSSID));
    print_config(table);
    std::cout << "Parsing Test" << std::endl;
    char kvstr1[] = "SSID: ANewSSID";
    config_parseKVStr(&table, kvstr1, sizeof(kvstr1));
    print_config(table);
    char kvstr2[] = "SSID: \"A\"";
    config_parseKVStr(&table, kvstr2, sizeof(kvstr2));
    print_config(table);
    return 0;
}
