cmake_minimum_required(VERSION 3.28)
project(src)

aux_source_directory(./ SRC)
set(LIBRARY_OUTPUT_PATH ${LIB_PATH})
add_library(${CALC_LIB} STATIC ${SRC})
target_include_directories(${CALC_LIB} PUBLIC ${HEAD_PATH})
