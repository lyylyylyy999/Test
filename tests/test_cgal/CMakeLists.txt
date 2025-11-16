cmake_minimum_required(VERSION 3.28)
project(test_cgal LANGUAGES C CXX)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS test_*.cpp)

add_executable(test_cgal ${TEST_SOURCES})

target_link_libraries(test_cgal
    PRIVATE
        cgal_lib
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_cgal)


