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

class FlipTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        gray_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
        status = read_binary_file(GRAY_1280X720_U8_BIN,
                gray_u8_src.data(), gray_u8_src.total_byte_size());
        ASSERT_EQ(status, 0);

        bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN,
                bgr_u8_src.data(), bgr_u8_src.total_byte_size());
        ASSERT_EQ(status, 0);
    }

    Mat gray_u8_src;
    Mat bgr_u8_src;
};

TEST_F(FlipTest, FlipXPositiveInput) {
    Mat gray_u8_dst;
    int status = flip(gray_u8_src, gray_u8_dst, FlipType::X);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)gray_u8_src.data();
    unsigned char* dst_data = (unsigned char*)gray_u8_dst.data();

    const unsigned char* ptr_src = src_data;
    unsigned char* ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * gray_u8_dst.stride();

    int i = 0, j = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= gray_u8_dst.stride();
        ptr_src += gray_u8_src.stride();
    }

    Mat bgr_u8_dst;
    status = flip(bgr_u8_src, bgr_u8_dst, FlipType::X);
    ASSERT_EQ(status, 0);

    src_data = (unsigned char*)bgr_u8_src.data();
    dst_data = (unsigned char*)bgr_u8_dst.data();

    ptr_src = src_data;
    ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * bgr_u8_dst.stride();

    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= bgr_u8_dst.stride();
        ptr_src += bgr_u8_src.stride();
    }
}

TEST_F(FlipTest, FlipYPositiveInput) {
    Mat gray_u8_dst;
    int status = flip(gray_u8_src, gray_u8_dst, FlipType::Y);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)gray_u8_src.data();
    unsigned char* dst_data = (unsigned char*)gray_u8_dst.data();

    int i = 0, j = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + gray_u8_dst.stride() - 1;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row--), *(src_row++));
        }

        dst_data += gray_u8_dst.stride();
        src_data += gray_u8_src.stride();
    }

    Mat bgr_u8_dst;
    status = flip(bgr_u8_src, bgr_u8_dst, FlipType::Y);
    ASSERT_EQ(status, 0);

    src_data = (unsigned char*)bgr_u8_src.data();
    dst_data = (unsigned char*)bgr_u8_dst.data();


    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + bgr_u8_dst.stride() - 3;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row++), *(src_row++));
            ASSERT_EQ(*(dst_row++), *(src_row++));
            ASSERT_EQ(*(dst_row++), *(src_row++));
            dst_row -= 6;
        }

        dst_data += bgr_u8_dst.stride();
        src_data += bgr_u8_src.stride();
    }
}
