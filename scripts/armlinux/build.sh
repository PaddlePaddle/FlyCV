#!/bin/bash
current_dir=$(cd `dirname $0`; pwd)
repository_dir=$(cd ${current_dir}/../..; pwd)
build_dir=${repository_dir}/build
rebuild=0

function build_armhf() {
    core_num=`cat /proc/cpuinfo | grep processor | wc -l`
    cd ${build_dir}
    cmake \
        -DCMAKE_INSTALL_PREFIX=${build_dir}/install \
        -DCMAKE_SYSTEM_PROCESSOR=armhf \
        -DCMAKE_TOOLCHAIN_FILE=${repository_dir}/cmake/platform/armlinux/toolchain/armlinux.toolchain.cmake \
        -DENABLE_NEON=ON \
        -DBUILD_TEST=ON \
        -DBUILD_SAMPLES=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_BENCHMARK=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_FCV_MEDIA_IO=ON \
        -DWITH_LIB_PNG=ON \
        -DWITH_LIB_JPEG_TURBO=ON \
        ..
    make -j${core_num}
    make install
}

function build_aarch64() {
    core_num=`cat /proc/cpuinfo | grep processor | wc -l`
    cd ${build_dir}
    cmake \
        -DCMAKE_INSTALL_PREFIX=${build_dir}/install \
        -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
        -DCMAKE_TOOLCHAIN_FILE=cmake/platform/armlinux/toolchain/armlinux.toolchain.cmake \
        -DENABLE_NEON=ON \
        -DBUILD_TEST=ON \
        -DBUILD_SAMPLES=ON \
        -DBUILD_SHARED_LIBS=OFF \
        -DBUILD_BENCHMARK=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_FCV_MEDIA_IO=ON \
        -DWITH_LIB_PNG=ON \
        -DWITH_LIB_JPEG_TURBO=ON \
        ..
    make -j${core_num}
    make install
}

if [ $# -lt 1 ];then
    echo "Select architecture by serial number:"
    echo "    0 armhf"
    echo "    1 arm64"
    read index
else
    index=$1
fi

case ${index} in
    0|armhf)
        arch=armhf
        ;;
    1|arm64)
        arch=arm64
        ;;
    *)
        echo "Unsupported architecture"
        exit 1
    ;;
esac

echo "The ${arch} has been chosen."
# echo "current dir is ${current_dir}"
# echo "repository_dir dir is ${repository_dir}"

if [ $# -lt 2 ];then
    echo "Do you need clean previous project files? [Y/N]"
    read -n 1 index
    echo ""
else
    index=${2}
fi

case ${index} in
    Y|y)
        rebuild=1
        ;;
    N|n)
        rebuild=0
        ;;
    *)
        echo "Unsupported arguments"
        exit 1
    ;;
esac

mkdir -p ${build_dir}
if [ ${rebuild} -eq 1 ];then
    echo "clean previous project files ..."
    rm -rf ${build_dir}/*
fi

case ${arch} in
    armhf)
        build_armhf
        ;;
    arm64)
        build_aarch64
        ;;
    *)
        echo "Unsupported architecture"
        exit 1
    ;;
esac
