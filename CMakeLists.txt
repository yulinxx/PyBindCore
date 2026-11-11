cmake_minimum_required(VERSION 4.3)

# ============================================================================
# PyBindCore —— 独立构建入口
# ============================================================================
# 集成构建：根 CMakeLists.txt → add_subdirectory(PyBindCore/PyBindCore)（内层）。
# 独立构建：本文件提供 project() 与环境配置，把 PyBindCore 单独拉出来开发。
#
# 三条设计约束同时成立：
#
#   1) 复用仓库统一配置，不自己手搓一套，避免与主工程漂移：
#        ../Config.cmake               —— vcpkg / Qt 路径、MSVC 运行库、模块开关
#        ../CMake/StandaloneInit.cmake —— APR_ROOT、编译选项、GTest 查找、兄弟模块拉入
#      与 Log / FileIO / Engine 系列的独立入口保持同一套写法。
#
#   2) PyBindCore 是 pybind11 扩展模块（Python MODULE，非 DLL），内层 PRIVATE
#      链接 EngineCommon / Engine2D —— 独立构建按依赖顺序把它们连同
#      Utility / Log 一起拉进来构建。
#
#   3) 当前没有 C++ 侧测试（无 Test 目录）：它的契约是 Python 侧的
#      _apr_core 模块接口，验证放在 Python 集成测试层。示例程序同样
#      以 Python 脚本形态存在（Python/apr 包结构建立后再补），暂缓。
#
# 用法：
#   cmake -S PyBindCore -B PyBindCore/build
#   cmake --build PyBindCore/build --config Release --target AprPyBindCore
# ============================================================================

include(${CMAKE_CURRENT_LIST_DIR}/../Config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../CMake/StandaloneInit.cmake)

project(AprPyBindCoreModule
    VERSION 1.0.0
    DESCRIPTION "April CAD pybind11 facade extension (_apr_core)"
    LANGUAGES CXX
)

# MSVC 的 /W4 等选项必须在 project() 之后才能正确判定编译器
apr_setup_compiler_options()

# 集成构建时 APR_PYTHON_VERSION 由根 CMakeLists 定义；独立构建默认留空，
# 交给内层 find_package(Python 3.11) 自动匹配 vcpkg pybind11 配套的解释器，
# 这样每个人本地小版本不同也不必改这行；需要锁版本时用 -DAPR_PYTHON_VERSION=3.x。
if(NOT DEFINED APR_PYTHON_VERSION)
    set(APR_PYTHON_VERSION "")
endif()

# 本模块没有 C++ 测试，声明空测试范围（避免顺带编出整套 Engine 测试）。
# 空参数 = 全部模块不放行，不是「未声明」，这是单开关模型下保留的显式语义。
apr_only_own_tests()

# 严格按依赖顺序：Utility / Log 是叶子，Engine2D 依赖 EngineCommon。
apr_add_sibling(Utility      "${APR_UTILITY_DIR}")
apr_add_sibling(Log          "${APR_LOG_DIR}")
apr_add_sibling(EngineCommon "${APR_ENGINE_COMMON_DIR}")
apr_add_sibling(Engine2D     "${APR_ENGINE_2D_DIR}")

# 内层：创建 AprPyBindCore 扩展模块、构建后拷贝进 Python/apr、输出状态
add_subdirectory(PyBindCore)

# --- 独立构建摘要 -----------------------------------------------------------
set(_apr_python_display "${APR_PYTHON_VERSION}")
if(_apr_python_display STREQUAL "")
    set(_apr_python_display "auto (>= 3.11, matched from vcpkg pybind11)")
endif()
message(STATUS "")
message(STATUS "========================================")
message(STATUS "  [Standalone: ${PROJECT_NAME}]")
message(STATUS "========================================")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Python:         ${_apr_python_display}")
message(STATUS "  Output:         ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "========================================")
message(STATUS "")
