# - Find HYPRE library
# This module sets the following variables on success:
#   HYPRE_FOUND          - Set to FALSE, or undefined, if we haven't found,
#                          or we have a problem with it.
#   HYPRE_INCLUDE_DIRS   - Include directories for HYPRE
#   HYPRE_LIBRARIES      - Libraries to link against HYPRE
#   HYPRE_VERSION        - Version of HYPRE (e.g. "2.30.0")
#   HYPRE_IS_BIGINT      - TRUE if 64-bit indices
#   HYPRE_IS_MIXEDINT    - TRUE if mixed 32/64 precision
#   HYPRE_USES_MPI       - TRUE if HYPRE was built with MPI support
#
# It also defines an imported target HYPRE::HYPRE
#
# Hints:
#   HYPRE_ROOT=/path/to/prefix
#   HYPRE_DIR=/path/to/prefix
#
# Usage:
#   find_package(HYPRE REQUIRED)
#   target_link_libraries(my_target HYPRE::HYPRE)

# Standard CMake modules
include(FindPackageHandleStandardArgs)
include(CMakeFindDependencyMacro)

# ===== User Hints and Search Paths =====
set(_HYPRE_SEARCH_DIRS)
if(HYPRE_ROOT)
    list(APPEND _HYPRE_SEARCH_DIRS "${HYPRE_ROOT}")
elseif(DEFINED ENV{HYPRE_ROOT})
    list(APPEND _HYPRE_SEARCH_DIRS "$ENV{HYPRE_ROOT}")
endif()
if(HYPRE_DIR)
    list(APPEND _HYPRE_SEARCH_DIRS "${HYPRE_DIR}")
elseif(DEFINED ENV{HYPRE_DIR})
    list(APPEND _HYPRE_SEARCH_DIRS "$ENV{HYPRE_DIR}")
endif()

# Default search paths for system installations
list(APPEND _HYPRE_SEARCH_DIRS
    /usr
    /usr/local
    /opt
    /opt/hypre
)

# Common path suffixes
set(_HYPRE_INC_SUFFIXES include include/hypre)
set(_HYPRE_LIB_SUFFIXES lib lib64 lib/x86_64-linux-gnu lib/aarch64-linux-gnu)

# ===== Find Headers =====
find_path(HYPRE_INCLUDE_DIR
    NAMES HYPRE.h
    HINTS ${_HYPRE_SEARCH_DIRS}
    PATH_SUFFIXES ${_HYPRE_INC_SUFFIXES}
    DOC "HYPRE header directory"
)

# Canonicalize include directories
set(HYPRE_INCLUDE_DIRS)
if(HYPRE_INCLUDE_DIR)
    # Prefer the parent directory containing 'hypre/' subdirectory
    get_filename_component(_parent_dir "${HYPRE_INCLUDE_DIR}" DIRECTORY)
    if(EXISTS "${_parent_dir}/hypre/HYPRE.h")
        set(HYPRE_INCLUDE_DIRS "${_parent_dir}")
    else()
        set(HYPRE_INCLUDE_DIRS "${HYPRE_INCLUDE_DIR}")
    endif()
    mark_as_advanced(HYPRE_INCLUDE_DIR)
endif()

# ===== Find Library =====
find_library(HYPRE_LIBRARY
    NAMES HYPRE hypre
    HINTS ${_HYPRE_SEARCH_DIRS}
    PATH_SUFFIXES ${_HYPRE_LIB_SUFFIXES}
    DOC "HYPRE library"
)

set(HYPRE_LIBRARIES ${HYPRE_LIBRARY})
mark_as_advanced(HYPRE_LIBRARY)

# ===== Parse Configuration =====
set(HYPRE_VERSION "")
set(HYPRE_IS_BIGINT FALSE)
set(HYPRE_IS_MIXEDINT FALSE)
set(HYPRE_USES_MPI FALSE)

