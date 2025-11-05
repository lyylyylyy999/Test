include(CMakePrintHelpers)

if(USE_CGAL)
    add_executable(aabb_tree_example aabb_tree_example.cpp)
    target_include_directories(aabb_tree_example PRIVATE ${CGAL_INCLUDE_DIRS})
    target_compile_features(aabb_tree_example PRIVATE cxx_std_11)
    target_link_libraries(aabb_tree_example PRIVATE CGAL::CGAL)
endif()
