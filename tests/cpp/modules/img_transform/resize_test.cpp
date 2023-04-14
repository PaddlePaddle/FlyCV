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

#include <thread>
#include <vector>

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

template<class T, class D>
static void resize_single_thread(
        Mat& src,
        Size size,
        float fx,
        float fy,
        InterpolationType type,
        const std::vector<int>& gt_index,
        const std::vector<T>& gt,
        T precision,
        int loop_cnt = 1000) {
    for (int i = 0; i < loop_cnt; ++i) {
        Mat dst;
        resize(src, dst, size, fx, fy, type);
        D* dst_data = reinterpret_cast<D*>(dst.data());

        for (size_t i = 0; i < gt_index.size(); ++i) {
            ASSERT_NEAR(gt[i], (T)dst_data[gt_index[i]], precision);
        }
    }
}

template<class T, class D>
static void resize_multi_thread(
        Mat& src,
        Size size,
        float fx,
        float fy,
        InterpolationType type,
        const std::vector<int>& gt_index,
        const std::vector<T>& gt,
        T precision,
        int thread_num = 4,
        int loop_cnt = 1000) {
    std::vector<std::thread> threads;

    for (int i = 0; i < thread_num; ++i) {
        threads.push_back(std::thread(resize_single_thread<T, D>, std::ref(src), size,
                fx, fy, type, std::ref(gt_index), std::ref(gt), precision, loop_cnt));
    }

    for (auto iter = threads.begin(); iter != threads.end(); ++iter) {
        iter->join();
    }
}

TEST_F(ResizeTest, GRAYU8InterLinearPositiveInput) {
    std::vector<int> groundtruth = {65, 65, 63, 219, 109, 117, 201, 202, 203};
    resize_multi_thread<int, unsigned char>(gray_u8_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_LINEAR, C1_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, PkgBGRU8InterLinearPositiveInput) {
    std::vector<int> groundtruth = {0, 86, 51, 85, 115, 122, 183, 159, 255};
    resize_multi_thread<int, unsigned char>(bgr_u8_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_LINEAR, C3_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, PkgBGRAU8InterLinearPositiveInput) {
    std::vector<int> groundtruth = {0, 86, 51, 103, 115, 122, 255, 184, 255};
    resize_multi_thread<int, unsigned char>(bgra_u8_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_LINEAR, C4_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, GRAYF32InterLinearPositiveInput) {
    std::vector<float> groundtruth = {65.25f, 65.0f, 62.75f, 218.75f,
            109.25f, 116.5f, 201.0f, 201.75f, 203.0f};
    resize_multi_thread<float, float>(gray_f32_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_LINEAR, C1_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, PkgBGRF32InterLinearPositiveInput) {
    std::vector<float> groundtruth = {0.0f, 85.5f, 51.0f, 85.25f,
            115.25f, 122.0f, 183.25f, 159.0f, 255.0f};
    resize_multi_thread<float, float>(bgr_f32_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_LINEAR, C3_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, GRAYU8InterCubicPositiveInput) {
    std::vector<int> groundtruth = {64, 64, 62, 218, 108, 118, 201, 202, 203};
    resize_multi_thread<int, unsigned char>(gray_u8_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_CUBIC, C1_640X360_IDX, groundtruth, 1);
}

TEST_F(ResizeTest, PkgBGRU8InterCubicPositiveInput) {
    std::vector<int> groundtruth = {0, 84, 50, 85, 117, 125, 183, 159, 255};
    resize_multi_thread<int, unsigned char>(bgr_u8_src, Size(640, 360), 0, 0,
            InterpolationType::INTER_CUBIC, C3_640X360_IDX, groundtruth, 1);
}