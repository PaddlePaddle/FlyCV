// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
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

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

class WarpAffineTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN,
                pkg_bgr_u8_src.data(), pkg_bgr_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        pkg_bgr_f32_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_F32);
        status = read_binary_file(BGR_1280X720_F32_BIN,
                pkg_bgr_f32_src.data(), pkg_bgr_f32_src.total_byte_size());
        EXPECT_EQ(status, 0);

        gray_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
        status = read_binary_file(GRAY_1280X720_U8_BIN, gray_u8_src.data(),
                gray_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        gray_f32_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);
        status = read_binary_file(GRAY_1280X720_F32_BIN, gray_f32_src.data(),
                gray_f32_src.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat pkg_bgr_u8_src;
    Mat pkg_bgr_f32_src;
    Mat gray_u8_src;
    Mat gray_f32_src;
};

TEST_F(WarpAffineTest, GrayU8PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    Mat matrix(3, 2, FCVImageType::GRAY_F32, m);

    int status;
    Mat gray_u8_dst;
    std::vector<int> groundtruth_gray = {62, 58, 55, 0, 84, 148, 0, 0, 0};

    status = warp_affine(gray_u8_src, gray_u8_dst, matrix);
    ASSERT_EQ(status, 0);

    unsigned char* data = reinterpret_cast<unsigned char*>(gray_u8_dst.data());

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR((int)data[C1_1280X720_IDX[i]], groundtruth_gray[i], 1);
    }
}

TEST_F(WarpAffineTest, PkgBGRU8PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    Mat matrix(3, 2, FCVImageType::GRAY_F32, m);

    int status;
    Mat pkg_bgr_u8_dst;
    status = warp_affine(pkg_bgr_u8_src, pkg_bgr_u8_dst, matrix);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth_bgr = {0, 82, 47, 0, 77, 96, 0, 0, 0};
    unsigned char* data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_dst.data());

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR((int)data[C3_1280X720_IDX[i]], groundtruth_bgr[i], 1);
    }
}

TEST_F(WarpAffineTest, GrayF32PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
    Mat gray_f32_dst;

    int status = warp_affine(gray_f32_src, gray_f32_dst, matrix);
    ASSERT_EQ(status, 0);

    std::vector<float> groundtruth_gray = {62.0f, 58.0f, 54.84375f,
            0.0f, 84.472656f, 147.957031f, 0.0f, 0.0f, 0.0f};

    float* gray_data = reinterpret_cast<float*>(gray_f32_dst.data());

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(gray_data[C1_1280X720_IDX[i]], groundtruth_gray[i], 10e-6);
    }
}

TEST_F(WarpAffineTest, PkgBGRF32PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    Mat matrix(3, 2, FCVImageType::GRAY_F32, m);
    Mat pkg_bgr_f32_dst;

    int status = warp_affine(pkg_bgr_f32_src, pkg_bgr_f32_dst, matrix);
    ASSERT_EQ(status, 0);

    std::vector<int> index = {0, 3, 6, 100, 1000, 1382400, 1843200};
    std::vector<float> groundtruth_bgr = {0.0f, 82.0f, 47.0f, 0.0f, 76.675781f,
            96.390625f, 0.0f, 0.0f, 0.0f};

    float* bgr_data = reinterpret_cast<float*>(pkg_bgr_f32_dst.data());

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(bgr_data[C3_1280X720_IDX[i]], groundtruth_bgr[i]);
    }
}
