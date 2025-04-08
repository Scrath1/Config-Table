cmake_minimum_required(VERSION 3.28)
project(Config_Table)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        googletest
        # Specify the commit you depend on and update it regularly.
        URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
enable_testing()

include_directories(include)

file(GLOB config_table_src
    "include/config_table.h" "src/config_table.c"
)

add_executable(basic_example examples/basic-example.cpp ${config_table_src})
add_executable(struct_example examples/example_with_config_struct.cpp ${config_table_src})
add_executable(run_unit_tests test/main.cpp
        test/test_config_table.cpp
        ${config_table_src}
)
target_link_libraries(run_unit_tests gtest)
add_test(NAME config_table_test COMMAND run_unit_tests)