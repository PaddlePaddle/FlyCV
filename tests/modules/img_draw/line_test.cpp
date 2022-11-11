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

class LineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_u8 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8.data(),
                pkg_bgr_u8.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat pkg_bgr_u8;
};

TEST_F(LineTest, PkgBGRU8PositiveInput) {
    Mat line_mat;
    pkg_bgr_u8.copy_to(line_mat);
    Point p1(30, 30);
    Point p2(700, 700);
    line(line_mat, p1, p2, Scalar(255, 0, 0), 2);

#ifdef WITH_LIB_JPEG_TURBO
    imwrite("test_line.jpg", line_mat);
#endif
}