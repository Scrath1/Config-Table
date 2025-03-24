#include <gtest/gtest.h>
#include "config_table.h"

#define UINT32_T_DEFAULT_VALUE (115200)
#define INT32_T_DEFAULT_VALUE (-42)
#define FLOAT_DEFAULT_VALUE (1.5f)
#define STRING_DEFAULT_VALUE ("foobar")
#define BOOL_DEFAULT_VALUE (true)

#define MAX_STRING_LEN (16)

class Config_Table_Test : public testing::Test {
protected:
    uint32_t _uint32_config_entry = UINT32_T_DEFAULT_VALUE;
    int32_t _int32_config_entry = INT32_T_DEFAULT_VALUE;
    float _float_config_entry = FLOAT_DEFAULT_VALUE;
    char _string_config_entry[MAX_STRING_LEN] = STRING_DEFAULT_VALUE;
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

TEST_F(Config_Table_Test, KeyToIndexResolutionTest) {
    uint32_t known_string_index = UINT32_MAX;
    constexpr char string_key[] = "string";
    for(uint32_t i = 0; i < config_table.count; i++) {
        const char* entry_key = config_entries[i].key;
        if(strcmp(string_key, entry_key) == 0) known_string_index = i;
    }
    ASSERT_LT(known_string_index, config_table.count);

    int32_t resolved_idx = config_getIdxFromKey(&config_table, string_key);
    EXPECT_EQ(resolved_idx, known_string_index);
}

TEST_F(Config_Table_Test, GenericGetterTest) {
    // Begin by testing valid keys
    ConfigEntry_t* uint_entry_ptr = nullptr;
    ASSERT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "uint32_t", &uint_entry_ptr));
    ASSERT_NE(nullptr, uint_entry_ptr);
    EXPECT_EQ(CONFIG_UINT32, uint_entry_ptr->type);
    EXPECT_EQ(sizeof(uint32_t), uint_entry_ptr->size);
    EXPECT_EQ(UINT32_T_DEFAULT_VALUE, *static_cast<uint32_t*>(uint_entry_ptr->value));

    ConfigEntry_t* float_entry_ptr = nullptr;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "float", &float_entry_ptr));
    EXPECT_NE(nullptr, float_entry_ptr);
    EXPECT_EQ(CONFIG_FLOAT, float_entry_ptr->type);
    EXPECT_EQ(sizeof(float), float_entry_ptr->size);
    EXPECT_NEAR(FLOAT_DEFAULT_VALUE, *static_cast<float*>(float_entry_ptr->value), 0.005);

    ConfigEntry_t* string_entry_ptr = nullptr;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "string", &string_entry_ptr));
    EXPECT_NE(nullptr, string_entry_ptr);
    EXPECT_EQ(CONFIG_STRING, string_entry_ptr->type);
    EXPECT_STREQ(STRING_DEFAULT_VALUE, static_cast<char*>(string_entry_ptr->value));

    // Test invalid keys
    ConfigEntry_t* invalid_entry_ptr = nullptr;
    EXPECT_EQ(CFG_RC_ERROR_UNKNOWN_KEY, config_getByKey(&config_table, "invalid", &invalid_entry_ptr));

    // Test nullptr handling
    EXPECT_EQ(CFG_RC_ERROR_NULLPTR, config_getByKey(nullptr, "uint32_t", nullptr));
    EXPECT_EQ(CFG_RC_ERROR_NULLPTR, config_getByKey(&config_table, nullptr, nullptr));
}

