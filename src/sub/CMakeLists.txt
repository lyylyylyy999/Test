set(SUB_SOURCES
    sub.cpp
)
add_library(sub_lib ${SUB_SOURCES})
target_include_directories(sub_lib
  PUBLIC  ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)