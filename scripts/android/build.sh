#!/bin/bash

current_dir=$(cd `dirname $0`;pwd)
root_dir=${current_dir}/../..
build_dir=${root_dir}/build
install_dir=${root_dir}/install

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

clean_build() {
    if [ -e ${build_dir} ];then
        rm -rf ${build_dir}
    fi
}

create_build() {
    mkdir -p ${build_dir}
}

compile() {

    clean_build
    create_build
    cd ${build_dir}

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
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_TEST=ON \
        -DBUILD_BENCHMARK=ON \
        -DWITH_LIB_JPEG_TURBO=ON \
        -DWITH_LIB_PNG=ON \
        ..

    make -j4
    make install

    cd ..
}

for var in ${archs[@]}
do
    compile $var
done
