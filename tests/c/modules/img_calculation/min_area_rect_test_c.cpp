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

TEST(FcvMinAreaRectTest, PositiveInput) {
    std::vector<int> points = {
            1, 1, 3, 5, 20, 100, 74, 37, 532, 77,
            94, 333, 149, 633, 20, 30, 57, 22, 44, 48, 331, 232};

    CMat* src = create_cmat(2, 11, CFCVImageType::GRAY_S32);
    memcpy(src->data, points.data(), src->total_byte_size);

    CRotatedRect box = fcvMinAreaRect(src);

    ASSERT_NEAR(box.center.x, 142.688, 10e-4);
    ASSERT_NEAR(box.center.y, 218.737, 10e-4);
    ASSERT_NEAR(box.size.width, 480.404, 10e-4);
    ASSERT_NEAR(box.size.height, 675.148, 10e-4);
    ASSERT_NEAR(box.angle, 34.561, 10e-4);
}