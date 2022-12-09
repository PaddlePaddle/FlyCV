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

class RemapTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p(gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p(gray_f32_src), 0);
    }

    Mat gray_u8_src;
    Mat gray_f32_src;
};

TEST_F(RemapTest, GrayU8PositiveInput) {
    Mat map_x(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);
    Mat map_y(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    // creat dst Mat
    Mat dst;
    int status = remap(gray_u8_src, dst, map_x, map_y);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = (unsigned char*)dst.data();

    std::vector<int> groundtruth = {161, 161, 161, 104, 54, 53, 94, 96, 98};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(groundtruth[i], (int)dst_data[C1_1280X720_IDX[i]]);
    }
}

TEST_F(RemapTest, GrayF32PositiveInput) {
    Mat map_x(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);
    Mat map_y(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_F32);

    for (int i = 0; i < map_x.height(); i++) {
        for (int j = 0; j < map_x.width(); j++) {
            map_x.at<float>(j, i) = static_cast<float>(j) * 0.233333;
            map_y.at<float>(j, i) = static_cast<float>(map_x.height() - i) * 0.933333;
        }
    }

    // creat dst Mat
    Mat dst;
    remap(gray_f32_src, dst, map_x, map_y);

    float* dst_data = (float*)dst.data();

    std::vector<int> index = {0, 1, 460799, 460800, 921598, 921599};
    std::vector<float> groundtruth = {161.0f, 160.78125f, 160.53125f, 103.996094f,
            54.0f, 53.0625f, 94.09375f, 96.291016f, 97.640625f};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[C1_1280X720_IDX[i]]);
    }
}