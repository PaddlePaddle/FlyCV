set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# append your customize tool chain path to TOOLCHAIN_PATH for searching
set(TOOLCHAIN_PATH /opt/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf)
set(CMAKE_FIND_ROOT_PATH
    /opt/gcc/linux-x86/arm/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf
)

# set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)

set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-linux-gnueabihf-g++)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/bin/arm-linux-gnueabihf-gcc)

# set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
# set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# set build platform 
set(TARGET_PLATFORM "rv1109")
# search for rga
set(PLATFORM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/${TARGET_PLATFORM}/include)

set(FCV_INSTALL_LIB_NAME ${CMAKE_SYSTEM_PROCESSOR})

list(APPEND FCV_EXPORT_LIBS
    ${CMAKE_SOURCE_DIR}/third_party/${TARGET_PLATFORM}/lib/librga.so
    ${CMAKE_SOURCE_DIR}/third_party/${TARGET_PLATFORM}/lib/libdrm.so
    )

include_directories(${PLATFORM_INCLUDE_DIR})
