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

class ConnectedComponentsTest : public ::testing::Test {
protected:
    void SetUp() override {
        gray_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
        int status = read_binary_file(GRAY_1280X720_U8_BIN,
                gray_u8_src.data(), gray_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        unsigned char* data = reinterpret_cast<unsigned char*>(gray_u8_src.data());

        for (size_t i = 0; i < gray_u8_src.total_byte_size(); ++i) {
            data[i] = data[i] % 2;
        }
    }

    Mat gray_u8_src;
};

// to check
TEST_F(ConnectedComponentsTest, Connectivity8PositiveInput) {
    Mat dst;
    int connected_num = connected_components(gray_u8_src, dst, 8, FCVImageType::GRAY_S32);
    ASSERT_EQ(connected_num, 3022);

    int* dst_data = reinterpret_cast<int*>(dst.data());

    std::vector<int> groundtruth = {0, 0, 2, 0, 0, 2, 0, 2984, 2984};

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(groundtruth[i], dst_data[C1_1280X720_IDX[i]]);
    }
}