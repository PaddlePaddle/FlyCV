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
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace g_fcv_ns;

class AddWeightedTest : public ::testing::Test {
public:
    void SetUp() override {
        int status = 0;
        src1 = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, src1.data(),
                src1.total_byte_size());
        EXPECT_EQ(status, 0);

        src2 = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_2_BIN, src2.data(), 
                src2.total_byte_size());
        EXPECT_EQ(status, 0);
    }

public:
    Mat src1;
    Mat src2;
};

TEST_F(AddWeightedTest, AddWeightedCommonPositiveInput) {
    double alpha = 0.5;
    double beta = 0.5;
    double gama = 20;
    
    Mat add_weighted_dst;
    add_weighted(src1, alpha, src2, beta, gama, add_weighted_dst);
}