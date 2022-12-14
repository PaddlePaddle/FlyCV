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

class FcvRemapTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p_cmat(&gray_f32_src), 0);
    }

    void TearDown() override {
        release_cmat(gray_u8_src);
        gray_u8_src = nullptr;
        release_cmat(gray_f32_src);
        gray_f32_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
    CMat* gray_f32_src = nullptr;
};

TEST_F(FcvRemapTest, GrayU8PositiveInput) {
    CMat* map_x = create_cmat(IMG_720P_WIDTH,
            IMG_720P_HEIGHT, CFCVImageType::GRAY_F32);
    CMat* map_y = create_cmat(IMG_720P_WIDTH,
            IMG_720P_HEIGHT, CFCVImageType::GRAY_F32);

    float* mx_data = (float*)map_x->data;
    float* my_data = (float*)map_y->data;

    for (int i = 0; i < map_x->height; i++) {
        for (int j = 0; j < map_x->width; j++) {
            mx_data[i * map_x->width + j] = static_cast<float>(j) * 0.233333;
            my_data[i * map_y->width + j] = ((float)(map_y->height - i)) * 0.933333;
        }
    }

    // creat dst Mat
    CMat* dst = create_cmat(map_x->width,
            map_x->height, gray_u8_src->type);

    int status = fcvRemap(gray_u8_src, dst, map_x, map_y, 
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = (unsigned char*)dst->data;
    std::vector<int> groundtruth = {161, 161, 161, 104, 54, 53, 94, 96, 98};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(groundtruth[i], (int)dst_data[C1_1280X720_IDX[i]]);
    }

    release_cmat(dst);
    dst = nullptr;
    release_cmat(map_x);
    map_x = nullptr;
    release_cmat(map_y);
    map_y = nullptr;
}

TEST_F(FcvRemapTest, GrayF32PositiveInput) {
    CMat* map_x = create_cmat(IMG_720P_WIDTH,
            IMG_720P_HEIGHT, CFCVImageType::GRAY_F32);
    CMat* map_y = create_cmat(IMG_720P_WIDTH,
            IMG_720P_HEIGHT, CFCVImageType::GRAY_F32);

    float* mx_data = (float*)map_x->data;
    float* my_data = (float*)map_y->data;

    for (int i = 0; i < map_x->height; i++) {
        for (int j = 0; j < map_x->width; j++) {
            mx_data[i * map_x->width + j] = j * 0.233333f;
            my_data[i * map_y->width + j] = (map_y->height - i) * 0.933333f;
        }
    }

    // creat dst Mat
    CMat* dst = create_cmat(map_x->width,
            map_x->height, gray_f32_src->type);

    int status = fcvRemap(gray_f32_src, dst, map_x, map_y, 
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    float* dst_data = (float*)dst->data;
    std::vector<float> groundtruth = {161.0f, 160.78125f, 160.53125f, 103.996094f,
            54.0f, 53.0625f, 94.09375f, 96.291016f, 97.640625f};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[C1_1280X720_IDX[i]]);
    }

    release_cmat(dst);
    dst = nullptr;
    release_cmat(map_x);
    map_x = nullptr;
    release_cmat(map_y);
    map_y = nullptr;
}