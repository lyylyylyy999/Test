cmake_minimum_required(VERSION 3.28)
project(Test)

set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(EXEC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(CALC_LIB calc)
set(TEST_EXE test)

add_subdirectory(src)
add_subdirectory(test)

