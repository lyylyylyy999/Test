cmake_minimum_required(VERSION 3.28)
project(test_sub LANGUAGES C CXX)

set(TEST_SOURCES
    test_sub.cpp
)

add_executable(test_sub ${TEST_SOURCES})

target_link_libraries(test_sub
    PRIVATE
        sub_lib
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_sub)


