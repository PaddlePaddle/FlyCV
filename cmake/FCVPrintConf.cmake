fcv_status_purple("========================== FlyCV Compile Details Start ==========================")
fcv_status_yellow("Version: ${FLYCV_VERSION}")
fcv_status_yellow("Git hash: ${GIT_HASH}")
fcv_status_yellow("Build Timestamp: ${BUILD_TIMESTAMP}")
fcv_status_purple("Compile Configuration:")
fcv_status_yellow("cmake_build_type: ${CMAKE_BUILD_TYPE}")
fcv_status_yellow("current system name: ${CMAKE_SYSTEM_NAME}")
fcv_status_yellow("current system processor: ${CMAKE_SYSTEM_PROCESSOR}")

if(ANDROID)
    fcv_status_yellow("android_ndk: ${ANDROID_NDK}")
    fcv_status_yellow("android_abi: ${ANDROID_ABI}")
    fcv_status_yellow("android_platform: ${ANDROID_PLATFORM}")
    fcv_status_purple("CMake Toolchain file:")
    fcv_status_yellow("${CMAKE_TOOLCHAIN_FILE}")
else()
    if(${CMAKE_TOOLCHAIN_FILE})
        fcv_status_purple("CMake Toolchain file:")
        fcv_status_yellow("${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

fcv_status_purple("Compilers Location:")
fcv_status_yellow("C Compiler: ${CMAKE_C_COMPILER}")
fcv_status_yellow("C++ Compiler: ${CMAKE_CXX_COMPILER}")

# === c/c++ options ===
fcv_status_purple("C/CXX Flags Configuration:")
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    fcv_status_yellow("C flags (Release): ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
    fcv_status_yellow("C++ flags (Release): ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    fcv_status_yellow("C flags (Debug): ${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
    fcv_status_yellow("C++ flags (Debug): ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
endif()

# === build module list ===
fcv_status_purple("Build Modules Configuration: ")
fcv_status_yellow("Enabled Modules: ${FCV_BUILD_MODULE_LIST}")
fcv_status_yellow("Build modules detail:")

foreach(module_name ${FCV_BUILD_MODULE_LIST})
    fcv_status_yellow("==> ${module_name}")

    set(SUBMODULE_LIST "")
    get_subdir_list(SUBMODULE_LIST ${CMAKE_CURRENT_SOURCE_DIR}/modules/${module_name})

    foreach(submodule_name ${SUBMODULE_LIST})
        string(TOUPPER ${submodule_name} _name)
        if(${WITH_FCV_${_name}})
            fcv_status_yellow("    |__ ${submodule_name}")
        endif()
    endforeach()
endforeach()

fcv_status_purple("Test Configuration:")
fcv_status_yellow("Unit Test: ${BUILD_TEST}")
fcv_status_yellow("Performance Test: ${BUILD_BENCHMARK}")

# === fcv target conf ===
fcv_status_purple("Target Configuration:")
fcv_status_yellow("Target name: ${FCV_TARGET_NAME}")
fcv_status_yellow("Target namespace: ${FLYCV_NAMESPACE}")
fcv_status_yellow("Target dependencies: ${FCV_LINK_DEPS}")
fcv_status_yellow("Install path: ${CMAKE_INSTALL_PREFIX}")

fcv_status_purple("========================== FlyCV Compile Details End ==========================")