TEST_F(Config_Table_Test, GenericSetterTest) {
    // Test for simple ints
    ConfigEntry_t* entry_ptr = nullptr;
    ASSERT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "int32_t", &entry_ptr));
    ASSERT_NE(nullptr, entry_ptr);

    int32_t new_value = *static_cast<int32_t*>(entry_ptr->value) * 2;
    EXPECT_EQ(CFG_RC_SUCCESS, config_setByKey(&config_table, "int32_t", &new_value, sizeof(new_value)));

    EXPECT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "int32_t", &entry_ptr));
    EXPECT_EQ(new_value, *static_cast<int32_t*>(entry_ptr->value));

    // Also test for correct string handling
    constexpr char oversized_len_str[MAX_STRING_LEN + 1] = "abcdefghijklmnop";
    constexpr char max_len_str[MAX_STRING_LEN] = "abcdefghijklmno";
    constexpr char half_len_str[] = "abcdefg";

    EXPECT_EQ(CFG_RC_SUCCESS, config_getByKey(&config_table, "string", &entry_ptr));
    EXPECT_NE(nullptr, entry_ptr);

    EXPECT_STREQ(STRING_DEFAULT_VALUE, static_cast<char*>(entry_ptr->value));

    // Try writing a string that is too large for the memory
    EXPECT_EQ(CFG_RC_ERROR_TOO_LARGE, config_setByKey(&config_table, "string", oversized_len_str, sizeof(oversized_len_str)));
    // Try writing a string that barely fits
    EXPECT_EQ(CFG_RC_SUCCESS, config_setByKey(&config_table, "string", max_len_str, sizeof(max_len_str)));
    EXPECT_STREQ(max_len_str, static_cast<char*>(entry_ptr->value));
    // Determine correct null-terminator placement
    EXPECT_EQ('\0', static_cast<char*>(entry_ptr->value)[MAX_STRING_LEN - 1]);
    // Try writing a string that fits and make sure that the null-terminator is again set correctly
    EXPECT_EQ(CFG_RC_SUCCESS, config_setByKey(&config_table, "string", half_len_str, sizeof(half_len_str)));
    EXPECT_STREQ(half_len_str, static_cast<char*>(entry_ptr->value));
    EXPECT_EQ('\0', static_cast<char*>(entry_ptr->value)[MAX_STRING_LEN / 2 - 1]);
}

TEST_F(Config_Table_Test, SpecializedGetterTest) {
    // uint
    int32_t uint_idx = config_getIdxFromKey(&config_table, "uint32_t");
    uint32_t uint = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getUint32ByKey(&config_table, "uint32_t", &uint));
    EXPECT_EQ(uint, UINT32_T_DEFAULT_VALUE);
    uint = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getUint32ByIdx(&config_table, uint_idx, &uint));
    EXPECT_EQ(uint, UINT32_T_DEFAULT_VALUE);

    // int
    int32_t int_idx = config_getIdxFromKey(&config_table, "int32_t");
    int32_t integer = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getInt32ByKey(&config_table, "int32_t", &integer));
    EXPECT_EQ(integer, INT32_T_DEFAULT_VALUE);
    integer = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getInt32ByIdx(&config_table, int_idx, &integer));
    EXPECT_EQ(integer, INT32_T_DEFAULT_VALUE);

    // float
    int32_t float_idx = config_getIdxFromKey(&config_table, "float");
    float f = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getFloatByKey(&config_table, "float", &f));
    EXPECT_NEAR(f, FLOAT_DEFAULT_VALUE, FLT_EPSILON);
    f = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getFloatByIdx(&config_table, float_idx, &f));
    EXPECT_NEAR(f, FLOAT_DEFAULT_VALUE, FLT_EPSILON);

    // string
    int32_t string_idx = config_getIdxFromKey(&config_table, "string");
    char* str = nullptr;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getStringByKey(&config_table, "string", &str));
    EXPECT_STREQ(str, STRING_DEFAULT_VALUE);
    str = nullptr;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getStringByIdx(&config_table, string_idx, &str));
    EXPECT_STREQ(str, STRING_DEFAULT_VALUE);

    int32_t bool_idx = config_getIdxFromKey(&config_table, "bool");
    bool b = !BOOL_DEFAULT_VALUE;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByKey(&config_table, "bool", &b));
    EXPECT_EQ(b, BOOL_DEFAULT_VALUE);
    b = !BOOL_DEFAULT_VALUE;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &b));
    EXPECT_EQ(b, BOOL_DEFAULT_VALUE);
}

