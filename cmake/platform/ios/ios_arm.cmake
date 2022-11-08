set(CMAKE_CFLAGS "${CMAKE_CFLAGS} -std=c++11 -O3 -fPIC -ftree-vectorize -ffast-math -fno-stack-protector")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -fPIC -ftree-vectorize -ffast-math -fno-stack-protector -fexceptions")
if(NOT DEFINED CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER clang)
endif()

if(NOT DEFINED CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER clang++)
endif()
list(FIND CMAKE_OSX_ARCHITECTURES "arm64" ARM64_INDEX)

set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "$ENV{XCODE_ATTRIBUTE_DEVELOPMENT_TEAM}")
if(ENABLE_NEON)
    add_definitions(-DHAVE_NEON)
endif()

add_compile_options(-Werror=return-type)
add_compile_options(-Wall -Wextra)
add_compile_options(-fPIC)
add_compile_options(-O3)
add_compile_options(-ffast-math)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(FCV_INSTALL_LIB_NAME lib)

if(BUILD_SHARED_LIBS MATCHES "ON")
list (APPEND FCV_TARGET_PROPERTIES FRAMEWORK TRUE)
endif()
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version" FORCE)

