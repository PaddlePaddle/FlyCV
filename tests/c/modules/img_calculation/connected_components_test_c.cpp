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

class FcvConnectedComponentsTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        unsigned char* data = reinterpret_cast<unsigned char*>(gray_u8_src->data);

        for (size_t i = 0; i < gray_u8_src->total_byte_size; ++i) {
            data[i] = data[i] % 2;
        }
    }

    void TearDown() override {
        fcvReleaseCMat(gray_u8_src);
        gray_u8_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
};

TEST_F(FcvConnectedComponentsTest, Connectivity8PositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->width, gray_u8_src->height, CFCVImageType::GRAY_S32);
    int connected_num = fcvConnectedComponents(gray_u8_src, dst, 8, CFCVImageType::GRAY_S32);
    ASSERT_EQ(connected_num, 3022);

    int* dst_data = reinterpret_cast<int*>(dst->data);

    std::vector<int> groundtruth = {0, 0, 2, 0, 0, 2, 0, 2984, 2984};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(groundtruth[i], dst_data[C1_1280X720_IDX[i]]);
    }
}