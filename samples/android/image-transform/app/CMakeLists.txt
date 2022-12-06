cmake_minimum_required(VERSION 3.10)
set(CMAKE_VERBOSE_MAKEFILE on)

set(VERSION "1.0")
set(NAMEKEY "FlyCV")
set(NAME ${NAMEKEY}_${VERSION})

# 1）module libs
# set(FlyCV_ROOT ../flycv/src/main/flycv/)

# 2）build libs
set(FlyCV_ROOT ../../../../build/install/flycv/)

include_directories(${FlyCV_ROOT}/include)
link_directories(${FlyCV_ROOT}/${CMAKE_ANDROID_ARCH_ABI}/)

#### source cpp file
FILE(GLOB SOURCE_FILES
        src/main/jni/utils/*.cpp
        src/main/jni/Image_Transform_jni.cpp
        )

add_library(${NAME} SHARED ${SOURCE_FILES})

find_library(LOG_LIB log)

target_link_libraries(
        ${NAME}
        -Wl,--start-group
        flycv_static png16 turbojpeg z
        # flycv_static
        -Wl,--end-group
        ${LOG_LIB})