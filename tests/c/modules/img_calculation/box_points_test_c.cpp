// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

TEST(FcvBoxPointsTest, PositiveInput) {
    CRotatedRect rect;
    rect.center.x = 50;
    rect.center.y = 60;
    rect.size.width = 300;
    rect.size.height = 200;
    rect.angle = 50;

    CMat* points = fcvCreateCMat(2, 4, CFCVImageType::GRAY_F32);
    fcvBoxPoints(rect, points);

    std::vector<float> groundtruth = {-123.022591f, 9.372097f, 30.1863f,
            -119.185432f, 223.022583f, 110.627899f, 69.813698f, 239.185425};
    float* pdata = reinterpret_cast<float*>(points->data);

    for (size_t i = 0; i < groundtruth.size(); ++i) {
        ASSERT_NEAR(pdata[i], groundtruth[i], 10e-5);
    }

    fcvReleaseCMat(points);
    points = nullptr;
}