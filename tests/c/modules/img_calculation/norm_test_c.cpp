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

class FcvNormTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
    }

    void TearDown() override {
        release_cmat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        release_cmat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
};

TEST_F(FcvNormTest, NormL1PositiveInput) {
    double result1 = fcvNorm(pkg_bgr_u8_src, CNormType::NORM_L1);
    ASSERT_DOUBLE_EQ(398655120, result1);

    double result2 = fcvNorm(pkg_bgr_f32_src, CNormType::NORM_L1);
    ASSERT_DOUBLE_EQ(398655120, result2);
}

TEST_F(FcvNormTest, NormL2PositiveInput) {
    double result1 = fcvNorm(pkg_bgr_u8_src, CNormType::NORM_L2);
    ASSERT_FLOAT_EQ(266023, result1);

    double result2 = fcvNorm(pkg_bgr_f32_src, CNormType::NORM_L2);
    ASSERT_FLOAT_EQ(266023, result2);
}

TEST_F(FcvNormTest, NormINFPositiveInput) {
    double result1 = fcvNorm(pkg_bgr_u8_src, CNormType::NORM_INF);
    ASSERT_FLOAT_EQ(255.0f, result1);

    double result2 = fcvNorm(pkg_bgr_f32_src, CNormType::NORM_INF);
    ASSERT_FLOAT_EQ(255.0f, result2);
}