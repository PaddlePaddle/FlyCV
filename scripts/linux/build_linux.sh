#!/bin/bash

echo "Start building FalconCV on x86_64 Linux platform..."
current_dir=$(cd `dirname $0`; pwd)
repository_dir=$(cd ${current_dir}/../..; pwd)
build_dir=${repository_dir}/build

if [ -d ${build_dir} ];then
    rm -rf ${build_dir}
fi

mkdir -p build
cd build
cmake \
    -DBUILD_TEST=ON \
    -DBUILD_BENCHMARK=ON \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/install \
    -DWITH_LIB_PNG=ON \
    -DWITH_LIB_JPEG_TURBO=ON \
    -DCMAKE_BUILD_TYPE=Release \
    ..

make -j${nproc}
make install
