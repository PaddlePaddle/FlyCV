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

#include <fstream>

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

TEST(BGRAToResizeToBGR, PositiveInput) {
    int status = 0;
    Mat pkg_bgra_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGRA_U8);
    status = read_binary_file(BGRA_1280X720_U8_BIN, pkg_bgra_u8_src.data(),
            pkg_bgra_u8_src.total_byte_size());
    ASSERT_EQ(status, 0);

    Size dst_size(640, 360);
    Mat dst;
    bgra_to_resize_to_bgr(pkg_bgra_u8_src, dst, dst_size, InterpolationType::INTER_NEAREST);

    std::vector<unsigned char> groundtruth = {0, 89, 54, 84, 144, 143, 184, 159, 255};
    unsigned char* dst_data = (unsigned char*)dst.data();

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_EQ((int)dst_data[C3_640X360_IDX[i]], groundtruth[i]);
    }
}
