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

#include "flycv.h"
#include "gtest/gtest.h"
#include "test_util.h"

    using namespace g_fcv_ns;

class ExtractChannelTest : public ::testing::Test {
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

TEST_F(ExtractChannelTest, PkgBGRU8PositiveInput) {
    Mat dst[3];
    extract_channel(pkg_bgr_u8_src, dst[0], 0);
    extract_channel(pkg_bgr_u8_src, dst[1], 1);
    extract_channel(pkg_bgr_u8_src, dst[2], 2);

    std::vector<int> index = {0, 1, 2, 921597, 921598, 921599};
    std::vector<std::vector<unsigned char>> groundtruth = {
        {0, 0, 3, 159, 159, 159},
        {82, 84, 85, 184, 184, 184},
        {47, 49, 50, 255, 255, 255}};

    for (int n = 0; n < pkg_bgr_u8_src.channels(); n++) {
        unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst[n].data());

        for (size_t i = 0; i < index.size(); ++i) {
            ASSERT_EQ(groundtruth[n][i], (int)dst_data[index[i]]);
        }
    }
}
