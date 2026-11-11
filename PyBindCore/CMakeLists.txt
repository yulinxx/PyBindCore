cmake_minimum_required(VERSION 4.3)

set(MODULE_NAME "AprPyBindCore")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
endif()

set(PYBIND11_FINDPYTHON ON)
# 不传 EXACT：每个人 vcpkg 装的 pybind11 会带出不同 Python 小版本（3.12/3.13/...），
# vcpkg 工具链已把它设为最高优先级，CMake 自动匹配到的就是跟 pybind11 配套的那个，
# ABI 保证一致。若需强制指定版本（如 CI），用 -DAPR_PYTHON_VERSION=3.x 覆盖。
if(DEFINED APR_PYTHON_VERSION AND NOT APR_PYTHON_VERSION STREQUAL "")
    find_package(Python ${APR_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)
else()
    find_package(Python 3.11 COMPONENTS Interpreter Development REQUIRED)
endif()

# pybind11 发现顺序：工具链/包管理器（vcpkg、brew 等）提供的 config → 由
# Python 解释器自身推导 pip 安装的 config → 直接探测 site-packages。
# 避免依赖手工 -Dpybind11_DIR，保证在 IDE 或全新构建目录里也能直接配置。
find_package(pybind11 CONFIG QUIET)

if(NOT pybind11_FOUND AND Python_Interpreter_FOUND)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE _apr_pybind11_cmakedir
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _apr_pybind11_cmakedir_result)
    if(_apr_pybind11_cmakedir_result EQUAL 0
       AND EXISTS "${_apr_pybind11_cmakedir}/pybind11Config.cmake")
        find_package(pybind11 CONFIG REQUIRED PATHS "${_apr_pybind11_cmakedir}" NO_DEFAULT_PATH)
    endif()
endif()

if(NOT pybind11_FOUND AND Python_Interpreter_FOUND)
    # 兜底：部分环境里 pip 包的 cmake 配置位于 site-packages 下
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_paths()['purelib'])"
        OUTPUT_VARIABLE _apr_python_purelib
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
        RESULT_VARIABLE _apr_python_purelib_result)
    if(_apr_python_purelib_result EQUAL 0)
        foreach(_apr_pybind11_candidate
            "${_apr_python_purelib}/pybind11/share/cmake/pybind11"
            "${_apr_python_purelib}/pybind11/cmake")
            if(EXISTS "${_apr_pybind11_candidate}/pybind11Config.cmake")
                find_package(pybind11 CONFIG REQUIRED PATHS "${_apr_pybind11_candidate}" NO_DEFAULT_PATH)
                break()
            endif()
        endforeach()
    endif()
endif()

if(NOT pybind11_FOUND)
    message(FATAL_ERROR
        "[${MODULE_NAME}] pybind11 not found for Python ${Python_VERSION} (${Python_EXECUTABLE}).\n"
        "Install it with:\n"
        "  \"${Python_EXECUTABLE}\" -m pip install \"pybind11>=2.11\"\n"
        "Do not use the vcpkg pybind11 port for this project: it pulls vcpkg's python3\n"
        "and forces FindPython onto that interpreter, which changes the extension ABI\n"
        "away from the Python CMake already selected (${Python_VERSION}).\n"
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
    OUTPUT_NAME "_apr_core"
)

if(MSVC)
    target_compile_options(${MODULE_NAME} PRIVATE /utf-8)
endif()

add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../Python/apr"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${MODULE_NAME}>"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../Python/apr/$<TARGET_FILE_NAME:${MODULE_NAME}>"
    COMMENT "Copying _apr_core extension into Python/apr"
)

message(STATUS "[${MODULE_NAME}]")
message(STATUS "  Module: _apr_core")
message(STATUS "  Python: ${Python_VERSION}")
message(STATUS "----------------------------------------")

apr_export_imported_target(${MODULE_NAME}
    LIB_NAME "${MODULE_NAME}"
    PUBLIC_INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/Include"
    INTERFACE_LINK_LIBRARIES
        EngineCommon::EngineCommon
        Engine2D::Engine2D
)
