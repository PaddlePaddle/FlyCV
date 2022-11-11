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

class SubtractTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_f32_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_F32);
        status = read_binary_file(BGR_1280X720_F32_BIN,
                pkg_bgr_f32_src.data(), pkg_bgr_f32_src.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat pkg_bgr_f32_src;
};

TEST_F(SubtractTest, PkgBGRU8PositiveInput) {
    Mat dst;
    int status = subtract(pkg_bgr_f32_src, {50, 100, 150}, dst);
    EXPECT_EQ(status, 0);

    float* dst_data = reinterpret_cast<float*>(dst.data());
    std::vector<int> index = {0, 1, 2, 345600, 345601, 345602, 691197, 691198, 691199};
    std::vector<float> groundtruth = {-50.0f, -18.0f, -103.0f, -16.0f, 40.0f,
            -46.0f, 105.0f, 109.0f, 105.0f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[C3_1280X720_IDX[i]]);
    }
}
