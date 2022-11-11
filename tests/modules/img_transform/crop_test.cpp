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

class CropTest : public ::testing::Test {
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

TEST_F(CropTest, PkgBGRU8PositiveInput) {
    // Set rect of src for crop
    Rect area(int(IMG_720P_WIDTH / 4), int(IMG_720P_HEIGHT / 4),
            int(IMG_720P_WIDTH / 2), int(IMG_720P_HEIGHT / 2));

    Mat dst;
    crop(pkg_bgr_u8_src, dst, area);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());
    std::vector<int> index = {0, 1, 2, 345600, 345601, 345602, 691197, 691198, 691199};
    std::vector<int> groundtruth = {184, 235, 255, 139, 133, 126, 135, 143, 172};

    for (size_t i = 0; i < index.size(); ++i) {
        ASSERT_EQ(groundtruth[i], (int)dst_data[index[i]]);
    }
}
