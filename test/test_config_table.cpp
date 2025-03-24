#include <gtest/gtest.h>
#include "config_table.h"

#define UINT32_T_DEFAULT_VALUE (115200)
#define INT32_T_DEFAULT_VALUE (-42)
#define FLOAT_DEFAULT_VALUE (1.5f)
#define STRING_DEFAULT_VALUE ("foobar")
#define BOOL_DEFAULT_VALUE (true)

class Config_Table_Test : public testing::Test {
protected:
    uint32_t _uint32_config_entry = UINT32_T_DEFAULT_VALUE;
    int32_t _int32_config_entry = INT32_T_DEFAULT_VALUE;
    float _float_config_entry = FLOAT_DEFAULT_VALUE;
    char _string_config_entry[128] = STRING_DEFAULT_VALUE;
    bool _bool_config_entry = BOOL_DEFAULT_VALUE;

    ConfigEntry_t config_entries[5] = {
        {"uint32_t", CONFIG_UINT32, &_uint32_config_entry, sizeof(_uint32_config_entry)},
        {"int32_t", CONFIG_INT32, &_int32_config_entry, sizeof(_int32_config_entry)},
        {"float", CONFIG_FLOAT, &_float_config_entry, sizeof(_float_config_entry)},
        {"string", CONFIG_STRING, &_string_config_entry, sizeof(_string_config_entry)},
        {"bool", CONFIG_BOOL, &_bool_config_entry, sizeof(_bool_config_entry)}
    };

    ConfigTable_t config_table = {
        .entries = config_entries,
        .count = static_cast<uint32_t>(std::size(config_entries))
    };

    void SetUp() override {

    }

    void TearDown() override {

    }
};

TEST_F(Config_Table_Test, GenericGetterTest) {
    // Begin by testing valid keys
    ConfigEntry_t uint_entry;
    ASSERT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "uint32_t", &uint_entry));
    EXPECT_EQ(CONFIG_UINT32, uint_entry.type);
    EXPECT_EQ(sizeof(uint32_t), uint_entry.size);
    EXPECT_EQ(UINT32_T_DEFAULT_VALUE, *static_cast<uint32_t*>(uint_entry.value));
}