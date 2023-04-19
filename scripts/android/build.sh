#!/bin/bash

current_dir=$(cd `dirname $0`;pwd)
root_dir=${current_dir}/../..
build_dir=${root_dir}/build
install_dir=${build_dir}/install

clean_build() {
    if [ -e ${build_dir} ];then
        rm -rf ${build_dir}
    fi
}

create_build() {
    mkdir -p ${build_dir}
}

compile() {

    mkdir -p ${build_dir}/$1
    cd ${build_dir}/$1

    cmake \
        -DCMAKE_INSTALL_PREFIX=${install_dir} \
        -DCMAKE_SYSTEM_NAME=Android \
        -DANDROID_NDK=${ANDROID_NDK} \
        -DCMAKE_TOOLCHAIN_FILE="${ANDROID_NDK}"/build/cmake/android.toolchain.cmake \
        -DANDROID_ABI=$1 \
        -DANDROID_STL=c++_static \
        -DANDROID_PLATFORM=${android_platform} \
        -DANDROID_ARM_NEON=ON \
    	-DENABLE_SVE2=${arm64_with_sve2} \
        -DBUILD_SHARED_LIBS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TEST=ON \
        -DBUILD_BENCHMARK=ON \
        -DWITH_LIB_JPEG_TURBO=ON \
        -DWITH_LIB_PNG=ON \
        -DBUILD_C=OFF \
        ../..

    make -j4
    make install

    cd ..
}

if [ $# -lt 1 ];then
    echo "Select architecture by serial number:"
    echo "    0 armeabi-v7a"
    echo "    1 arm64-v8a"
    echo "    2 armeabi-v7a arm64-v8a"
    read -r index
else
    index=$1
fi

case ${index} in
    0|armeabi-v7a)
        archs=("armeabi-v7a")
        android_platform=android-21
        arm64_with_sve2=OFF
        ;;
    1|arm64-v8a)
        archs=("arm64-v8a")
        android_platform=android-21
        arm64_with_sve2=OFF		
        ;;
    2)
        archs=("armeabi-v7a" "arm64-v8a")
        android_platform=android-21
        ;;
    *)
        echo "Unsupported architecture"
        exit 1
    ;;
esac

echo "The ${archs[0]} ${archs[1]} has been chosen."
# echo "current dir is ${current_dir}"
# echo "repository_dir dir is ${repository_dir}"

if [ $# -lt 2 ];then
echo "Do you need clean previous project files? [Y/N]"
read -n 1 index
echo ""
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
else
    rebuild=1
fi

create_build

if [ ${rebuild} -eq 1 ];then
    echo "clean previous project files ..."
    clean_build
fi

for var in ${archs[@]}
do
    compile $var
done
