// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

class NormalizeToSubmeanToReorderTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8_src.data(),
                pkg_bgr_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat pkg_bgr_u8_src;
};

TEST_F(NormalizeToSubmeanToReorderTest, PositiveInput) {
    Mat dst(pkg_bgr_u8_src.width(), pkg_bgr_u8_src.height(), FCVImageType::PLA_BGR_F32);
    std::vector<float> mean_params = {127.5, 127.5, 127.5};
    std::vector<float> std_params = {255.0, 255.0, 255.0};

    int status = normalize_to_submean_to_reorder(pkg_bgr_u8_src,
            mean_params, std_params, {2, 0, 1}, dst);
    EXPECT_EQ(status, 0);

    float* dst_data = (float*)dst.data();
    std::vector<float> groundtruth = {-0.315686f, -0.307843f, -0.303922f,
            -0.186275f, -0.147059f, -0.135294f, 0.217647f, 0.221569f, 0.221569f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(dst_data[C3_1280X720_IDX[i]], groundtruth[i], 10e-6);
    }
}