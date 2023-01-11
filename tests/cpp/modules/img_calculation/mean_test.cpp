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

class MeanTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_gray_u8_720p(gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_u16_720p(gray_u16_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p(pkg_bgr_f32_src), 0);
    }

    Mat gray_u8_src;
    Mat gray_u16_src;
    Mat pkg_bgr_u8_src;
    Mat pkg_bgr_f32_src;
};

TEST_F(MeanTest, GRAYU8PositiveInput) {
    Scalar scalar;
    scalar = mean(gray_u8_src);
    EXPECT_NEAR(scalar[0], 147.357341f, 10e-6);
}

TEST_F(MeanTest, GRAYU16PositiveInput) {
    Scalar scalar;
    scalar = mean(gray_u16_src);
    EXPECT_NEAR(scalar[0], 147.357341f, 10e-6);
}

TEST_F(MeanTest, PkgBGRU8PositiveInput) {
    Scalar scalar;
    scalar = mean(pkg_bgr_u8_src);
    EXPECT_NEAR(scalar[0], 120.1931f, 10e-6);
    EXPECT_NEAR(scalar[1], 139.640468f, 10e-6);
    EXPECT_NEAR(scalar[2], 172.734920f, 10e-6);
}

TEST_F(MeanTest, PkgBGRF32PositiveInput) {
    Scalar scalar;
    scalar = mean(pkg_bgr_f32_src);
    EXPECT_NEAR(scalar[0], 120.1931f, 10e-6);
    EXPECT_NEAR(scalar[1], 139.640468f, 10e-6);
    EXPECT_NEAR(scalar[2], 172.734920f, 10e-6);
}

// to check
TEST_F(MeanTest, GRAYU8WithMaskPositiveInput) {
    Scalar scalar;
    Mat empty_mask;
    scalar = mean(gray_u8_src, empty_mask);
    EXPECT_NEAR(scalar[0], 147.357341f, 10e-6);

    Mat mask(gray_u8_src.width(), gray_u8_src.height(), gray_u8_src.type());
    unsigned char* mask_data = reinterpret_cast<unsigned char*>(mask.data());
    for (int i = 0; i < mask.width() * mask.height(); ++i) {
        mask_data[i] = i % 1000 == 0 ? 1 : 0;
    }

    scalar = mean(gray_u8_src, mask);
    EXPECT_NEAR(scalar[0], 145.802603f, 10e-6);
}

// to check
TEST_F(MeanTest, GRAYU16WithMaskPositiveInput) {
    Scalar scalar;

    Mat mask(gray_u16_src.width(), gray_u16_src.height(), FCVImageType::GRAY_U8);
    unsigned char* mask_data = reinterpret_cast<unsigned char*>(mask.data());
    for (int i = 0; i < mask.width() * mask.height(); ++i) {
        mask_data[i] = i % 100 == 0 ? 1 : 0;
    }

    scalar = mean(gray_u16_src, mask);
    EXPECT_NEAR(scalar[0], 146.847981f, 10e-6);
}

// to check
TEST_F(MeanTest, PkgBGRU8WithMaskPositiveInput) {
    Scalar scalar;

    Mat mask(pkg_bgr_u8_src.width(), pkg_bgr_u8_src.height(), FCVImageType::GRAY_U8);
    unsigned char* mask_data = reinterpret_cast<unsigned char*>(mask.data());
    for (int i = 0; i < mask.width() * mask.height(); ++i) {
        mask_data[i] = i % 100 == 0 ? 1 : 0;
    }

    scalar = mean(pkg_bgr_u8_src, mask);
    EXPECT_NEAR(scalar[0], 119.527235f, 10e-6);
    EXPECT_NEAR(scalar[1], 139.225151f, 10e-6);
    EXPECT_NEAR(scalar[2], 172.099934f, 10e-6);
}

// to check
TEST_F(MeanTest, PkgBGRF32WithMaskPositiveInput) {
    Scalar scalar;

    Mat mask(pkg_bgr_f32_src.width(), pkg_bgr_f32_src.height(), FCVImageType::GRAY_U8);
    unsigned char* mask_data = reinterpret_cast<unsigned char*>(mask.data());
    for (int i = 0; i < mask.width() * mask.height(); ++i) {
        mask_data[i] = i % 10 == 0 ? 1 : 0;
    }

    scalar = mean(pkg_bgr_f32_src, mask);
    EXPECT_NEAR(scalar[0], 120.130468f, 10e-6);
    EXPECT_NEAR(scalar[1], 139.612022f, 10e-6);
    EXPECT_NEAR(scalar[2], 172.662955f, 10e-6);
}

