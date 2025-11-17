cmake_minimum_required(VERSION 3.28)
project(tests LANGUAGES C CXX)

# GoogleTest（假设使用 FetchContent）
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 引入两个测试模块
add_subdirectory(test_add)
add_subdirectory(test_sub)
