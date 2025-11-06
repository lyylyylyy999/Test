cmake_minimum_required(VERSION 3.28)
project(src)

aux_source_directory(./ SRC)
set(EXECUTABLE_OUTPUT_PATH ${EXEC_PATH})
add_executable(${TEST_EXE} ${SRC})
target_include_directories(${TEST_EXE} PUBLIC ${HEAD_PATH})
target_link_libraries(${TEST_EXE} ${CALC_LIB})