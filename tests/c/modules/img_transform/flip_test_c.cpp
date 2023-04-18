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

class FcvFlipTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&bgr_u8_src), 0);
        ASSERT_EQ(prepare_nv12_720p_cmat(&nv12_src), 0);
        ASSERT_EQ(prepare_nv21_720p_cmat(&nv21_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(gray_u8_src);
        gray_u8_src = nullptr;
        fcvReleaseCMat(bgr_u8_src);
        bgr_u8_src = nullptr;
        fcvReleaseCMat(nv12_src);
        nv12_src = nullptr;
        fcvReleaseCMat(nv21_src);
        nv21_src = nullptr;
    }

    CMat* gray_u8_src = nullptr;
    CMat* bgr_u8_src = nullptr;
    CMat* nv12_src = nullptr;
    CMat* nv21_src = nullptr;
};

TEST_F(FcvFlipTest, GrayU8FlipXPositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->width,
            gray_u8_src->height, gray_u8_src->type);
    int status = fcvFlip(gray_u8_src, dst, CFlipType::X);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)gray_u8_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    const unsigned char* ptr_src = src_data;
    unsigned char* ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * dst->stride;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += gray_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, PkgBgrU8FlipXPositiveInput) {
    CMat* dst = fcvCreateCMat(bgr_u8_src->width,
            bgr_u8_src->height, bgr_u8_src->type);
    int status = fcvFlip(bgr_u8_src, dst, CFlipType::X);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)bgr_u8_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    const unsigned char* ptr_src = src_data;
    unsigned char* ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * dst->stride;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += bgr_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, NV12FlipXPositiveInput) {
    CMat* dst = fcvCreateCMat(nv12_src->width, nv12_src->height, nv12_src->type);
    int status = fcvFlip(nv12_src, dst, CFlipType::X);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)nv12_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    const unsigned char* ptr_src = src_data;
    unsigned char* ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * dst->stride;

    int i = 0, j = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += nv12_src->stride;
    }
    
    int UV_720P_HEIGHT = IMG_720P_HEIGHT >> 1;
    int UV_720P_WIDTH = IMG_720P_WIDTH >> 1;
    ptr_src = src_data + IMG_720P_HEIGHT * IMG_720P_WIDTH;
    ptr_dst = dst_data + IMG_720P_HEIGHT * IMG_720P_WIDTH
            + (UV_720P_HEIGHT - 1) * dst->stride;

    for (int i = 0; i < UV_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (j = 0; j < UV_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += nv12_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, NV21FlipXPositiveInput) {
    CMat* dst = fcvCreateCMat(nv21_src->width, nv21_src->height, nv21_src->type);
    int status = fcvFlip(nv21_src, dst, CFlipType::X);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)nv21_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    unsigned char* ptr_src = src_data;
    unsigned char* ptr_dst = dst_data + (IMG_720P_HEIGHT - 1) * dst->stride;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += nv21_src->stride;
    }

    int UV_720P_HEIGHT = IMG_720P_HEIGHT >> 1;
    int UV_720P_WIDTH = IMG_720P_WIDTH >> 1;

    ptr_src = src_data + IMG_720P_HEIGHT * IMG_720P_WIDTH;
    ptr_dst = dst_data + IMG_720P_HEIGHT * IMG_720P_WIDTH
            + (UV_720P_HEIGHT - 1) * dst->stride;

    for (int i = 0; i < UV_720P_HEIGHT; i++) {
        const unsigned char* src_row = ptr_src;
        unsigned char* dst_row = ptr_dst;

        for (int j = 0; j < UV_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[j], src_row[j]);
        }

        ptr_dst -= dst->stride;
        ptr_src += nv21_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, GrayU8FlipYPositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->width,
            gray_u8_src->height, gray_u8_src->type);
    int status = fcvFlip(gray_u8_src, dst, CFlipType::Y);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)gray_u8_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    int i = 0, j = 0;
    for (; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 1;

        for (j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row--), *(src_row++));
        }

        dst_data += dst->stride;
        src_data += gray_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, PkgBgrU8FlipYPositiveInput) {
    CMat* dst = fcvCreateCMat(bgr_u8_src->width,
            bgr_u8_src->height, bgr_u8_src->type);
    int status = fcvFlip(bgr_u8_src, dst, CFlipType::Y);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)bgr_u8_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 3;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row++), *(src_row++));
            ASSERT_EQ(*(dst_row++), *(src_row++));
            ASSERT_EQ(*(dst_row++), *(src_row++));
            dst_row -= 6;
        }

        dst_data += dst->stride;
        src_data += bgr_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, NV12FlipYPositiveInput) {
    CMat* dst = fcvCreateCMat(nv12_src->width, nv12_src->height, nv12_src->type);
    int status = fcvFlip(nv12_src, dst, CFlipType::Y);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)nv12_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 1;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row--), *(src_row++));
        }

        src_data += dst->stride;
        dst_data += nv12_src->stride;
    }
    
    int UV_720P_HEIGHT = IMG_720P_HEIGHT >> 1;
    int UV_720P_WIDTH = IMG_720P_WIDTH >> 1;

    for (int i = 0; i < UV_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 2;

        for (int j = 0; j < UV_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[0], src_row[0]);
            ASSERT_EQ(dst_row[1], src_row[1]);
            dst_row -= 2;
            src_row += 2;
        }

        dst_data += dst->stride;
        src_data += nv12_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvFlipTest, NV21FlipYPositiveInput) {
    CMat* dst = fcvCreateCMat(nv21_src->width, nv21_src->height, nv21_src->type);
    int status = fcvFlip(nv21_src, dst, CFlipType::Y);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = (unsigned char*)nv21_src->data;
    unsigned char* dst_data = (unsigned char*)dst->data;

    for (int i = 0; i < IMG_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 1;

        for (int j = 0; j < IMG_720P_WIDTH; j++) {
            ASSERT_EQ(*(dst_row--), *(src_row++));
        }

        src_data += dst->stride;
        dst_data += nv21_src->stride;
    }
    
    int UV_720P_HEIGHT = IMG_720P_HEIGHT >> 1;
    int UV_720P_WIDTH = IMG_720P_WIDTH >> 1;

    for (int i = 0; i < UV_720P_HEIGHT; i++) {
        const unsigned char* src_row = src_data;
        unsigned char* dst_row = dst_data + dst->stride - 2;

        for (int j = 0; j < UV_720P_WIDTH; j++) {
            ASSERT_EQ(dst_row[0], src_row[0]);
            ASSERT_EQ(dst_row[1], src_row[1]);
            dst_row -= 2;
            src_row += 2;
        }

        dst_data += dst->stride;
        src_data += nv12_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}