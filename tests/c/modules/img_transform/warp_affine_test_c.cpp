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

class FcvWarpAffineTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p_cmat(&gray_f32_src), 0);
    }

    void TearDown() override {
        release_cmat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        release_cmat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
        release_cmat(gray_u8_src);
        gray_u8_src = nullptr;
        release_cmat(gray_f32_src);
        gray_f32_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
    CMat* gray_u8_src = nullptr;
    CMat* gray_f32_src = nullptr;
};

TEST_F(FcvWarpAffineTest, GrayU8PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    CMat* matrix = create_cmat(3, 2, CFCVImageType::GRAY_F32);
    float* m_data = (float*)matrix->data;

    for (int i = 0; i < 6; ++i) {
        m_data[i] = m[i];
    }

    CMat* dst = create_cmat(gray_u8_src->width,
            gray_u8_src->height, gray_u8_src->type);
    int status = fcvWarpAffine(gray_u8_src, dst, matrix,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth_gray = {62, 58, 55, 0, 84, 148, 0, 0, 0};
    unsigned char* data = reinterpret_cast<unsigned char*>(dst->data);

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR((int)data[C1_1280X720_IDX[i]], groundtruth_gray[i], 1);
    }

    release_cmat(matrix);
    matrix = nullptr;
    release_cmat(dst);
    dst = nullptr;
}

TEST_F(FcvWarpAffineTest, PkgBGRU8PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    CMat* matrix = create_cmat(3, 2, CFCVImageType::GRAY_F32);
    float* m_data = (float*)matrix->data;

    for (int i = 0; i < 6; ++i) {
        m_data[i] = m[i];
    }

    CMat* dst = create_cmat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, pkg_bgr_u8_src->type);
    int status = fcvWarpAffine(pkg_bgr_u8_src, dst, matrix,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth_bgr = {0, 82, 47, 0, 77, 96, 0, 0, 0};
    unsigned char* data = reinterpret_cast<unsigned char*>(dst->data);

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR((int)data[C3_1280X720_IDX[i]], groundtruth_bgr[i], 1);
    }

    release_cmat(matrix);
    matrix = nullptr;
    release_cmat(dst);
    dst = nullptr;
}

TEST_F(FcvWarpAffineTest, GrayF32PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    CMat* matrix = create_cmat(3, 2, CFCVImageType::GRAY_F32);
    float* m_data = (float*)matrix->data;

    for (int i = 0; i < 6; ++i) {
        m_data[i] = m[i];
    }

    CMat* dst = create_cmat(gray_f32_src->width,
            gray_f32_src->height, gray_f32_src->type);
    int status = fcvWarpAffine(gray_f32_src, dst, matrix,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    std::vector<float> groundtruth_gray = {62.0f, 58.0f, 54.84375f,
            0.0f, 84.472656f, 147.957031f, 0.0f, 0.0f, 0.0f};

    float* gray_data = reinterpret_cast<float*>(dst->data);

    for (size_t i = 0; i < C1_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(gray_data[C1_1280X720_IDX[i]], groundtruth_gray[i], 10e-6);
    }

    release_cmat(matrix);
    matrix = nullptr;
    release_cmat(dst);
    dst = nullptr;
}

TEST_F(FcvWarpAffineTest, PkgBGRF32PositiveInput) {
    float m[6] = {0.996, -0.08, 0, 0.08, 0.996, 0};
    CMat* matrix = create_cmat(3, 2, CFCVImageType::GRAY_F32);
    float* m_data = (float*)matrix->data;

    for (int i = 0; i < 6; ++i) {
        m_data[i] = m[i];
    }

    CMat* dst = create_cmat(pkg_bgr_f32_src->width,
            pkg_bgr_f32_src->height, pkg_bgr_f32_src->type);
    int status = fcvWarpAffine(pkg_bgr_f32_src, dst, matrix,
            CInterpolationType::INTER_LINEAR, CBorderType::BORDER_CONSTANT, nullptr);
    ASSERT_EQ(status, 0);

    std::vector<int> index = {0, 3, 6, 100, 1000, 1382400, 1843200};
    std::vector<float> groundtruth_bgr = {0.0f, 82.0f, 47.0f, 0.0f, 76.675781f,
            96.390625f, 0.0f, 0.0f, 0.0f};

    float* bgr_data = reinterpret_cast<float*>(dst->data);

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_EQ(bgr_data[C3_1280X720_IDX[i]], groundtruth_bgr[i]);
    }

    release_cmat(matrix);
    matrix = nullptr;
    release_cmat(dst);
    dst = nullptr;
}
