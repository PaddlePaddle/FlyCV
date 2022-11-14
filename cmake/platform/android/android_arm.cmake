if(NOT DEFINED ANDROID_NDK AND NOT DEFINED ENV{ANDROID_NDK})
    fcv_error("android ndk error")
endif()

add_compile_options(-Werror=return-type)
add_compile_options(-Wall -Wextra)
add_compile_options(-fPIC)
add_compile_options(-Ofast)
add_compile_options(-ffast-math)
add_compile_options(-Qunused-arguments)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -O0")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -Ofast -DNDEBUG -s")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -Ofast -DNDEBUG  -std=c++11 -s")
endif()

if(ANDROID_ARM_NEON)
    set(ENABLE_NEON ON)
    add_definitions(-DHAVE_NEON)
else()
    set(ENABLE_NEON OFF)
endif()

if(ENABLE_SVE2)
    add_definitions(-DHAVE_SVE2)
    set(CMAKE_C_FLAGS    "${CMAKE_C_FLAGS} -march=armv8.2-a+sve2+fp16+dotprod+f32mm+i8mm+nolse")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -march=armv8.2-a+sve2+fp16+dotprod+f32mm+i8mm+nolse")
endif()

if(WITH_FCV_OPENCL)
    add_definitions(-DHAVE_OPENCL)
endif()

find_library(
    log-lib
    log)

list(APPEND FCV_LINK_DEPS ${log-lib})

set(FCV_INSTALL_LIB_NAME ${CMAKE_ANDROID_ARCH_ABI})
