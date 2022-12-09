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

class SplitToMemcpyTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_f32_720p(pkg_bgr_f32_src), 0);
    }

    Mat pkg_bgr_f32_src;
};

TEST_F(SplitToMemcpyTest, PositiveInput) {
    float* dst_data = new float[pkg_bgr_f32_src.total_byte_size() / pkg_bgr_f32_src.type_byte_size()];

    int status = split_to_memcpy(pkg_bgr_f32_src, dst_data);
    EXPECT_EQ(status, 0);

    std::vector<float> groundtruth = {0.0f, 0.0f, 3.0f, 90.0f,
                90.0f, 92.0f, 254.0f, 255.0f, 255.0f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(dst_data[C3_1280X720_IDX[i]], groundtruth[i], 10e-6);
    }

    delete[] dst_data;
    dst_data = nullptr;
}