TEST_F(Config_Table_Test, KeyValueParsingTest) {
    // Test rejection of unknown keys
    char invalid_key_str[] = "foo: bar";
    EXPECT_EQ(CFG_RC_ERROR_UNKNOWN_KEY, config_parseKVStr(&config_table, invalid_key_str, sizeof(invalid_key_str)));
    // Test missing separator
    char missing_sep_str[] = "hello world";
    EXPECT_EQ(CFG_RC_ERROR_FORMAT, config_parseKVStr(&config_table, missing_sep_str, sizeof(missing_sep_str)));
    // Test all types
    // uint
    char valid_uint_str[] = "uint32_t: 9600";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_uint_str, sizeof(valid_uint_str)));
    uint32_t parsed_uint = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getUint32ByKey(&config_table, "uint32_t", &parsed_uint));
    EXPECT_EQ(parsed_uint, 9600);
    char invalid_uint_str[] = "uint32_t: -1";
    EXPECT_EQ(CFG_RC_ERROR, config_parseKVStr(&config_table, invalid_uint_str, sizeof(invalid_uint_str)));
    // int
    char valid_int_str[] = "int32_t: -50";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_int_str, sizeof(valid_int_str)));
    int32_t parsed_int = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getInt32ByKey(&config_table, "int32_t", &parsed_int));
    char invalid_int_str[] = "int32_t: 4294967295"; // out of range
    EXPECT_EQ(CFG_RC_ERROR, config_parseKVStr(&config_table, invalid_int_str, sizeof(invalid_int_str)));
    // float
    char valid_float_str[] = "float: 1.5";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_float_str, sizeof(valid_float_str)));
    float parsed_float = 0;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getFloatByKey(&config_table, "float", &parsed_float));
    EXPECT_NEAR(parsed_float, 1.5, FLT_EPSILON);
    // string
    char valid_str_1[] = "string: valid string";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_str_1, sizeof(valid_str_1)));
    char* parsed_str = nullptr;
    EXPECT_EQ(CFG_RC_SUCCESS, config_getStringByKey(&config_table, "string",&parsed_str));
    EXPECT_STREQ("valid string", parsed_str);
    parsed_str = nullptr;
    char valid_str_2[] = "string: \"valid string\"";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_str_2, sizeof(valid_str_2)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getStringByKey(&config_table, "string", &parsed_str));
    EXPECT_STREQ("valid string", parsed_str);
    char oversized_str[] = "string: This string is too long for the current character limit";
    EXPECT_EQ(CFG_RC_ERROR_TOO_LARGE, config_parseKVStr(&config_table, oversized_str, sizeof(oversized_str)));

    // bool
    // Test all configurations for strings
    bool parsed_bool = false;
    int32_t bool_idx = config_getIdxFromKey(&config_table, "bool");
    ASSERT_NE(bool_idx, -1);

    char valid_true_str1[] = "bool: True";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_true_str1, sizeof(valid_true_str1)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_TRUE(parsed_bool);
    parsed_bool = false;

    char valid_true_str2[] = "bool: true";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_true_str2, sizeof(valid_true_str2)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_TRUE(parsed_bool);
    parsed_bool = false;

    char valid_true_str3[] = "bool: 1";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_true_str3, sizeof(valid_true_str3)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_TRUE(parsed_bool);

    char valid_false_str1[] = "bool: False";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_false_str1, sizeof(valid_false_str1)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_FALSE(parsed_bool);
    parsed_bool = true;

    char valid_false_str2[] = "bool: false";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_false_str2, sizeof(valid_false_str2)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_FALSE(parsed_bool);
    parsed_bool = true;

    char valid_false_str3[] = "bool: 0";
    EXPECT_EQ(CFG_RC_SUCCESS, config_parseKVStr(&config_table, valid_false_str3, sizeof(valid_false_str3)));
    EXPECT_EQ(CFG_RC_SUCCESS, config_getBoolByIdx(&config_table, bool_idx, &parsed_bool));
    EXPECT_FALSE(parsed_bool);
    parsed_bool = true;
}