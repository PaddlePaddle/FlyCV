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

class FcvNormalizeToSubmeanToReorderTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
    }

    void TearDown() override {
        release_cmat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
};

TEST_F(FcvNormalizeToSubmeanToReorderTest, PositiveInput) {
    CMat* dst = create_cmat(pkg_bgr_u8_src->width, pkg_bgr_u8_src->height, CFCVImageType::PLA_BGR_F32);
    float mean_params[3] = {127.5, 127.5, 127.5};
    float std_params[3] = {255.0, 255.0, 255.0}; 
    int channel_index[3] = {2, 0, 1};

    int status = fcvNormalizeToSubmeanToReorder(pkg_bgr_u8_src,
            mean_params, std_params, channel_index, dst, false);
    EXPECT_EQ(status, 0);

    float* dst_data = (float*)dst->data;
    std::vector<float> groundtruth = {-0.315686f, -0.307843f, -0.303922f,
            -0.186275f, -0.147059f, -0.135294f, 0.217647f, 0.221569f, 0.221569f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(dst_data[C3_1280X720_IDX[i]], groundtruth[i], 10e-6);
    }
}