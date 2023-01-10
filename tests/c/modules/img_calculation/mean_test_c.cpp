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

class FcvMeanTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_u16_720p_cmat(&gray_u16_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(gray_u8_src);
        gray_u8_src = nullptr;
        fcvReleaseCMat(gray_u16_src);
        gray_u16_src = nullptr;
        fcvReleaseCMat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
    CMat* gray_u16_src = nullptr;
    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
};

TEST_F(FcvMeanTest, GRAYU8PositiveInput) {
    CScalar scalar = fcvMean(gray_u8_src);
    EXPECT_NEAR(scalar.val[0], 147.357341f, 10e-6);
}

TEST_F(FcvMeanTest, GRAYU16PositiveInput) {
    CScalar scalar = fcvMean(gray_u16_src);
    EXPECT_NEAR(scalar.val[0], 147.357341f, 10e-6);
}

TEST_F(FcvMeanTest, PkgBGRU8PositiveInput) {
    CScalar scalar = fcvMean(pkg_bgr_u8_src);
    EXPECT_NEAR(scalar.val[0], 120.1931f, 10e-6);
    EXPECT_NEAR(scalar.val[1], 139.640468f, 10e-6);
    EXPECT_NEAR(scalar.val[2], 172.734920f, 10e-6);
}

TEST_F(FcvMeanTest, PkgBGRF32PositiveInput) {
    CScalar scalar = fcvMean(pkg_bgr_f32_src);
    EXPECT_NEAR(scalar.val[0], 120.1931f, 10e-6);
    EXPECT_NEAR(scalar.val[1], 139.640468f, 10e-6);
    EXPECT_NEAR(scalar.val[2], 172.734920f, 10e-6);
}

TEST(FcvMeanStddevTest, GrayU8PositiveInput) {
    // Allocate src Mat
    CMat* src = fcvCreateCMat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::GRAY_U8);
    unsigned char* src_data = reinterpret_cast<unsigned char*>(src->data);

    for (int i = 0; i < src->width * src->height; ++i) {
        src_data[i] = i % 256;
    }

    // creat dst Mat
    CMat* fcv_mean = fcvCreateCMat(1, 1, CFCVImageType::GRAY_F64);
    CMat* fcv_stddev = fcvCreateCMat(1, 1, CFCVImageType::GRAY_F64);

    fcvMeanStddev(src, fcv_mean, fcv_stddev);

    double* fcv_mean_data = reinterpret_cast<double*>(fcv_mean->data);
    double* fcv_stddev_data = reinterpret_cast<double*>(fcv_stddev->data);

    EXPECT_NEAR(fcv_mean_data[0], 127.5, 10e-6);
    EXPECT_NEAR(fcv_stddev_data[0], 73.900271, 10e-6);
}

TEST(FcvMeanStddevTest, GrayS32PositiveInput) {
    // Allocate src Mat
    CMat* src = fcvCreateCMat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::GRAY_S32);
    int* src_data = reinterpret_cast<int*>(src->data);

    for (int i = 0; i < src->width * src->height; ++i) {
        src_data[i] = i % 256;
    }

    // creat dst Mat
    CMat* fcv_mean = fcvCreateCMat(1, 1, CFCVImageType::GRAY_F64);
    CMat* fcv_stddev = fcvCreateCMat(1, 1, CFCVImageType::GRAY_F64);

    fcvMeanStddev(src, fcv_mean, fcv_stddev);

    double* fcv_mean_data = reinterpret_cast<double*>(fcv_mean->data);
    double* fcv_stddev_data = reinterpret_cast<double*>(fcv_stddev->data);

    EXPECT_NEAR(fcv_mean_data[0], 127.5, 10e-6);
    EXPECT_NEAR(fcv_stddev_data[0], 73.900271, 10e-6);

    fcvReleaseCMat(src);
    src = nullptr;
    fcvReleaseCMat(fcv_mean);
    fcv_mean = nullptr;
    fcvReleaseCMat(fcv_stddev);
    fcv_stddev = nullptr;
}