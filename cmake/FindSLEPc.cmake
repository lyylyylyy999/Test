
find_path(SLEPC_DIR
    NAMES include/slepc.h
    PATHS
        $ENV{SLEPC_DIR}
        "/usr/lib/slepc64"
    NO_DEFAULT_PATH
)

if(PETSC_DIR)
    find_path(SLEPC_INCLUDE_DIR
        NAMES slepc.h
        PATHS ${SLEPC_DIR}/include
        NO_DEFAULT_PATH
    )

    find_library(SLEPC_LIBRARY
        NAMES libslepc64_real.so
        PATHS ${SLEPC_DIR}/lib
        NO_DEFAULT_PATH
    )

    message(STATUS "SLEPc include dir: ${SLEPC_INCLUDE_DIR}")
    message(STATUS "SLEPc library dir: ${SLEPC_LIBRARY}")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SLEPc
        REQUIRED_VARS SLEPC_DIR SLEPC_INCLUDE_DIR SLEPC_LIBRARY
        VERSION_VAR SLEPC_VERSION
    )

    if(PETSc_FOUND AND NOT TARGET SLEPc::SLEPc)
        add_library(SLEPc::SLEPc UNKNOWN IMPORTED)
        set_target_properties(SLEPc::SLEPc PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${SLEPC_INCLUDE_DIR}"
            IMPORTED_LOCATION "${SLEPC_LIBRARY}"
        )
    endif()
endif()
