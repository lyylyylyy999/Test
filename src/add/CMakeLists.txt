set(ADD_SOURCES
    add.cpp
)
add_library(add_lib ${ADD_SOURCES})
target_include_directories(add_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)