cmake_minimum_required(VERSION 3.28)


# ------------------------------
# vcpkg integration (Windows)
# *This must be done before project()*
# ------------------------------
if(WIN32 AND DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    set(USE_VCPKG ON)
    set(VCPKG_FEATURE_FLAGS "classic")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    message(STATUS "Using vcpkg toolchain from $ENV{VCPKG_ROOT}")
endif()

project(openfinitex VERSION 1.0.0 LANGUAGES C CXX)
message(STATUS "Configuring project ${PROJECT_NAME} version ${PROJECT_VERSION}")

# Show system information
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
message(STATUS "System version: ${CMAKE_SYSTEM_VERSION}")
message(STATUS "System processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(UNIX AND NOT APPLE)
    message(STATUS "Detected Unix-like OS")
    execute_process(
        COMMAND uname -r
        OUTPUT_VARIABLE KERNEL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Kernel version: ${KERNEL_VERSION}")
    execute_process(
        COMMAND lsb_release -si
        OUTPUT_VARIABLE DISTRO_NAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Distro: ${DISTRO_NAME} ${DISTRO_VERSION}")
endif()

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug, Release, etc.)" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17) # global c++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(WIN32) # Windows
    add_compile_definitions(DNOMINMAX _USE_MATH_DEFINES)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)  # 简化Windows DLL导出
else()
    find_package(PkgConfig REQUIRED)
    add_compile_definitions(_USE_MATH_DEFINES)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
    find_library(M_LIB m)
endif()

if(MSVC)
    add_compile_options(/O2 /W4 /DNOMINMAX)
else()
    add_compile_options(-O2 -Wall -Wextra)
endif()


option(USE_JSON "Whether to link nlohmann_json" ON)
if(USE_JSON)
    find_package(nlohmann_json REQUIRED)
    message(STATUS "nlohmann_json found: ${nlohmann_json_VERSION}")
else()
    message(STATUS "nlohmann_json not used")
endif()

option(USE_CGAL "Whether to link CGAL" ON) 
if(USE_CGAL)
    set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE ON)
    find_package(CGAL REQUIRED COMPONENTS Core)
    message(STATUS "CGAL found: ${CGAL_VERSION} ${CGAL_INCLUDE_DIRS}")
else()
    message(STATUS "CGAL not used")
endif()

# apt install pybind11-dev pybind11-json-dev
option(USE_PYBIND11 "Whether to build Python bindings" ON)
option(USE_PYBIND11_JOSN "Whether to use nlohmann_json with pybind11" ON)
option(USE_CONDA_RPATH "Use CONDA_PREFIX/lib in rpath when present" OFF)
if(USE_PYBIND11)
    find_package(Python3 REQUIRED COMPONENTS Interpreter Development Development.Module)
    find_package(pybind11 REQUIRED)
    if(USE_PYBIND11_JOSN)
        find_package(pybind11_json REQUIRED)
    endif()
    message(STATUS "pybind11 found: ${pybind11_VERSION} ${pybind11_INCLUDE_DIRS}")
else()
    message(STATUS "pybind11 not used")
endif()


# sudo apt install --reinstall openmpi-bin libopenmpi-dev
option(USE_MPI "Whether to link MPI" ON) 
if(USE_MPI)
    find_package(MPI REQUIRED COMPONENTS CXX C)
    message(STATUS "Found MPI: ${MPI_VERSION}")
    message(STATUS "MPI_C_LIBRARIES: ${MPI_C_LIBRARIES}")
    message(STATUS "MPI_C_INCLUDE_DIRS: ${MPI_C_INCLUDE_DIRS}")
endif()

# On Ubuntu, you may need to install VTK development files: 
# sudo apt install libvtk9-dev qtbase5-dev 
# On Windows, consider using vcpkg:
# vcpkg install vtk
option(USE_VTK "Whether to link VTK" ON)
if(USE_VTK)
    find_package(VTK REQUIRED COMPONENTS 
        CommonCore
        CommonDataModel
        IOCore
        IOGeometry
        IOXML
    )
    message(STATUS "Found VTK: ${VTK_VERSION}")
endif()


option(USE_EIGEN "Whether to link Eigen3" ON)
if(USE_EIGEN)
    find_package(Eigen3 3.4 REQUIRED NO_MODULE)
    message(STATUS "Found Eigen3: ${Eigen3_VERSION}")
endif()

option(USE_ARPACK "Whether to link ARPACK(ng)" ON)
if(USE_ARPACK)
    if(WIN32)
        find_package(ARPACK-ng REQUIRED)
        message(STATUS "Found ARPACK-ng: ${ARPACK-ng_VERSION}")
    else()
        pkg_check_modules(ARPACK REQUIRED arpack)
        message(STATUS "Found ARPACK: ${ARPACK_VERSION}")
        message(STATUS "ARPACK_LIBRARIES: ${ARPACK_LIBRARIES}")
        message(STATUS "ARPACK_INCLUDE_DIRS: ${ARPACK_INCLUDE_DIRS}")
        # 查找实际的库文件路径
        find_library(ARPACK_LIBRARY NAMES arpack libarpack HINTS ${ARPACK_LIBRARY_DIRS})
        if(NOT ARPACK_LIBRARY)
            message(FATAL_ERROR "ARPACK library not found")
        endif()
        message(STATUS "ARPACK_LIBRARY: ${ARPACK_LIBRARY}")
        if(NOT TARGET ARPACK::ARPACK)
            add_library(ARPACK::ARPACK UNKNOWN IMPORTED)
            set_target_properties(ARPACK::ARPACK PROPERTIES
                IMPORTED_LOCATION "${ARPACK_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${ARPACK_INCLUDE_DIRS}"
            )
        endif()
    endif()
    message(STATUS "ARPACK_LIBRARIES: ${ARPACK_LIBRARIES}")
    message(STATUS "ARPACK_INCLUDE_DIRS: ${ARPACK_INCLUDE_DIRS}")
endif()

option(USE_HYPRE "Whether to link hypre" ON)
if(USE_HYPRE)
    find_package(HYPRE REQUIRED)
    message(STATUS "Found hypre: ${HYPRE_VERSION}")
endif()


option(USE_SPECTRA "Whether to link Spectra" ON)
if(USE_SPECTRA)
    find_package(spectra CONFIG REQUIRED)
    message(STATUS "Found spectra: ${spectra_VERSION}")
endif()


option(USE_PETSC "Whether to link PETSc" ON)
if(USE_PETSC)
    find_package(PETSc REQUIRED)
    message(STATUS "PETSc found: ${PETSc_VERSION}")
endif()

option(USE_SLEPC "Whether to link SLEPc" ON)
if(USE_SLEPC)
    message(STATUS "SLEPc enabled")
    find_package(SLEPc REQUIRED)
    if(NOT USE_PETSC)
        message(FATAL_ERROR "SLEPc requires PETSc. Please enable USE_PETSC option.")
    endif()
    message(STATUS "SLEPc found: ${SLEPc_VERSION}")
endif()

option(BUILD_TESTING "Build unit tests" ON)
include(CTest)  # 定义 BUILD_TESTING，并为 ctest 做好准备

option(USE_GTEST "Whether to build tests with GoogleTest" ON)
if(USE_GTEST)
    find_package(GTest REQUIRED)
    include(GoogleTest)
    message(STATUS "GoogleTest found: ${GTEST_VERSION}")
endif()

add_subdirectory(examples)
add_subdirectory(apps)
add_subdirectory(tests)
