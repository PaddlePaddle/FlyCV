// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "test_util.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "gtest/gtest.h"

using namespace g_fcv_ns;

int read_binary_file(
        const std::string& file_path,
        void* buffer,
        long length) {
    std::ifstream ifs(file_path, std::ios::binary);

    if (!ifs) {
        std::cout << "Cannot read file: " << file_path << std::endl;
        return -1;
    }

    long file_len = 0;

    ifs.seekg(0, std::ios::end);
    file_len = ifs.tellg();
    ifs.seekg (0, std::ios::beg);

    ifs.read((char*)buffer, std::min(file_len, length));
    ifs.close();

    return 0;
}

int write_binary_file(
        const std::string& file_path,
        void* buffer,
        long length) {
    std::ofstream ofs(file_path.c_str(), std::ios::binary);

    if (!ofs) {
        printf("Cannot write file: %s", file_path.c_str());
        return -1;
    }

    ofs.write((char*)buffer, length);
    ofs.close();

    return 0;
}

int prepare_gray_u8_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH,
            IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
    int status = read_binary_file(GRAY_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_gray_u16_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U16);
    int status = read_binary_file(GRAY_1280X720_U16_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_gray_f32_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);
    int status = read_binary_file(GRAY_1280X720_F32_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_pkg_rgb_u8_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_RGB_U8);
    int status = read_binary_file(RGB_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_pkg_bgr_u8_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
    int status = read_binary_file(BGR_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_pkg_bgr_u8_720p_2(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
    int status = read_binary_file(BGR_1280X720_U8_2_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_pkg_bgr_f32_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_F32);
    int status = read_binary_file(BGR_1280X720_F32_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_pkg_bgra_u8_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGRA_U8);
    int status = read_binary_file(BGRA_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_nv12_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV12);
    int status = read_binary_file(NV12_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_nv21_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV21);
    int status = read_binary_file(NV21_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

int prepare_i420_720p(Mat& src) {
    src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::I420);
    int status = read_binary_file(I420_1280X720_U8_BIN,
            src.data(), src.total_byte_size());
    return status;
}

#ifdef USE_C_API
int prepare_gray_u8_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::GRAY_U8);
    int status = read_binary_file(GRAY_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_gray_u16_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::GRAY_U16);
    int status = read_binary_file(GRAY_1280X720_U16_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;

}

int prepare_gray_f32_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::GRAY_F32);
    int status = read_binary_file(GRAY_1280X720_F32_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_pkg_rgb_u8_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_RGB_U8);
    int status = read_binary_file(RGB_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;

}

int prepare_pkg_bgr_u8_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_BGR_U8);
    int status = read_binary_file(BGR_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_pkg_bgr_u8_720p_2_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_BGR_U8);
    int status = read_binary_file(BGR_1280X720_U8_2_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_pkg_bgr_f32_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_BGR_F32);
    int status = read_binary_file(BGR_1280X720_F32_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_pkg_bgra_u8_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_BGRA_U8);
    int status = read_binary_file(BGRA_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_nv12_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::NV12);
    int status = read_binary_file(NV12_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_nv21_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::NV21);
    int status = read_binary_file(NV21_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

int prepare_i420_720p_cmat(CMat** src) {
    *src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::I420);
    int status = read_binary_file(I420_1280X720_U8_BIN,
            (*src)->data, (*src)->total_byte_size);
    return status;
}

#endif