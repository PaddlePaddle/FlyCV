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

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

TEST(MinAreaRectTest, PositiveInput) {
    std::vector<Point> pts(11);
    pts[0] = {1, 1};
    pts[1] = {3, 5};
    pts[2] = {20, 100};
    pts[3] = {74, 37};
    pts[4] = {532, 77};
    pts[5] = {94, 333};
    pts[6] = {149, 633};
    pts[7] = {20, 30};
    pts[8] = {57, 22};
    pts[9] = {44, 48};
    pts[10] = {331, 232};

    RotatedRect box = min_area_rect(pts);

    ASSERT_NEAR(box.center_x(), 142.688, 10e-4);
    ASSERT_NEAR(box.center_y(), 218.737, 10e-4);
    ASSERT_NEAR(box.width(), 480.404, 10e-4);
    ASSERT_NEAR(box.height(), 675.148, 10e-4);
    ASSERT_NEAR(box.angle(), 34.561, 10e-4);
}
