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

class ColorConvertTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        nv21_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::NV21);
        status = read_binary_file(NV21_1280X720_U8_BIN, nv21_src.data(),
                nv21_src.total_byte_size());
        EXPECT_EQ(status, 0);

        i420_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::I420);
        status = read_binary_file(I420_1280X720_U8_BIN, i420_src.data(),
                i420_src.total_byte_size());
        EXPECT_EQ(status, 0);

        gray_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
        status = read_binary_file(GRAY_1280X720_U8_BIN, gray_u8_src.data(),
                gray_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8_src.data(),
                pkg_bgr_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        pkg_rgb_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_RGB_U8);
        status = read_binary_file(RGB_1280X720_U8_BIN, pkg_rgb_u8_src.data(),
                pkg_rgb_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);

        pkg_bgra_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGRA_U8);
        status = read_binary_file(BGRA_1280X720_U8_BIN, pkg_bgra_u8_src.data(),
                pkg_bgra_u8_src.total_byte_size());
        EXPECT_EQ(status, 0);
    }

    Mat i420_src;
    Mat nv21_src;
    Mat gray_u8_src;
    Mat pkg_bgr_u8_src;
    Mat pkg_rgb_u8_src;
    Mat pkg_bgra_u8_src;
};

TEST_F(ColorConvertTest, PkgBGRA2NV12PositiveInput) {
    Mat dst;
    int status = cvt_color(pkg_bgra_u8_src, dst, ColorConvertType::CVT_PA_BGRA2NV21);
    EXPECT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());
    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 105, 106, 107};

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }
}

TEST_F(ColorConvertTest, PkgRGBA2NV12PositiveInput) {
    Mat dst;
    int status = cvt_color(pkg_bgra_u8_src, dst, ColorConvertType::CVT_PA_RGBA2NV12);
    EXPECT_EQ(status, 0);

    std::vector<int> groundtruth = {62, 64, 65, 174, 140, 138, 111, 112, 112};

    unsigned char* dst_data = (unsigned char*)dst.data();

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }
}

TEST_F(ColorConvertTest, PkgBGR2NV12PositiveInput) {
    Mat dst;
    int status = cvt_color(pkg_bgr_u8_src, dst, ColorConvertType::CVT_PA_BGR2NV12);

    EXPECT_EQ(status, 0);

    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 161, 161, 161};

    unsigned char* dst_data = (unsigned char*)dst.data();

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }
}

TEST_F(ColorConvertTest, PkgBGR2NV21PositiveInput) {
    Mat dst;
    int status = cvt_color(pkg_bgr_u8_src, dst, ColorConvertType::CVT_PA_BGR2NV21);
    EXPECT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());
    std::vector<int> groundtruth = {70, 71, 73, 189, 143, 141, 105, 106, 107};

    for (size_t i = 0; i < YUV_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[YUV_1280X720_IDX[i]], 1);
    }
}

TEST_F(ColorConvertTest, NV212PkgBGRPositiveInput) {
    Mat dst;
    int status = cvt_color(nv21_src, dst, ColorConvertType::CVT_NV212PA_BGR);
    EXPECT_EQ(status, 0);

    std::vector<int> groundtruth = {0, 82, 49, 85, 88, 99, 155, 160, 255};

    unsigned char* dst_data = (unsigned char*)dst.data();

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
       ASSERT_NEAR((int)dst_data[C3_1280X720_IDX[i]], groundtruth[i], 1);
    }
}

