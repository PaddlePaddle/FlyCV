# disable in source compile, prevent conflicts with source file directories
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "in-source builds are not allowed.
    You should create a separate directory for build files.")
endif()

if(CMAKE_GENERATOR MATCHES Xcode)
    cmake_minimum_required(VERSION 3.12)
else()
    cmake_minimum_required(VERSION 3.10)
endif()

project(flycv C CXX)

#[[
    @brief  Set the default namespace of flycv, users can customize by
    adding compile options: -DFLYCV_NAMESPACE="xxx".
]]
set(FLYCV_NAMESPACE fcv CACHE STRING "fcv")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/")
list(APPEND CMAKE_FIND_ROOT_PATH "${CMAKE_SOURCE_DIR}/third_party/")

include(FCVVersion)
include(FCVUtils)
include(FCVModuleCollect)

get_git_branch(FLYCV_VERSION)
get_git_hash(GIT_HASH)
string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")

option(BUILD_C "Turn this ON when you need native C interface" OFF)
option(BUILD_TEST "Turn this ON when build flycv tests" OFF)
option(BUILD_BENCHMARK "Turn this ON when build flycv benchmarks" OFF)
option(BUILD_SAMPLES "Turn this ON when build flycv samples" OFF)

# set the external dependencies
set(FCV_EXTERNAL_DEPS)

# list the libraries the flycv need to link
set(FCV_LINK_DEPS)

# list the dependency libraries need to export
set(FCV_EXPORT_LIBS)

# set the name of the output lib directory
set(FCV_INSTALL_LIB_NAME "")

# set the target name of flycv
set(FCV_TARGET_NAME "")

# set the properties of the target
set(FCV_TARGET_PROPERTIES)

include(FCVPlatforms)

# ======================= Build Module List  ===========================
include(FCVBuildList)
include(FCVFindDependencies)

if (BUILD_C)
    set(WITH_FCV_CMAT ON)
    set(USE_C_API ON)
else()
    set(WITH_FCV_CMAT OFF)
endif()

if(BUILD_SHARED_LIBS)
    set(FCV_TARGET_NAME "flycv_shared")
else()
    set(FCV_TARGET_NAME "flycv_static")
endif()

# ======================= Install Configuration ===========================
# switch default install path to build directory
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR} CACHE STRING "default install dir" FORCE)
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/flycv_namespace.h.in"
    "${CMAKE_INSTALL_PREFIX}/flycv/include/flycv_namespace.h")

configure_file(
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_INSTALL_PREFIX}/flycv/include/version.h")

# add modules
add_subdirectory(modules)

configure_file(
    "${PROJECT_SOURCE_DIR}/include/flycv.h.in"
    "${CMAKE_INSTALL_PREFIX}/flycv/include/flycv.h")

# ============================ Unit Test ==================================
# Add gtest
if(BUILD_TEST)
    fcv_download_dependency(
        "https://github.com/google/googletest.git"
        release-1.12.1
        gtest
        ${PROJECT_SOURCE_DIR}/third_party)

    cmake_policy(PUSH)
    option(BUILD_GMOCK "Builds the googlemock subproject" OFF)
    option(
        gtest_force_shared_crt
        "Use shared (DLL) run-time lib even when Google Test is built as static lib."
        ON)
    # set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set_if_not_defined(GTEST_DIR third_party/gtest)
    add_subdirectory(${GTEST_DIR})
    set(GTEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/gtest/googletest/include)
    list(APPEND GTEST_LIBRARIES gtest gtest_main)
    cmake_policy(POP)
endif(BUILD_TEST)

if(BUILD_TEST)
    add_subdirectory(tests)
endif()

# ============================ Benchmark ==================================
# Add benchmark
if(BUILD_BENCHMARK)
    fcv_download_dependency(
        "https://github.com/google/benchmark.git"
        v1.6.1
        benchmark
        ${PROJECT_SOURCE_DIR}/third_party)

    cmake_policy(PUSH)
    set_if_not_defined(BENCHMARK_DIR third_party/benchmark)
    set(BENCHMARK_ENABLE_TESTING FALSE CACHE BOOL FALSE FORCE)
    add_subdirectory(${BENCHMARK_DIR})
    set(BENCHMARK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/benchmark/include)
    list(APPEND BENCHMARK_LIBRARIES benchmark benchmark_main)
    cmake_policy(POP)
endif(BUILD_BENCHMARK)

if(BUILD_BENCHMARK)
    add_subdirectory(benchmark)
endif()

# ============================ Sample ==================================
# Add sample
if(BUILD_SAMPLES)
    fcv_download_dependency(
        "https://github.com/gflags/gflags.git"
        v2.2.2
        gflags
        ${PROJECT_SOURCE_DIR}/third_party)

    cmake_policy(PUSH)
    set_if_not_defined(GFLAGS_DIR third_party/gflags)
    set(GFLAGS_INSTALL_HEADERS ON)
    set(GFLAGS_BUILD_gflags_LIB ON)
    add_subdirectory(${GFLAGS_DIR})
    cmake_policy(POP)
endif(BUILD_SAMPLES)

if(BUILD_SAMPLES)
    add_subdirectory(samples)
endif()

# ====================== Print Compile Configuration ======================
include(FCVPrintConf)
