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

class BgrToRgbaWithMaskTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
    }

    Mat pkg_bgr_u8_src;
};

TEST_F(BgrToRgbaWithMaskTest, PositiveInput) {
    Mat mask(pkg_bgr_u8_src.width(), pkg_bgr_u8_src.height(), FCVImageType::GRAY_U8);
    unsigned char* mask_ptr = (unsigned char*)mask.data();

    for (size_t i = 0; i < mask.total_byte_size(); ++i) {
        mask_ptr[i] = i % 256;
    }

    Mat dst;
    int status = bgr_to_rgba_with_mask(pkg_bgr_u8_src, mask, dst);
    EXPECT_EQ(status, 0);

    std::vector<int> groundtruth = {47, 82, 0, 74, 104, 90, 254, 184, 255};
    unsigned char* dst_ptr = (unsigned char*)dst.data();

    for (size_t i = 0; i < C4_1280X720_IDX.size(); ++i) {
        ASSERT_EQ((int)dst_ptr[C4_1280X720_IDX[i]], groundtruth[i]);
    }
}
