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

using namespace g_fcv_ns;

class FcvMinMaxLocTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
    }

    void TearDown() override {
        release_cmat(gray_u8_src);
        gray_u8_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
};

TEST_F(FcvMinMaxLocTest, PositiveInput) {
    double min_val = -1;
    double max_val = -1;
    CPoint min_loc;
    CPoint max_loc;

    int status = fcvMinMaxLoc(gray_u8_src, &min_val, &max_val, &min_loc, &max_loc, nullptr);
    ASSERT_EQ(status, 0);

    EXPECT_NEAR(min_val, 11, 10e-6);
    EXPECT_NEAR(max_val, 255, 10e-6);
    EXPECT_EQ(min_loc.x, 26);
    EXPECT_EQ(min_loc.y, 266);
    EXPECT_EQ(max_loc.x, 655);
    EXPECT_EQ(max_loc.y, 1);
}