#!/bin/bash

current_dir=$(cd `dirname $0`;pwd)
root_dir=${current_dir}/../..
build_dir=${root_dir}/build
if [ -d ${build_dir} ];then
    if [ "$1" = "clean" ];then
        rm -rf ${build_dir}
    fi
fi
cd ${root_dir}/modules

mkdir -p ${build_dir}
cd ${build_dir}

echo "Start configure iPhone project ..."
cmake -B_builds -G"Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=./cmake/platform/ios/toolchain/iOS.cmake \
    "-DCMAKE_OSX_ARCHITECTURES=arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=9.3 \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/Release \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DENABLE_NEON=ON \
    ..

cmake --build _builds --config Release --target install
cp -R ${build_dir}/Release/flycv/include  ${build_dir}/Release/flycv/lib/flycv_shared.framework/Headers
