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

// to check
TEST(FcvFindHomographyTest, PositiveInput) {
    // init src point an dst point
    std::vector<float> s_vec = {22, 1, -111.1, 222.1, 0, -222.1, 161.1, 332.1};
    CMat* src_points = create_cmat(2, 4, CFCVImageType::GRAY_F32);
    memcpy(src_points->data, s_vec.data(), src_points->total_byte_size);

    std::vector<float> d_vec = {1.11, 1, 479, 1, 475, 1, 1, 639};
    CMat* dst_points = create_cmat(2, 4, CFCVImageType::GRAY_F32);
    memcpy(dst_points->data, d_vec.data(), dst_points->total_byte_size);

    // creat Mat
    CMat* dst = fcvFindHomography(src_points, dst_points, 0);
    double* dst_data = reinterpret_cast<double*>(dst->data);
    std::vector<double> groundtruth = {-22.540573f, 9.469605f, 486.428063f,
            -0.047052f, 0.019843f, 0.993992f, -0.047081f, 0.019828f, 1.f};

    for (int i = 0; i < dst->width * dst->height; ++i) {
        ASSERT_NEAR(groundtruth[i], dst_data[i], 10e-6);
    }

    release_cmat(dst);
    dst = nullptr;
    release_cmat(src_points);
    src_points = nullptr;
    release_cmat(dst_points);
    dst_points = nullptr;
}