// to check
TEST_F(MeanTest, GRAYU8WithRectPositiveInput) {
    Scalar scalar;
    Rect rect(0, 0, gray_u8_src.width() / 2, gray_u8_src.height() / 2);
    scalar = mean(gray_u8_src, rect);
    EXPECT_NEAR(scalar[0], 117.736445f, 10e-6);
}

// to check
TEST_F(MeanTest, GRAYU16WithRectPositiveInput) {
    Scalar scalar;
    Rect rect(gray_u16_src.width() / 4, gray_u16_src.height() / 4,
            gray_u16_src.width() / 2, gray_u16_src.height() / 2);
    scalar = mean(gray_u16_src, rect);
    EXPECT_NEAR(scalar[0], 133.387278f, 10e-6);
}

// to check
TEST_F(MeanTest, PkgBGRU8WithRectPositiveInput) {
    Scalar scalar;
    Rect rect(pkg_bgr_u8_src.width() / 4, pkg_bgr_u8_src.height() / 4,
            pkg_bgr_u8_src.width() / 2, pkg_bgr_u8_src.height() / 2);
    scalar = mean(pkg_bgr_u8_src, rect);
    EXPECT_NEAR(scalar[0], 116.569513f, 10e-6);
}

TEST_F(MeanTest, PkgBGRF32WithRectPositiveInput) {
    Scalar scalar;
    Rect rect(pkg_bgr_f32_src.width() / 4, pkg_bgr_f32_src.height() / 4,
            pkg_bgr_f32_src.width() / 2, pkg_bgr_f32_src.height() / 2);
    scalar = mean(pkg_bgr_f32_src, rect);
    EXPECT_NEAR(scalar[0], 116.569513f, 10e-6);
}

TEST(MeanStddevTest, GrayU8PositiveInput2) {
    // Allocate src Mat
    Mat src(300, 300, FCVImageType::GRAY_U8);
    unsigned char* src_data = reinterpret_cast<unsigned char*>(src.data());

    for (int i = 0; i < src.width() * src.height(); ++i) {
        src_data[i] = i % 256;
    }

    // creat dst Mat
    Mat fcv_mean;
    Mat fcv_stddev;

    mean_stddev(src, fcv_mean, fcv_stddev);

    double* fcv_mean_data = reinterpret_cast<double*>(fcv_mean.data());
    double* fcv_stddev_data = reinterpret_cast<double*>(fcv_stddev.data());

    EXPECT_NEAR(fcv_mean_data[0], 127.41, 10e-3);
    EXPECT_NEAR(fcv_stddev_data[0], 73.89, 10e-3);
}

TEST(MeanStddevTest, GrayU8PositiveInput) {
    // Allocate src Mat
    Mat src(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
    unsigned char* src_data = reinterpret_cast<unsigned char*>(src.data());

    for (int i = 0; i < src.width() * src.height(); ++i) {
        src_data[i] = i % 256;
    }

    // creat dst Mat
    Mat fcv_mean;
    Mat fcv_stddev;

    mean_stddev(src, fcv_mean, fcv_stddev);

    double* fcv_mean_data = reinterpret_cast<double*>(fcv_mean.data());
    double* fcv_stddev_data = reinterpret_cast<double*>(fcv_stddev.data());

    EXPECT_NEAR(fcv_mean_data[0], 127.5, 10e-6);
    EXPECT_NEAR(fcv_stddev_data[0], 73.900271, 10e-6);
}

TEST(MeanStddevTest, GrayS32PositiveInput) {
    // Allocate src Mat
    Mat src(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_S32);
    int* src_data = reinterpret_cast<int*>(src.data());

    for (int i = 0; i < src.width() * src.height(); ++i) {
        src_data[i] = i % 256;
    }

    // creat dst Mat
    Mat fcv_mean;
    Mat fcv_stddev;

    mean_stddev(src, fcv_mean, fcv_stddev);

    double* fcv_mean_data = reinterpret_cast<double*>(fcv_mean.data());
    double* fcv_stddev_data = reinterpret_cast<double*>(fcv_stddev.data());

    EXPECT_NEAR(fcv_mean_data[0], 127.5, 10e-6);
    EXPECT_NEAR(fcv_stddev_data[0], 73.900271, 10e-6);
}