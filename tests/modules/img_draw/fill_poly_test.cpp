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
        rect = RectI(118, 301, 400, 588);
        int status = 0;
        img_jpg = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, img_jpg.data(),
                img_jpg.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat img_jpg;
    RectI rect;
};

TEST_F(FillPolyTest, PkgBGRU8PositiveInput) {
    Mat poly_mat;
    img_jpg.copy_to(poly_mat);
    Point p1(rect.x(), rect.y());
    Point p2(rect.x() + rect.width(), rect.y());
    Point p3(rect.x() + rect.width(),
            rect.y() + rect.height());
    Point p4(rect.x(), rect.y() + rect.height());
    Point arr_p[1][4] = {{p1, p2, p3, p4}};
    const Point* ppt[1] = {arr_p[0]};
    int arr_n[1] = {4};
    fill_poly(poly_mat, ppt, arr_n, 1, Scalar(0, 0, 255));

#ifdef WITH_LIB_JPEG_TURBO
    imwrite("test_poly.jpg", poly_mat);
#endif
}
