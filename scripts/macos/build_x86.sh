#!/bin/bash

current_dir=$(cd `dirname $0`;pwd)
root_dir=${current_dir}/../..
build_dir=${root_dir}/build
if [ -d ${build_dir} ];then
    if [ "$1" = "clean" ];then
        rm -rf ${build_dir}
    fi
fi

mkdir -p ${build_dir}
cd ${build_dir}

echo "Start configure x86 MacOS project ..."
cmake \
    "-DCMAKE_OSX_ARCHITECTURES=x86_64" \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/output \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DBUILD_TEST=ON \
    -DBUILD_BENCHMARK=ON \
    ..

make -j8
make install
