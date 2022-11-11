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
        solid_point = Point(100, 100);
        hollow = Point(300, 300);
        int status = 0;
        img_jpg = Mat(400, 400, FCVImageType::PKG_BGR_U8);
        EXPECT_EQ(status, 0);
    }

    Mat img_jpg;
    Point solid_point;
    Point hollow;
};

TEST_F(CircleTest, CirclePositiveInput) {
    Mat circle_mat;
    img_jpg.copy_to(circle_mat);
    circle(circle_mat, solid_point, 100, Scalar(255, 255, 255), -1);
    circle(circle_mat, hollow, 100, Scalar(255, 255, 255));
    unsigned char* image_data = (unsigned char*)circle_mat.data();
    double average_pixels = 0;
    for (int i = 0; i < circle_mat.width() *
            circle_mat.channels() * circle_mat.height(); i++) {
        average_pixels += image_data[i];
    }

    average_pixels /= circle_mat.width() * circle_mat.channels() * circle_mat.height();

    // 对比图像像素均值
    double groudthruth = 50.96653125;
    ASSERT_EQ((int)groudthruth, (int)average_pixels);
}
