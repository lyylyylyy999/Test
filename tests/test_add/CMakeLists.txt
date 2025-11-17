cmake_minimum_required(VERSION 3.28)
project(test_add LANGUAGES C CXX)

set(TEST_SOURCES
    test_add.cpp
)
add_executable(test_add ${TEST_SOURCES})
target_link_libraries(test_add
    PRIVATE
        add_lib
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_add)


