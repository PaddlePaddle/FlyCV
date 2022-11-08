add_compile_options(/W4)
set(CMAKE_CXX_FLAGS_RELEASE /MD)
set(CMAKE_CXX_FLAGS_DEBUG /MDd)
set(CMAKE_CXX_STANDARD 11)

if(CMAKE_BUILD_TYPE STREQUAL Release)
    add_compile_options(/O2)
else()
    add_compile_options(/O0)
endif()

if(CMAKE_CL_64)
    set(FCV_INSTALL_LIB_NAME x64)
else()
    set(FCV_INSTALL_LIB_NAME x86)
endif(CMAKE_CL_64)

option(ENABLE_SSE "default open sse support" ON)
option(ENABLE_SSE2 "default open sse2 support" ON)
option(ENABLE_SSE3 "default open sse2 support" ON)
option(ENABLE_SSE4_1 "default open sse2 support" ON)
option(ENABLE_SSE4_2 "default open sse2 support" ON)
option(ENABLE_AVX "default open avx support" ON)
option(ENABLE_AVX2 "default open avx2 support" ON)
option(ENABLE_AVX512 "default open avx512 support" ON)

if(ENABLE_SSE)
    add_definitions(-DHAVE_SSE)
endif()

if(ENABLE_SSE2)
    add_definitions(-DHAVE_SSE2)
endif()

if(ENABLE_SSE3)
    add_definitions(-DHAVE_SSE3)
endif()

if(ENABLE_SSE4_1)
    add_definitions(-DHAVE_SSE4_1)
endif()

if(ENABLE_SSE4_2)
    add_definitions(-DHAVE_SSE4_2)
endif()

if(ENABLE_AVX)
    add_definitions(-DHAVE_AVX)
endif()

if(ENABLE_AVX2)
    add_definitions(-DHAVE_AVX2)
endif()

if(ENABLE_AVX2)
    add_definitions(-DHAVE_AVX512)
endif()