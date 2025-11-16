file(GLOB_RECURSE CGAL_SOURCES CONFIGURE_DEPENDS *.cpp)
add_library(cgal_lib ${CGAL_SOURCES})
target_include_directories(cgal_lib PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)