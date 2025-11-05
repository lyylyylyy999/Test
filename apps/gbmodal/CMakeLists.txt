
include(CMakePrintHelpers)

add_library(gbmodal STATIC
    gbmodal_interface.cpp
    dirichlet_bc.cpp
    tet31.cpp
    tet_mesh.cpp
)

target_compile_features(gbmodal PRIVATE cxx_std_17)

target_include_directories(gbmodal PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(gbmodal PRIVATE 
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::IOCore
    VTK::IOXML
    VTK::IOGeometry
    Eigen3::Eigen
)

if(USE_VTK)
    add_executable(main main.cpp)
    target_compile_features(main PRIVATE cxx_std_11)

    target_include_directories(main PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_link_libraries(main PRIVATE 
        gbmodal
    )

    cmake_print_properties(
        TARGETS main 
        PROPERTIES LINK_LIBRARIES INCLUDE_DIRECTORIES)
endif()
