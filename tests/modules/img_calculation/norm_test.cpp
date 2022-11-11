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

class NormTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT,
                FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8_src.data(),
                pkg_bgr_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        pkg_bgr_f32_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT,
                FCVImageType::PKG_BGR_F32);
        status = read_binary_file(BGR_1280X720_F32_BIN, pkg_bgr_f32_src.data(),
                pkg_bgr_f32_src.total_byte_size());
        EXPECT_EQ(status, 0);

    }

    Mat pkg_bgr_u8_src;
    Mat pkg_bgr_f32_src;
};

TEST_F(NormTest, NormL1PositiveInput) {
    double result1 = norm(pkg_bgr_u8_src, NormTypes::NORM_L1);
    ASSERT_DOUBLE_EQ(398655120, result1);

    double result2 = norm(pkg_bgr_f32_src, NormTypes::NORM_L1);
    ASSERT_DOUBLE_EQ(398655120, result2);
}

TEST_F(NormTest, NormL2PositiveInput) {
    double result1 = norm(pkg_bgr_u8_src);
    ASSERT_FLOAT_EQ(266023, result1);

    double result2 = norm(pkg_bgr_f32_src);
    ASSERT_FLOAT_EQ(266023, result2);
}

TEST_F(NormTest, NormINFPositiveInput) {
    double result1 = norm(pkg_bgr_u8_src, NormTypes::NORM_INF);
    ASSERT_FLOAT_EQ(255.0f, result1);

    double result2 = norm(pkg_bgr_f32_src, NormTypes::NORM_INF);
    ASSERT_FLOAT_EQ(255.0f, result2);
}
