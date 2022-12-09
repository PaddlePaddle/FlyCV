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
#include <stdio.h>
#include <stdlib.h>

using namespace g_fcv_ns;

class ResizeTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p(gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p(gray_f32_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p(bgr_f32_src), 0);
        ASSERT_EQ(prepare_pkg_bgra_u8_720p(bgra_u8_src), 0);
    }

    Mat gray_u8_src;
    Mat gray_f32_src;
    Mat bgr_u8_src;
    Mat bgr_f32_src;
    Mat bgra_u8_src;
};

TEST_F(ResizeTest, GRAYU8InterLinearPositiveInput) {
    Mat dst;
    resize(gray_u8_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());

    std::vector<int> groundtruth = {65, 65, 63, 219, 109, 117, 201, 202, 203};

    for (size_t i = 0; i < C1_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[C1_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, PkgBGRU8InterLinearPositiveInput) {
    Mat dst;
    resize(bgr_u8_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());

    std::vector<int> groundtruth = {0, 86, 51, 85, 115, 122, 183, 159, 255};

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[C3_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, PkgBGRAU8InterLinearPositiveInput) {
    Mat dst;
    resize(bgra_u8_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());

    std::vector<int> groundtruth = {0, 86, 51, 103, 115, 122, 255, 184, 255};

    for (size_t i = 0; i < C4_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[C4_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, GRAYF32InterLinearPositiveInput) {
    Mat dst;
    resize(gray_f32_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    float* dst_data = reinterpret_cast<float*>(dst.data());
    
    std::vector<float> groundtruth = {65.25f, 65.0f, 62.75f, 218.75f,
            109.25f, 116.5f, 201.0f, 201.75f, 203.0f};

    for (size_t i = 0; i < C1_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], dst_data[C1_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, PkgBGRF32InterLinearPositiveInput) {
    Mat dst;
    resize(bgr_f32_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_LINEAR);
    float* dst_data = reinterpret_cast<float*>(dst.data());

    std::vector<float> groundtruth = {0.0f, 85.5f, 51.0f, 85.25f,
            115.25f, 122.0f, 183.25f, 159.0f, 255.0f};

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], dst_data[C3_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, GRAYU8InterCubicPositiveInput) {
    Mat dst;
    resize(gray_u8_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_CUBIC);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());

    std::vector<int> groundtruth = {64, 64, 62, 218, 108, 118, 201, 202, 203};

    for (size_t i = 0; i < C1_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], dst_data[C1_640X360_IDX[i]], 1);
    }
}

TEST_F(ResizeTest, PkgBGRU8InterCubicPositiveInput) {
    Mat dst;
    resize(bgr_u8_src, dst, Size(640, 360), 0, 0, InterpolationType::INTER_CUBIC);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());

    std::vector<int> groundtruth = {0, 84, 50, 85, 117, 125, 183, 159, 255};

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], dst_data[C3_640X360_IDX[i]], 1);
    }
}