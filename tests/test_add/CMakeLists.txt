
set(TEST_SOURCES
    test_add.cpp
)
add_executable(test_add ${TEST_SOURCES})
target_link_libraries(test_add
    PRIVATE
        add_lib
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_add)


