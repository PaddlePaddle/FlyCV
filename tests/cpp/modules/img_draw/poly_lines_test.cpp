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

#include "flycv.h"
#include "gtest/gtest.h"
#include "test_util.h"

using namespace g_fcv_ns;

class PolyLinesTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
    }

    Mat pkg_bgr_u8_src;
};

TEST_F(PolyLinesTest, PkgBGRU8PositiveInput) {
    Point2l pts[4] = {Point2l(118, 301), Point2l(518, 301),
            Point2l(518, 600), Point2l(118, 600)};

    unsigned char color[3] = {255, 0, 0}; 
    poly_lines(pkg_bgr_u8_src, pts, 4, true, color, 3, LineType::LINE_8, 0);

    imwrite("poly_line.jpg", pkg_bgr_u8_src);

    unsigned char* data = (unsigned char*)pkg_bgr_u8_src.data();
    double sum = 0;

    for (size_t i = 0; i < pkg_bgr_u8_src.total_byte_size(); ++i) {
        sum += data[i];
    }

    ASSERT_NEAR(143.847576, sum / pkg_bgr_u8_src.total_byte_size(), 10e-6);
}
