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

#include "flycv.h"
#include "gtest/gtest.h"
#include "test_util.h"

using namespace g_fcv_ns;

class FcvCircleTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(pkg_bgr_u8);
        pkg_bgr_u8 = nullptr;
    }

    CMat* pkg_bgr_u8 = nullptr;
};

TEST_F(FcvCircleTest, CirclePositiveInput) {
    CPoint solid_point = {100, 100};
    CPoint hollow = {300, 300};
    CScalar s;
    s.val[0] = 255;
    s.val[1] = 255;
    s.val[2] = 255;

    fcvCircle(pkg_bgr_u8, solid_point, 100, &s, -1, CLineType::LINE_8, 0);
    fcvCircle(pkg_bgr_u8, hollow, 100, &s, 1, CLineType::LINE_8, 0);
    unsigned char* image_data = (unsigned char*)pkg_bgr_u8->data;
    double average_pixels = 0;

    for (size_t i = 0; i < pkg_bgr_u8->total_byte_size; i++) {
        average_pixels += image_data[i];
    }

    average_pixels /= pkg_bgr_u8->total_byte_size;

    // 对比图像像素均值
    double groudthruth = 150.110037;
    ASSERT_NEAR(groudthruth, average_pixels, 10e-6);
}
