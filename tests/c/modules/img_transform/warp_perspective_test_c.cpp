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

class FcvWarpPerspectiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
};

TEST_F(FcvWarpPerspectiveTest, GetPerspectiveTransformPositiveInput) {
    CPoint2f src_pts[4] = {{165, 270}, {360, 125}, {615, 125}, {835, 270}};
    CPoint2f dst_pts[4] = {{165, 270}, {165, 30}, {835, 30}, {835, 270}};

    CMat* trans_m = fcvGetPerspectiveTransform(src_pts, dst_pts);
    double* data = (double*)trans_m->data;
    std::vector<double> groundtruth = {-6.520134, -13.364094,
            3608.305369, 0, -11.627517, 1378.993289, 0, -0.027852, 1};

    for (int i = 0; i < 9; ++i) {
        ASSERT_NEAR(groundtruth[i], data[i], 10e-6);
    }

    fcvReleaseCMat(trans_m);
    trans_m = nullptr;
}

TEST_F(FcvWarpPerspectiveTest, PkgBGRU8PositiveInput) {
    CPoint2f src_pts[4] = {{165, 270}, {360, 125}, {615, 125}, {835, 270}};
    CPoint2f dst_pts[4] = {{160, 260}, {360, 125}, {615, 125}, {835, 270}};
    CMat* trans_m = fcvGetPerspectiveTransform(src_pts, dst_pts);

    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, pkg_bgr_u8_src->type);
    int status = fcvWarpPerspective(pkg_bgr_u8_src, dst, trans_m,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    unsigned char* data = (unsigned char*)dst->data;
    std::vector<int> groundtruth = {0, 0, 0, 124, 75, 95, 154, 152, 255};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(groundtruth[i], (int)data[C3_1280X720_IDX[i]]);
    }

    fcvReleaseCMat(trans_m);
    trans_m = nullptr;
    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvWarpPerspectiveTest, PkgBGRF32PositiveInput) {
    CPoint2f src_pts[4] = {{165, 270}, {360, 125}, {615, 125}, {835, 270}};
    CPoint2f dst_pts[4] = {{160, 260}, {360, 125}, {615, 125}, {835, 270}};
    CMat* trans_m = fcvGetPerspectiveTransform(src_pts, dst_pts);

    CMat* dst = fcvCreateCMat(pkg_bgr_f32_src->width,
            pkg_bgr_f32_src->height, pkg_bgr_f32_src->type);
    int status = fcvWarpPerspective(pkg_bgr_f32_src, dst, trans_m,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    float* data = (float*)dst->data;

    std::vector<float> groundtruth = {0.0f, 0.0f, 0.0f, 123.568359f,
            75.117188f, 95.162109f, 153.75f, 151.886719f, 254.718750f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], data[C3_1280X720_IDX[i]]);
    }

    fcvReleaseCMat(trans_m);
    trans_m = nullptr;
    fcvReleaseCMat(dst);
    dst = nullptr;
}