if(HYPRE_INCLUDE_DIRS)
    # Try to find HYPRE_config.h in common locations
    set(_config_files
        "${HYPRE_INCLUDE_DIRS}/hypre/HYPRE_config.h"
        "${HYPRE_INCLUDE_DIRS}/HYPRE_config.h"
        "${HYPRE_INCLUDE_DIRS}/config/HYPRE_config.h"
    )
    
    foreach(_config_file IN LISTS _config_files)
        if(EXISTS "${_config_file}")
            file(READ "${_config_file}" _config_content)
            
            # Extract version
            if(NOT HYPRE_VERSION)
                string(REGEX MATCH
                    "#define[ \t]+HYPRE_RELEASE_NUMBER[ \t]+\"([0-9\\.]+)\""
                    _version_match "${_config_content}")
                if(CMAKE_MATCH_1)
                    set(HYPRE_VERSION "${CMAKE_MATCH_1}")
                endif()
                
                if(NOT HYPRE_VERSION)
                    string(REGEX MATCH
                        "#define[ \t]+HYPRE_VERSION[ \t]+\"([0-9\\.]+)\""
                        _version_match "${_config_content}")
                    if(CMAKE_MATCH_1)
                        set(HYPRE_VERSION "${CMAKE_MATCH_1}")
                    endif()
                endif()
            endif()
            
            # Check integer precision
            if(_config_content MATCHES "HYPRE_BIGINT")
                set(HYPRE_IS_BIGINT TRUE)
            endif()
            if(_config_content MATCHES "HYPRE_MIXEDINT")
                set(HYPRE_IS_MIXEDINT TRUE)
            endif()
            
            # Check MPI support
            if(_config_content MATCHES "(HYPRE_HAVE_MPI|HYPRE_USING_MPI|HYPRE_USE_MPI)")
                set(HYPRE_USES_MPI TRUE)
            endif()
            
            break()
        endif()
    endforeach()
endif()

# ===== Imported Target =====
if(HYPRE_LIBRARIES AND HYPRE_INCLUDE_DIRS AND NOT TARGET HYPRE::HYPRE)
    add_library(HYPRE::HYPRE UNKNOWN IMPORTED)
    
    set_target_properties(HYPRE::HYPRE PROPERTIES
        IMPORTED_LOCATION "${HYPRE_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${HYPRE_INCLUDE_DIRS}"
        INTERFACE_COMPILE_DEFINITIONS HYPRE_USING
    )
    
    # Handle MPI dependency
    if(HYPRE_USES_MPI OR HYPRE_REQUIRE_MPI)
        if(MPI_FOUND)
            target_link_libraries(HYPRE::HYPRE INTERFACE MPI::MPI)
            # Add MPI definitions if needed
            target_compile_definitions(HYPRE::HYPRE INTERFACE HAVE_MPI)
        elseif(HYPRE_REQUIRE_MPI)
            message(FATAL_ERROR "MPI is required but not found")
        endif()
    endif()
    
    # Add precision definitions
    if(HYPRE_IS_BIGINT)
        target_compile_definitions(HYPRE::HYPRE INTERFACE HYPRE_BIGINT)
    endif()
    if(HYPRE_IS_MIXEDINT)
        target_compile_definitions(HYPRE::HYPRE INTERFACE HYPRE_MIXEDINT)
    endif()
endif()

# ===== Version Validation =====
if(HYPRE_VERSION AND HYPRE_FIND_VERSION)
    if(HYPRE_VERSION VERSION_LESS HYPRE_FIND_VERSION)
        set(HYPRE_VERSION_OK FALSE)
        message(WARNING "Found HYPRE ${HYPRE_VERSION} but at least version ${HYPRE_FIND_VERSION} is required")
    else()
        set(HYPRE_VERSION_OK TRUE)
    endif()
else()
    set(HYPRE_VERSION_OK TRUE)
endif()

# ===== Handle FindPackage Standard Args =====
find_package_handle_standard_args(HYPRE
    REQUIRED_VARS HYPRE_LIBRARIES HYPRE_INCLUDE_DIRS
    HANDLE_COMPONENTS
    VERSION_VAR HYPRE_VERSION
)

mark_as_advanced(
    HYPRE_INCLUDE_DIRS
    HYPRE_LIBRARIES
    HYPRE_VERSION
)
