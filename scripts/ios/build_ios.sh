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
cmake .. -GXcode \
    "-DCMAKE_OSX_ARCHITECTURES=arm64" \
    -DCMAKE_TOOLCHAIN_FILE=./cmake/platform/ios/toolchain/iOS.cmake \
    -DCMAKE_SYSTEM_NAME=ios \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/Release \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DENABLE_NEON=ON \
    -DWITH_LIB_PNG=OFF \
            -DOPENMP_ENABLE=OFF \
        ..
make -j8
make install

xcodebuild -sdk iphoneos build -project falconcv.xcodeproj -scheme install -configuration Release



