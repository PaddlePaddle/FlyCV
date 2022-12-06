#!/bin/bash

echo "Start building FalconCV on x86_64 Linux platform..."

core_num=$(grep -c processor /proc/cpuinfo)
arch=$(uname -m)
CUDA_SW=OFF

if [ $# -lt 1 ];then
    echo "Select architecture by serial number:"
    echo "    0 CPU only"
    echo "    1 CPU with CUDA"
    read -r index
else
    index=$1
fi

case ${index} in
    0|cpu|CPU)
        CUDA_SW=OFF
        ;;
    1|cuda|CUDA)
        CUDA_SW=ON
        ;;
    *)
        echo "Unsupported select"
        exit 1
    ;;
esac

current_dir=$(cd `dirname $0`; pwd)
repository_dir=$(cd ${current_dir}/../..; pwd)
build_dir=${repository_dir}/build

if [ -d ${build_dir} ];then
    rm -rf ${build_dir}
fi

mkdir -p ${build_dir}

cd ${build_dir} || exit 0

cmake \
    -DENABLE_CUDA="${CUDA_SW}" \
    -DBUILD_TEST=ON \
    -DBUILD_BENCHMARK=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${build_dir}/install \
    -DBUILD_FCV_MEDIA_IO=ON \
    -DWITH_LIB_PNG=ON \
    -DWITH_LIB_JPEG_TURBO=ON \
    -DBUILD_SHARED_LIBS=OFF \
    ..

make -j"${core_num}"
make install
