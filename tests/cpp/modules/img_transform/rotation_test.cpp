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

class RotationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p(gray_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgra_u8_720p(pkg_bgra_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p(gray_f32_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p(pkg_bgr_f32_src), 0);
    }

    Mat gray_u8_src;
    Mat pkg_bgr_u8_src;
    Mat pkg_bgra_u8_src;
    Mat gray_f32_src;
    Mat pkg_bgr_f32_src;
};

TEST_F(RotationTest, U8PositiveInput) {
    Mat gray_u8_dst;
    Mat pkg_bgr_u8_dst;
    Mat pkg_bgra_u8_dst;
    int status;
    status = transpose(gray_u8_src, gray_u8_dst);
    EXPECT_EQ(status, 0);
    status = transpose(pkg_bgr_u8_src, pkg_bgr_u8_dst);
    EXPECT_EQ(status, 0);
    status = transpose(pkg_bgra_u8_src, pkg_bgra_u8_dst);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(gray_u8_dst.data());
    int sc = gray_u8_src.channels();

    int i = 0, j = 0, k = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += gray_u8_dst.stride();
        }

        dst_data += sc;
        src_data += gray_u8_src.stride();
    }

    src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src.data());
    dst_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_dst.data());
    sc = pkg_bgr_u8_src.channels();

    for (; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += pkg_bgr_u8_dst.stride();
        }

        dst_data += sc;
        src_data += pkg_bgr_u8_src.stride();
    }

    src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src.data());
    dst_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_dst.data());
    sc = pkg_bgr_u8_src.channels();

    for (; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += pkg_bgra_u8_dst.stride();
        }

        dst_data += sc;
        src_data += pkg_bgr_u8_src.stride();
    }
}

TEST_F(RotationTest, F32PositiveInput) {
    Mat gray_f32_dst;
    Mat pkg_bgr_f32_dst;

    int status;
    status = transpose(gray_f32_src, gray_f32_dst);
    EXPECT_EQ(status, 0);
    status = transpose(pkg_bgr_f32_src, pkg_bgr_f32_dst);
    EXPECT_EQ(status, 0);

    float* src_data = reinterpret_cast<float*>(gray_f32_src.data());
    float* dst_data = reinterpret_cast<float*>(gray_f32_dst.data());
    int sc = gray_f32_src.channels();

    int i = 0, j = 0, k = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        float* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += (gray_f32_dst.stride() / sizeof(float));
        }

        dst_data += sc;
        src_data += (gray_f32_src.stride() / sizeof(float));
    }

    src_data = reinterpret_cast<float*>(pkg_bgr_f32_src.data());
    dst_data = reinterpret_cast<float*>(pkg_bgr_f32_dst.data());
    sc = pkg_bgr_u8_src.channels();

    for (; i < IMG_720P_HEIGHT; i++) {
        float* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += (pkg_bgr_f32_dst.stride() / sizeof(float));
        }

        dst_data += sc;
        src_data += (pkg_bgr_u8_src.stride() / sizeof(float));
    }
}


