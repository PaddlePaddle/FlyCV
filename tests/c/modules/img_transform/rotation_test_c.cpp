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

class FcvRotationTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgra_u8_720p_cmat(&pkg_bgra_u8_src), 0);
        ASSERT_EQ(prepare_gray_f32_720p_cmat(&gray_f32_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p_cmat(&pkg_bgr_f32_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(gray_u8_src);
        gray_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgra_u8_src);
        pkg_bgra_u8_src = nullptr;
        fcvReleaseCMat(gray_f32_src);
        gray_f32_src = nullptr;
        fcvReleaseCMat(pkg_bgr_f32_src);
        pkg_bgr_f32_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_bgra_u8_src = nullptr;
    CMat* gray_f32_src = nullptr;
    CMat* pkg_bgr_f32_src = nullptr;
};

TEST_F(FcvRotationTest, GrayU8PositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->height,
            gray_u8_src->width, gray_u8_src->type);

    int status = fcvTranspose(gray_u8_src, dst);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = gray_u8_src->channels;

    int i = 0, j = 0, k = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (j = 0; j < IMG_720P_WIDTH; j++) {
            for (k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += dst->stride;
        }

        dst_data += sc;
        src_data += gray_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvRotationTest, PkgBgrU8PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->height,
            pkg_bgr_u8_src->width, pkg_bgr_u8_src->type);

    int status = fcvTranspose(pkg_bgr_u8_src, dst);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgr_u8_src->channels;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            for (int k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += dst->stride;
        }

        dst_data += sc;
        src_data += pkg_bgr_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvRotationTest, PkgBgraU8PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->height,
            pkg_bgra_u8_src->width, pkg_bgra_u8_src->type);

    int status = fcvTranspose(pkg_bgra_u8_src, dst);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgra_u8_src->channels;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        unsigned char* dst_col = dst_data;
        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            for (int k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += dst->stride;
        }

        dst_data += sc;
        src_data += pkg_bgra_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvRotationTest, GrayF32PositiveInput) {
    CMat* dst = fcvCreateCMat(gray_f32_src->height,
            gray_f32_src->width, gray_f32_src->type);

    int status = fcvTranspose(gray_f32_src, dst);
    ASSERT_EQ(status, 0);

    float* src_data = reinterpret_cast<float*>(gray_f32_src->data);
    float* dst_data = reinterpret_cast<float*>(dst->data);
    int sc = gray_f32_src->channels;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        float* dst_col = dst_data;
        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            for (int k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += (dst->stride / sizeof(float));
        }

        dst_data += sc;
        src_data += (gray_f32_src->stride / sizeof(float));
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvRotationTest, PkgBgrF32PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_f32_src->height,
            pkg_bgr_f32_src->width, pkg_bgr_f32_src->type);

    int status = fcvTranspose(pkg_bgr_f32_src, dst);
    ASSERT_EQ(status, 0);

    float* src_data = reinterpret_cast<float*>(pkg_bgr_f32_src->data);
    float* dst_data = reinterpret_cast<float*>(dst->data);
    int sc = pkg_bgr_f32_src->channels;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        float* dst_col = dst_data;
        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            for (int k = 0; k < sc; k++) {
                ASSERT_EQ(dst_col[k], src_data[j * sc + k]);
            }
            dst_col += (dst->stride / sizeof(float));
        }

        dst_data += sc;
        src_data += (pkg_bgr_f32_src->stride / sizeof(float));
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}