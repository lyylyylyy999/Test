cmake_minimum_required(VERSION 3.28)
project(openfinitex VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 构建 src 子模块
add_subdirectory(src)

# 测试：推荐用 CTest 约定
include(CTest)      # 定义 BUILD_TESTING 并 enable_testing()
if (BUILD_TESTING)
  add_subdirectory(tests)
endif()


