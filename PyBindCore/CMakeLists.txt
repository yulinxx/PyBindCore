cmake_minimum_required(VERSION 4.3)

set(MODULE_NAME "SanYiPyBindCore")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/../../CMake/Utils.cmake")
endif()

set(PYBIND11_FINDPYTHON ON)
find_package(Python ${SANYI_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

set(PYBINDCORE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/FacadeTypes.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/SceneGateway.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/DocumentFacade.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/py_module.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/py_types.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/py_document.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Src/py_snapshot.cpp"
)

set(PYBINDCORE_HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/PyFacade/FacadeTypes.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/PyFacade/DocumentFacade.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/Include/PyFacade/SceneGateway.h"
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
message(STATUS "")
