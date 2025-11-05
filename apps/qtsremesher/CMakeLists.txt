if(USE_PYBIND11 AND USE_CGAL)
    set(QTSREMESHER_PY_MODULE_NAME "qtsremesher" CACHE STRING "Python module name")
    set(QTSREMESHER_SRC
      ${CMAKE_CURRENT_SOURCE_DIR}/qtsremesher.cpp
    )

    # 这会创建一个与 Python 扩展命名规则一致的目标（.so/.pyd）
    pybind11_add_module(${QTSREMESHER_PY_MODULE_NAME} MODULE ${QTSREMESHER_SRC})
    target_include_directories(${QTSREMESHER_PY_MODULE_NAME}
        PRIVATE
        ${Python3_INCLUDE_DIRS}
    )

    target_link_libraries(${QTSREMESHER_PY_MODULE_NAME}
        PRIVATE 
        CGAL::CGAL 
        CGAL::CGAL_Core 
        Threads::Threads)

    set(_rpath "$ORIGIN")
    if (USE_CONDA_RPATH AND DEFINED ENV{CONDA_PREFIX})
        file(TO_CMAKE_PATH "$ENV{CONDA_PREFIX}" _conda_prefix)
        if (EXISTS "${_conda_prefix}/lib")
            list(APPEND _rpath "${_conda_prefix}/lib")
        endif()
    endif()
    if (APPLE)
        list(TRANSFORM _rpath REPLACE "\\$ORIGIN" "@loader_path")
        set_target_properties(${QTSREMESHER_PY_MODULE_NAME} 
            PROPERTIES
            BUILD_RPATH           "${_rpath}"
            INSTALL_RPATH         "${_rpath}"
            MACOSX_RPATH          ON
        )
    else()
        set_target_properties(${QTSREMESHER_PY_MODULE_NAME}
            PROPERTIES 
            BUILD_RPATH    "${_rpath}"  
            INSTALL_RPATH  "${_rpath}"
        )
    endif()

    include(GNUInstallDirs)

    if (DEFINED Python3_SITEARCH)
        set(PY_SITE_DIR "${Python3_SITEARCH}")
    elseif (DEFINED Python3_SITELIB)
        set(PY_SITE_DIR "${Python3_SITELIB}")
    else()
        execute_process(
            COMMAND "${Python3_EXECUTABLE}" -c "import sysconfig,sys;print(sysconfig.get_paths().get('platlib') or sysconfig.get_paths().get('purelib'))"
            OUTPUT_VARIABLE PY_SITE_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()

    message(STATUS "Python site-packages: ${PY_SITE_DIR}")

    install(TARGETS ${QTSREMESHER_PY_MODULE_NAME}
        LIBRARY DESTINATION "${PY_SITE_DIR}"
        ARCHIVE DESTINATION "${PY_SITE_DIR}"  # Windows 等平台兜底
        RUNTIME DESTINATION "${PY_SITE_DIR}"
    )

    add_custom_command(TARGET qtsremesher POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/qtsremesher_example.py
        ${CMAKE_CURRENT_BINARY_DIR}/qtsremesher_example.py
        COMMENT "Syncing qtsremesher_example.py to build dir"
    )
    add_custom_command(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/qtsremesher_example.py
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${CMAKE_CURRENT_SOURCE_DIR}/qtsremesher_example.py
                ${CMAKE_CURRENT_BINARY_DIR}/qtsremesher_example.py
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/qtsremesher_example.py
        COMMENT "Copy qtsremesher_example.py -> binary dir"
    )

    add_custom_target(copy_qtsremesher_example ALL
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qtsremesher_example.py
)
endif()
