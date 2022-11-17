#!/bin/bash

current_dir=$(cd `dirname $0`;pwd)
root_dir=${current_dir}/../..
build_dir=${root_dir}/build
if [ -d ${build_dir} ];then
    rm -rf ${build_dir}/*
fi

project_32bit_dir=${build_dir}/project_32bit
project_64bit_dir=${build_dir}/project_64bit

mkdir -p ${project_32bit_dir}
mkdir -p ${project_64bit_dir}

cd ${project_32bit_dir}
cmake -G "Visual Studio 14 2015" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TEST=ON \
    -DWITH_LIB_PNG=OFF \
    -DWITH_LIB_JPEG_TURBO=ON \
    ../..

cd ${project_64bit_dir}
cmake -G "Visual Studio 14 2015 Win64" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TEST=ON \
    -DWITH_LIB_PNG=OFF \
    -DWITH_LIB_JPEG_TURBO=ON \
    ../..
