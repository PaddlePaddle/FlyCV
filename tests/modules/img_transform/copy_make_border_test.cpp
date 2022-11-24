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

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

class CopyMakeBorderTest : public ::testing::Test {
protected:
    void SetUp() override {
        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);

        int status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8_src.data(),
                pkg_bgr_u8_src.total_byte_size());
        ASSERT_EQ(status, 0);

        pkg_bgr_f32_src =  Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_F32);
        status = read_binary_file(BGR_1280X720_F32_BIN, pkg_bgr_f32_src.data(),
                pkg_bgr_f32_src.total_byte_size());
        ASSERT_EQ(status, 0);
    }

    Mat pkg_bgr_u8_src;
    Mat pkg_bgr_f32_src;
};

TEST_F(CopyMakeBorderTest, PkgBGRU8PositiveInput) {
    Mat dst;
    int top = 10;
    int bottom = 5;
    int left = 1;
    int right = 1;

    int status = copy_make_border(pkg_bgr_u8_src, dst, top, bottom, left, right,
            BorderType::BORDER_CONSTANT, Scalar(255, 255, 255));
    EXPECT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());
    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src.data());

    for (int j = 0; j < (IMG_720P_WIDTH + left + right) * top * 3; j++) {
        ASSERT_EQ(dst_data[j], 255);
    }

    for (int i = 0; i < IMG_720P_HEIGHT; ++i) {
        unsigned char* src_row0 = src_data + i * pkg_bgr_u8_src.stride();
        unsigned char* dst_row0 = dst_data + (i + 10) * dst.stride();

        ASSERT_EQ(dst_row0[0], 255);
        ASSERT_EQ(dst_row0[1], 255);
        ASSERT_EQ(dst_row0[2], 255);
        for (int j = 3; j < IMG_720P_WIDTH * 3; j++) {
            ASSERT_EQ(dst_row0[j], src_row0[j - 3]);
        }

        ASSERT_EQ(dst_row0[IMG_720P_WIDTH * 3 + 3], 255);
        ASSERT_EQ(dst_row0[IMG_720P_WIDTH * 3 + 4], 255);
        ASSERT_EQ(dst_row0[IMG_720P_WIDTH * 3 + 5], 255);
    }

    unsigned char* dst_bottom = dst_data + (IMG_720P_HEIGHT + top) * dst.stride();
    for (int j = 0; j < (IMG_720P_WIDTH + left + right) * bottom * 3; j++) {
        ASSERT_EQ(dst_bottom[j], 255);
    }
}

TEST_F(CopyMakeBorderTest, PkgBGRF32PositiveInput) {
    Mat dst;
    int top = 10;
    int bottom = 5;
    int left = 1;
    int right = 1;

    int status = copy_make_border(pkg_bgr_f32_src, dst, top, bottom, left, right,
            BorderType::BORDER_CONSTANT, Scalar(114, 114, 114));
    EXPECT_EQ(status, 0);

    float* dst_data = reinterpret_cast<float*>(dst.data());
    std::vector<int> index = {0, 1, 1280, 1281, 1282, 3000, 4000,
            5000, 1382399, 1382400, 2764798, 2764799};
    std::vector<float> groundtruth = {114.0f, 114.0f, 114.0f, 114.0f,
            114.0f, 114.0f, 114.0f, 114.0f, 96.0f, 49.0f, 180.0f, 254.0f};

    for (size_t i = 0; i < index.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[index[i]]);
    }
}

TEST_F(CopyMakeBorderTest, NegativeInput) {
    Mat empty_src;
    Mat dst;
    int status = copy_make_border(empty_src, dst,
            10, 4, 20, 2, BorderType::BORDER_CONSTANT,
            Scalar(255, 255, 255));
    EXPECT_NE(status, 0);
}
