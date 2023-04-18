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

class FcvCopyMakeBorderTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
};

TEST_F(FcvCopyMakeBorderTest, PkgBGRU8PositiveInput) {
    Mat dst;
    int top = 10;
    int bottom = 5;
    int left = 1;
    int right = 1;

    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width + left + right,
            pkg_bgr_u8_src->height + top + bottom, pkg_bgr_u8_src->type);
    CScalar s;
    s.val[0] = 255;
    s.val[1] = 255;
    s.val[2] = 255;

    int status = fcvCopyMakeBorder(pkg_bgr_u8_src, dst, top, bottom, left, right,
            CBorderType::BORDER_CONSTANT, &s);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src->data);

    for (int j = 0; j < (IMG_720P_WIDTH + left + right) * top * 3; j++) {
        ASSERT_EQ(dst_data[j], 255);
    }

    for (int i = 0; i < IMG_720P_HEIGHT; ++i) {
        unsigned char* src_row0 = src_data + i * pkg_bgr_u8_src->stride;
        unsigned char* dst_row0 = dst_data + (i + 10) * dst->stride;

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

    unsigned char* dst_bottom = dst_data + (IMG_720P_HEIGHT + top) * dst->stride;
    for (int j = 0; j < (IMG_720P_WIDTH + left + right) * bottom * 3; j++) {
        ASSERT_EQ(dst_bottom[j], 255);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvCopyMakeBorderTest, PkgBGRF32PositiveInput) {
    Mat dst;
    int top = 10;
    int bottom = 5;
    int left = 1;
    int right = 1;

    CMat* dst = fcvCreateCMat(pkg_bgr_f32_src->width + left + right,
            pkg_bgr_f32_src->height + top + bottom, pkg_bgr_f32_src->type);
    CScalar s;
    s.val[0] = 114;
    s.val[1] = 114;
    s.val[2] = 114;

    int status = fcvCopyMakeBorder(pkg_bgr_f32_src, dst, top, bottom, left, right,
            CBorderType::BORDER_CONSTANT, &s);
    ASSERT_EQ(status, 0);

    float* dst_data = reinterpret_cast<float*>(dst->data);
    std::vector<int> index = {0, 1, 1280, 1281, 1282, 3000, 4000,
            5000, 1382399, 1382400, 2764798, 2764799};
    std::vector<float> groundtruth = {114.0f, 114.0f, 114.0f, 114.0f,
            114.0f, 114.0f, 114.0f, 114.0f, 96.0f, 49.0f, 180.0f, 254.0f};

    for (size_t i = 0; i < index.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[index[i]]);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvCopyMakeBorderTest, NegativeInput) {
    CMat* empty_src = nullptr;
    CMat* dst = nullptr;
    int status = fcvCopyMakeBorder(empty_src, dst, 10, 4,
            20, 2, CBorderType::BORDER_CONSTANT, nullptr);
    EXPECT_NE(status, 0);
}
