cmake_minimum_required(VERSION 3.28)
project(openfinitex VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建 example 子模块
add_subdirectory(src)

# 构建 tests 子模块
enable_testing()
add_subdirectory(tests)


