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

class FcvColorConvertTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_nv21_720p_cmat(&nv21_src), 0);
        ASSERT_EQ(prepare_i420_720p_cmat(&i420_src), 0);
        ASSERT_EQ(prepare_gray_u8_720p_cmat(&gray_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_rgb_u8_720p_cmat(&pkg_rgb_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgra_u8_720p_cmat(&pkg_bgra_u8_src), 0);
    }

    void TearDown() override {
        fcvReleaseCMat(i420_src);
        i420_src = nullptr;
        fcvReleaseCMat(nv21_src);
        nv21_src = nullptr;
        fcvReleaseCMat(gray_u8_src);
        gray_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
        fcvReleaseCMat(pkg_rgb_u8_src);
        pkg_rgb_u8_src = nullptr;
        fcvReleaseCMat(pkg_bgra_u8_src);
        pkg_bgra_u8_src = nullptr;
    }

    CMat* i420_src = nullptr;
    CMat* nv21_src = nullptr;
    CMat* gray_u8_src = nullptr;
    CMat* pkg_bgr_u8_src = nullptr;
    CMat* pkg_rgb_u8_src = nullptr;
    CMat* pkg_bgra_u8_src = nullptr;
};

TEST_F(FcvColorConvertTest, PkgBGRA2NV12PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->width,
            pkg_bgra_u8_src->height, CFCVImageType::NV21);

    int status = fcvCvtColor(pkg_bgra_u8_src, dst, CColorConvertType::CVT_PA_BGRA2NV21);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 105, 106, 107};

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgRGBA2NV12PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->width,
            pkg_bgra_u8_src->height, CFCVImageType::NV12);
    int status = fcvCvtColor(pkg_bgra_u8_src, dst, CColorConvertType::CVT_PA_RGBA2NV12);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth = {62, 64, 65, 174, 140, 138, 111, 112, 112};

    unsigned char* dst_data = (unsigned char*)dst->data;

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGR2NV12PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::NV12);

    int status = fcvCvtColor(pkg_bgr_u8_src, dst, CColorConvertType::CVT_PA_BGR2NV12);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 161, 161, 161};
    unsigned char* dst_data = (unsigned char*)dst->data;

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGR2NV21PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::NV21);

    int status = fcvCvtColor(pkg_bgr_u8_src, dst, CColorConvertType::CVT_PA_BGR2NV21);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = (unsigned char*)dst->data;
    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 105, 106, 107};

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, NV212PkgBGRPositiveInput) {
    CMat* dst = fcvCreateCMat(nv21_src->width,
            nv21_src->height, CFCVImageType::PKG_BGR_U8);

    int status = fcvCvtColor(nv21_src, dst, CColorConvertType::CVT_NV212PA_BGR);
    ASSERT_EQ(status, 0);

    std::vector<int> groundtruth = {0, 82, 49, 85, 88, 99, 155, 160, 255};
    unsigned char* dst_data = (unsigned char*)dst->data;

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
       ASSERT_NEAR((int)dst_data[C3_1280X720_IDX[i]], groundtruth[i], 1);
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGR2GRAYPositiveInput) {
    CMat* gray_u8_dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::GRAY_U8);

    int status = fcvCvtColor(pkg_bgr_u8_src, gray_u8_dst, CColorConvertType::CVT_PA_BGR2GRAY);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(gray_u8_dst->data);
    int sc = pkg_bgr_u8_src->channels;

    int i = 0, j = 0;
    for (; i < pkg_bgr_u8_src->height; i++) {
        for (j = 0; j < pkg_bgr_u8_src->width; j++) {
            int temp = static_cast<unsigned short>(src_data[j * sc] * 29 +
                    src_data[j * sc + 1] * 150 + src_data[j * sc + 2] * 77);
            ASSERT_EQ(dst_data[j], (unsigned char)((temp + (1 << 7)) >> 8));
        }

        dst_data += gray_u8_dst->stride;
        src_data += pkg_bgr_u8_src->stride;
    }

    fcvReleaseCMat(gray_u8_dst);
    gray_u8_dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgRGB2GRAYPositiveInput) {
    CMat* gray_u8_dst = fcvCreateCMat(pkg_rgb_u8_src->width,
            pkg_rgb_u8_src->height, CFCVImageType::GRAY_U8);

    int status = fcvCvtColor(pkg_rgb_u8_src, gray_u8_dst, CColorConvertType::CVT_PA_RGB2GRAY);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_rgb_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(gray_u8_dst->data);
    int sc = pkg_bgr_u8_src->channels;

    int i = 0, j = 0;
    for (; i < pkg_rgb_u8_src->height; i++) {
        for (j = 0; j < pkg_rgb_u8_src->width; j++) {
            int temp = static_cast<unsigned short>(src_data[j * sc] * 77 +
                    src_data[j * sc + 1] * 150 + src_data[j * sc + 2] * 29);
            ASSERT_EQ(dst_data[j], (unsigned char)((temp + (1 << 7)) >> 8));
        }

        dst_data += gray_u8_dst->stride;
        src_data += pkg_rgb_u8_src->stride;
    }

    fcvReleaseCMat(gray_u8_dst);
    gray_u8_dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgRGBA2PkgRGBPositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->width,
            pkg_bgra_u8_src->height, CFCVImageType::PKG_RGB_U8);

    int status = fcvCvtColor(pkg_bgra_u8_src, dst, CColorConvertType::CVT_PA_RGBA2PA_RGB);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgra_u8_src->channels;
    int dc = dst->channels;

    int i = 0, j = 0, k = 0;
    for (; i < dst->height; i++) {
        for (j = 0; j < dst->width; j++) {
            for (k = 0; k < dc; k++) {
                ASSERT_EQ(dst_data[j * dc + k], src_data[j * sc + k]);
            }
        }

        dst_data += dst->stride;
        src_data += pkg_bgra_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGR2PkgRGBAPositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::PKG_RGBA_U8);

    int status = fcvCvtColor(pkg_bgr_u8_src, dst, CColorConvertType::CVT_PA_BGR2PA_RGBA);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgr_u8_src->channels;
    int dc = dst->channels;

    int i = 0, j = 0;
    for (; i < dst->height; i++) {
        for (j = 0; j < dst->width; j++) {
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += dst->stride;
        src_data += pkg_bgr_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGR2PkgRGBPositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::PKG_RGB_U8);

    int status = fcvCvtColor(pkg_bgr_u8_src, dst, CColorConvertType::CVT_PA_BGR2PA_RGB);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgr_u8_src->channels;
    int dc = dst->channels;

    int i = 0, j = 0;
    for (; i < dst->height; i++) {
        for (j = 0; j < dst->width; j++) {
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += dst->stride;
        src_data += pkg_bgr_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgBGRA2PkgRGBAPositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->width,
            pkg_bgra_u8_src->height, CFCVImageType::PKG_RGBA_U8);

    int status = fcvCvtColor(pkg_bgra_u8_src, dst, CColorConvertType::CVT_PA_BGRA2PA_RGBA);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = pkg_bgra_u8_src->channels;
    int dc = dst->channels;

    int i = 0, j = 0;
    for (; i < dst->height; i++) {
        for (j = 0; j < dst->width; j++) {
            ASSERT_EQ(dst_data[j * dc + 3], src_data[j * sc + 3]);
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += dst->stride;
        src_data += pkg_bgra_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, GRAY2PkgRGBPositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->width,
            gray_u8_src->height, CFCVImageType::PKG_RGB_U8);

    int status = fcvCvtColor(gray_u8_src, dst, CColorConvertType::CVT_GRAY2PA_RGB);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    int sc = gray_u8_src->channels;
    int dc = dst->channels;

    int i = 0, j = 0;
    for (; i < dst->height; i++) {
        for (j = 0; j < dst->width; j++) {
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc]);
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc]);
        }

        dst_data += dst->stride;
        src_data += gray_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, PkgRGBA2PkgBGR565PositiveInput) {
    CMat* dst = fcvCreateCMat(pkg_bgra_u8_src->width,
            pkg_bgra_u8_src->height, CFCVImageType::PKG_BGR565_U8);

    int status = fcvCvtColor(pkg_bgra_u8_src, dst, CColorConvertType::CVT_PA_RGBA2PA_BGR565);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);

    int i = 0, j = 0;
    for (; i < pkg_bgra_u8_src->height; i++) {
        for (j = 0; j < pkg_bgra_u8_src->width; j++) {
            int sj = j << 2;
            int dj = j << 1;

            unsigned char tmp = (src_data[sj + 2] >> 3)
                    | ((src_data[sj + 1] << 3) & (0x07E0))
                    | ((src_data[sj] << 8) & (0xF800));
            ASSERT_EQ(dst_data[dj], tmp);
        }

        dst_data += dst->stride;
        src_data += pkg_bgra_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}

TEST_F(FcvColorConvertTest, GRAY2PkgBGR565PositiveInput) {
    CMat* dst = fcvCreateCMat(gray_u8_src->width,
            gray_u8_src->height, CFCVImageType::PKG_BGR565_U8);

    int status = fcvCvtColor(gray_u8_src, dst, CColorConvertType::CVT_GRAY2PA_BGR565);
    ASSERT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src->data);
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);

    int i = 0, j = 0;
    for (; i < gray_u8_src->height; i++) {
        for (j = 0; j < gray_u8_src->width; j++) {
            int dj = j << 1;

            unsigned char tmp = (src_data[j] >> 3)
                    | ((src_data[j] << 3) & (0x07E0))
                    | ((src_data[j] << 8) & (0xF800));
            ASSERT_EQ(dst_data[dj], tmp);
        }

        dst_data += dst->stride;
        src_data += gray_u8_src->stride;
    }

    fcvReleaseCMat(dst);
    dst = nullptr;
}