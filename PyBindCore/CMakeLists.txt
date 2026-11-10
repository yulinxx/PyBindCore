cmake_minimum_required(VERSION 4.3)

set(MODULE_NAME "SanYiPyBindCore")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
endif()

set(PYBIND11_FINDPYTHON ON)
find_package(Python ${SANYI_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)

# pybind11 发现顺序：工具链/包管理器（vcpkg、brew 等）提供的 config → 由
# Python 解释器自身推导 pip 安装的 config → 直接探测 site-packages。
# 避免依赖手工 -Dpybind11_DIR，保证在 IDE 或全新构建目录里也能直接配置。
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND AND Python_Interpreter_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE _sanyi_pybind11_cmakedir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _sanyi_pybind11_cmakedir_result)
    if(_sanyi_pybind11_cmakedir_result EQUAL 0
       AND EXISTS "${_sanyi_pybind11_cmakedir}/pybind11Config.cmake")
        find_package(pybind11 CONFIG REQUIRED PATHS "${_sanyi_pybind11_cmakedir}" NO_DEFAULT_PATH)
    endif()
endif()

if(NOT pybind11_FOUND AND Python_Interpreter_FOUND)
    # 兜底：部分环境里 pip 包的 cmake 配置位于 site-packages 下
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_paths()['purelib'])"
        OUTPUT_VARIABLE _sanyi_python_purelib
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _sanyi_python_purelib_result)
    if(_sanyi_python_purelib_result EQUAL 0)
        foreach(_sanyi_pybind11_candidate
            "${_sanyi_python_purelib}/pybind11/share/cmake/pybind11"
            "${_sanyi_python_purelib}/pybind11/cmake")
            if(EXISTS "${_sanyi_pybind11_candidate}/pybind11Config.cmake")
                find_package(pybind11 CONFIG REQUIRED PATHS "${_sanyi_pybind11_candidate}" NO_DEFAULT_PATH)
                break()
            endif()
        endforeach()
    endif()
endif()

if(NOT pybind11_FOUND)
    if(VCPKG_TARGET_TRIPLET)
        set(_sanyi_pybind11_triplet "${VCPKG_TARGET_TRIPLET}")
    else()
        set(_sanyi_pybind11_triplet "<your-triplet>")
    endif()
    message(FATAL_ERROR
        "[${MODULE_NAME}] pybind11 not found for Python ${Python_VERSION} (${Python_EXECUTABLE}).\n"
        "Install it with one of the following commands, then re-run CMake:\n"
        "  pip:   \"${Python_EXECUTABLE}\" -m pip install \"pybind11>=2.11\"\n"
        "  vcpkg: <vcpkg-root>/vcpkg install pybind11:${_sanyi_pybind11_triplet} --classic\n"
        "Alternatively pass -Dpybind11_DIR=<directory containing pybind11Config.cmake>.")
endif()

file(GLOB PYBINDCORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/*.cpp"
)
file(GLOB PYBINDCORE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/PyFacade/*.h"
)

add_library(${MODULE_NAME} MODULE
    ${PYBINDCORE_SOURCES}
    ${PYBINDCORE_HEADERS}
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${PYBINDCORE_SOURCES} ${PYBINDCORE_HEADERS})

target_include_directories(${MODULE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine/Common/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine/Common/Include/Engine"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine/2D/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine/2D/Include/Engine2D"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Utility/Utility/Include"
)

target_link_libraries(${MODULE_NAME} PRIVATE
    pybind11::module
    EngineCommon
    Engine2D
)

set_target_properties(${MODULE_NAME} PROPERTIES
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    OUTPUT_NAME "_sanyi_core"
)

if(MSVC)
    target_compile_options(${MODULE_NAME} PRIVATE /utf-8)
endif()

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../Python/sanyi"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${MODULE_NAME}>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../Python/sanyi/$<TARGET_FILE_NAME:${MODULE_NAME}>"
    COMMENT "Copying _sanyi_core extension into Python/sanyi"
)

message(STATUS "[${MODULE_NAME}]")
message(STATUS "  Module: _sanyi_core")
message(STATUS "  Python: ${Python_VERSION}")
message(STATUS "----------------------------------------")

sanyi_export_imported_target(${MODULE_NAME}
    LIB_NAME "${MODULE_NAME}"
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    INTERFACE_LINK_LIBRARIES
        EngineCommon::EngineCommon
        Engine2D::Engine2D
)
