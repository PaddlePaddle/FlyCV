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
rm -rf ${build_dir}/*
cd ${build_dir}

cmake \
    -DCMAKE_C_COMPILER=${EMSCRIPTEN_DIR}/emcc \
    -DCMAKE_CXX_COMPILER=${EMSCRIPTEN_DIR}/em++ \
    -DCMAKE_TOOLCHAIN_FILE=${EMSCRIPTEN_DIR}/cmake/Modules/Platform/Emscripten.cmake \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/output \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_FCV_MEDIA_IO=OFF \
    -DBUILD_JS=ON \
    ..

make -j4
