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

class FillPolyTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
    }

    Mat pkg_bgr_u8_src;
};

TEST_F(FillPolyTest, PkgBGRU8PositiveInput) {
    Point p1(118, 301);
    Point p2(518, 301);
    Point p3(518, 600);
    Point p4(118, 600);
    Point arr_p[1][4] = {{p1, p2, p3, p4}};
    const Point* ppt[1] = {arr_p[0]};
    int arr_n[1] = {4};
    fill_poly(pkg_bgr_u8_src, ppt, arr_n, 1, Scalar(0, 0, 255));

    unsigned char* data = (unsigned char*)pkg_bgr_u8_src.data();
    double sum = 0;

    for (size_t i = 0; i < pkg_bgr_u8_src.total_byte_size(); ++i) {
        sum += data[i];
    }

    ASSERT_NEAR(138.284793, sum / pkg_bgr_u8_src.total_byte_size(), 10e-6);
}
