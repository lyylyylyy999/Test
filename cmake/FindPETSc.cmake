
find_path(PETSC_DIR
    NAMES include/petsc.h
    PATHS
        $ENV{PETSC_DIR}
        /usr/lib/petsc
        /usr/local/lib/petsc
    DOC "PETSc安装目录"
)

if(PETSC_DIR)
    find_path(PETSC_INCLUDE_DIR
        NAMES petsc.h
        PATHS ${PETSC_DIR}/include
        NO_DEFAULT_PATH
    )

    find_library(PETSC_LIBRARY
        NAMES libpetsc_real.so
        PATHS ${PETSC_DIR}/lib
        NO_DEFAULT_PATH
    )

    message(STATUS "PETSc include dir: ${PETSC_INCLUDE_DIR}")
    message(STATUS "PETSc library dir: ${PETSC_LIBRARY}")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(PETSc
        REQUIRED_VARS PETSC_DIR PETSC_INCLUDE_DIR PETSC_LIBRARY
        VERSION_VAR PETSC_VERSION
    )

    if(PETSc_FOUND AND NOT TARGET PETSc::PETSc)
        add_library(PETSc::PETSc UNKNOWN IMPORTED)
        set_target_properties(PETSc::PETSc PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${PETSC_INCLUDE_DIR}"
            IMPORTED_LOCATION "${PETSC_LIBRARY}"
        )
    endif()
endif()