TEST_F(ColorConvertTest, PkgBGR2GRAYPositiveInput) {
    Mat gray_u8_dst;
    int status;
    status = cvt_color(pkg_bgr_u8_src, gray_u8_dst, ColorConvertType::CVT_PA_BGR2GRAY);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(gray_u8_dst.data());
    int sc = pkg_bgr_u8_src.channels();

    int i = 0, j = 0;
    for (; i < pkg_bgr_u8_src.height(); i++) {
        for (j = 0; j < pkg_bgr_u8_src.width(); j++) {
            int temp = static_cast<unsigned short>(src_data[j * sc] * 29 +
                    src_data[j * sc + 1] * 150 + src_data[j * sc + 2] * 77);
            ASSERT_EQ(dst_data[j], (unsigned char)((temp + (1 << 7)) >> 8));
        }

        dst_data += gray_u8_dst.stride();
        src_data += pkg_bgr_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgRGB2GRAYPositiveInput) {
    Mat gray_u8_dst;
    int status;
    status = cvt_color(pkg_rgb_u8_src, gray_u8_dst, ColorConvertType::CVT_PA_RGB2GRAY);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_rgb_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(gray_u8_dst.data());
    int sc = pkg_bgr_u8_src.channels();

    int i = 0, j = 0;
    for (; i < pkg_rgb_u8_src.height(); i++) {
        for (j = 0; j < pkg_rgb_u8_src.width(); j++) {
            int temp = static_cast<unsigned short>(src_data[j * sc] * 77 +
                    src_data[j * sc + 1] * 150 + src_data[j * sc + 2] * 29);
            ASSERT_EQ(dst_data[j], (unsigned char)((temp + (1 << 7)) >> 8));
        }

        dst_data += gray_u8_dst.stride();
        src_data += pkg_rgb_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgRGBA2PkgRGBPositiveInput) {
    Mat pkg_bgr_u8_dst;
    int status;
    status = cvt_color(pkg_bgra_u8_src, pkg_bgr_u8_dst, ColorConvertType::CVT_PA_RGBA2PA_RGB);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_dst.data());
    int sc = pkg_bgra_u8_src.channels();
    int dc = pkg_bgr_u8_dst.channels();

    int i = 0, j = 0, k = 0;
    for (; i < pkg_bgr_u8_dst.height(); i++) {
        for (j = 0; j < pkg_bgr_u8_dst.width(); j++) {
            for (k = 0; k < dc; k++) {
                ASSERT_EQ(dst_data[j * dc + k], src_data[j * sc + k]);
            }
        }

        dst_data += pkg_bgr_u8_dst.stride();
        src_data += pkg_bgra_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgBGR2PkgRGBAPositiveInput) {
    Mat pkg_rgba_u8_dst;
    int status;
    status = cvt_color(pkg_bgr_u8_src, pkg_rgba_u8_dst, ColorConvertType::CVT_PA_BGR2PA_RGBA);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_rgba_u8_dst.data());
    int sc = pkg_bgr_u8_src.channels();
    int dc = pkg_rgba_u8_dst.channels();

    int i = 0, j = 0;
    for (; i < pkg_rgba_u8_dst.height(); i++) {
        for (j = 0; j < pkg_rgba_u8_dst.width(); j++) {
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += pkg_rgba_u8_dst.stride();
        src_data += pkg_bgr_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgBGR2PkgRGBPositiveInput) {
    Mat pkg_rgb_u8_dst;
    int status;
    status = cvt_color(pkg_bgr_u8_src, pkg_rgb_u8_dst, ColorConvertType::CVT_PA_BGR2PA_RGB);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgr_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_rgb_u8_dst.data());
    int sc = pkg_bgr_u8_src.channels();
    int dc = pkg_rgb_u8_dst.channels();

    int i = 0, j = 0;
    for (; i < pkg_rgb_u8_dst.height(); i++) {
        for (j = 0; j < pkg_rgb_u8_dst.width(); j++) {
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += pkg_rgb_u8_dst.stride();
        src_data += pkg_bgr_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgBGRA2PkgRGBAPositiveInput) {
    Mat pkg_rgba_u8_dst;
    int status;
    status = cvt_color(pkg_bgra_u8_src, pkg_rgba_u8_dst, ColorConvertType::CVT_PA_BGRA2PA_RGBA);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_rgba_u8_dst.data());
    int sc = pkg_bgra_u8_src.channels();
    int dc = pkg_rgba_u8_dst.channels();

    int i = 0, j = 0;
    for (; i < pkg_rgba_u8_dst.height(); i++) {
        for (j = 0; j < pkg_rgba_u8_dst.width(); j++) {
            ASSERT_EQ(dst_data[j * dc + 3], src_data[j * sc + 3]);
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc + 0]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc + 1]);
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc + 2]);
        }

        dst_data += pkg_rgba_u8_dst.stride();
        src_data += pkg_bgra_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, GRAY2PkgRGBPositiveInput) {
    Mat pkg_rgb_u8_dst;
    int status;
    status = cvt_color(gray_u8_src, pkg_rgb_u8_dst, ColorConvertType::CVT_GRAY2PA_RGB);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_rgb_u8_dst.data());
    int sc = gray_u8_src.channels();
    int dc = pkg_rgb_u8_dst.channels();

    int i = 0, j = 0;
    for (; i < pkg_rgb_u8_dst.height(); i++) {
        for (j = 0; j < pkg_rgb_u8_dst.width(); j++) {
            ASSERT_EQ(dst_data[j * dc + 0], src_data[j * sc]);
            ASSERT_EQ(dst_data[j * dc + 1], src_data[j * sc]);
            ASSERT_EQ(dst_data[j * dc + 2], src_data[j * sc]);
        }

        dst_data += pkg_rgb_u8_dst.stride();
        src_data += gray_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, PkgRGBA2PkgBGR565PositiveInput) {
    Mat pkg_bgr565_u8_dst;
    int status;
    status = cvt_color(pkg_bgra_u8_src, pkg_bgr565_u8_dst, ColorConvertType::CVT_PA_RGBA2PA_BGR565);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(pkg_bgra_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_bgr565_u8_dst.data());

    int i = 0, j = 0;
    for (; i < pkg_bgra_u8_src.height(); i++) {
        for (j = 0; j < pkg_bgra_u8_src.width(); j++) {
            int sj = j << 2;
            int dj = j << 1;

            unsigned char tmp = (src_data[sj + 2] >> 3)
                    | ((src_data[sj + 1] << 3) & (0x07E0))
                    | ((src_data[sj] << 8) & (0xF800));
            ASSERT_EQ(dst_data[dj], tmp);
        }

        dst_data += pkg_bgr565_u8_dst.stride();
        src_data += pkg_bgra_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, GRAY2PkgBGR565PositiveInput) {
    Mat pkg_bgr565_u8_dst;
    int status = cvt_color(gray_u8_src, pkg_bgr565_u8_dst, ColorConvertType::CVT_GRAY2PA_BGR565);
    EXPECT_EQ(status, 0);

    unsigned char* src_data = reinterpret_cast<unsigned char*>(gray_u8_src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(pkg_bgr565_u8_dst.data());

    int i = 0, j = 0;
    for (; i < gray_u8_src.height(); i++) {
        for (j = 0; j < gray_u8_src.width(); j++) {
            int dj = j << 1;

            unsigned char tmp = (src_data[j] >> 3)
                    | ((src_data[j] << 3) & (0x07E0))
                    | ((src_data[j] << 8) & (0xF800));
            ASSERT_EQ(dst_data[dj], tmp);
        }

        dst_data += pkg_bgr565_u8_dst.stride();
        src_data += gray_u8_src.stride();
    }
}

TEST_F(ColorConvertTest, YUV4202PkgBGRPositiveInput) {
    Mat dst;
    unsigned char* srcy_data = (unsigned char*)i420_src.data();
    unsigned char* srcu_data = srcy_data + IMG_720P_WIDTH * IMG_720P_HEIGHT;
    unsigned char* srcv_data = srcu_data + (IMG_720P_WIDTH * IMG_720P_HEIGHT / 4);
    Mat src_y(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8, srcy_data);
    Mat src_u(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcu_data);
    Mat src_v(IMG_720P_WIDTH, IMG_720P_HEIGHT / 4, FCVImageType::GRAY_U8, srcv_data);

    int status = cvt_color(src_y, src_u, src_v, dst, ColorConvertType::CVT_I4202PA_BGR);
    EXPECT_EQ(status, 0);

    std::vector<int> groundtruth = {0, 82, 47, 84, 85, 92, 153, 158, 255};

    unsigned char* dst_data = (unsigned char*)dst.data();

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_NEAR((int)dst_data[C3_1280X720_IDX[i]], groundtruth[i], 1);
    }
}
