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

class CircleTest : public ::testing::Test {
   protected:
    void SetUp() override {
        pkg_bgr_u8 = Mat(400, 400, FCVImageType::PKG_BGR_U8);

        unsigned char* data = (unsigned char*)pkg_bgr_u8.data();

        for (int i = 0; i < pkg_bgr_u8.total_byte_size(); ++i) {
            data[i] = 0;
        }
    }

    Mat pkg_bgr_u8;
    Point solid_point;
    Point hollow;
};

TEST_F(CircleTest, CirclePositiveInput) {
    Point solid_point(100, 100);
    Point hollow(300, 300);
    circle(pkg_bgr_u8, solid_point, 100, Scalar(255, 255, 255), -1);
    circle(pkg_bgr_u8, hollow, 100, Scalar(255, 255, 255));
    unsigned char* image_data = (unsigned char*)pkg_bgr_u8.data();
    double average_pixels = 0;

    for (int i = 0; i < pkg_bgr_u8.width() *
            pkg_bgr_u8.channels() * pkg_bgr_u8.height(); i++) {
        average_pixels += image_data[i];
    }

    average_pixels /= pkg_bgr_u8.width() * pkg_bgr_u8.channels() * pkg_bgr_u8.height();

    // 对比图像像素均值
    double groudthruth = 50.96653125;
    ASSERT_EQ((int)groudthruth, (int)average_pixels);
}
