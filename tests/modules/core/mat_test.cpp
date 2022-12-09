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

class MatTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_gray_u8_720p(gray_u8_src), 0);
        ASSERT_EQ(prepare_gray_u16_720p(gray_u16_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_u8_720p(pkg_bgr_u8_src), 0);
        ASSERT_EQ(prepare_pkg_bgr_f32_720p(pkg_bgr_f32_src), 0);
        ASSERT_EQ(prepare_nv21_720p(nv21_src), 0);
        ASSERT_EQ(prepare_i420_720p(i420_src), 0);
    }

    Mat gray_u8_src;
    Mat gray_u16_src;
    Mat pkg_bgr_u8_src;
    Mat pkg_bgr_f32_src;
    Mat nv21_src;
    Mat i420_src;
};

TEST_F(MatTest, ConstructorPositiveInput) {
    Mat gray_u8 = Mat(1280, 720, FCVImageType::GRAY_U8);
    EXPECT_EQ(1280, gray_u8.stride());

    Mat gray_u16 = Mat(1280, 720, FCVImageType::GRAY_U16);
    EXPECT_EQ(1280 * 2, gray_u16.stride());

    Mat gray_s16 = Mat(1280, 720, FCVImageType::GRAY_S16);
    EXPECT_EQ(1280 * 2, gray_s16.stride());

    Mat gray_s32 = Mat(1280, 720, FCVImageType::GRAY_S32);
    EXPECT_EQ(1280 * 4, gray_s32.stride());

    Mat gray_f64 = Mat(1280, 720, FCVImageType::GRAY_F64);
    EXPECT_EQ(1280 * 8, gray_f64.stride());

    Mat pkg_bgr_u8 = Mat(1280, 720, FCVImageType::PKG_BGR_U8);
    EXPECT_EQ(1280 * 3, pkg_bgr_u8.stride());

    Mat pkg_bgr_f32 = Mat(1280, 720, FCVImageType::PKG_BGR_F32);
    EXPECT_EQ(1280 * 3 * 4, pkg_bgr_f32.stride());

    Mat nv12 = Mat(1280, 720, FCVImageType::NV12);
    EXPECT_EQ(1280, nv12.stride());
    EXPECT_EQ(3, nv12.channels());

    Mat nv21 = Mat(1280, 720, FCVImageType::NV21);
    EXPECT_EQ(1280, nv21.stride());
    EXPECT_EQ(3, nv21.channels());

    Mat i420 = Mat(1280, 720, FCVImageType::I420);
    EXPECT_EQ(1280, i420.stride());
    EXPECT_EQ(3, i420.channels());

    Size size(1280, 720);
    Mat pla_rgb_u8 = Mat(size, FCVImageType::PLA_RGB_U8);
    EXPECT_EQ(1280, pla_rgb_u8.width());
    EXPECT_EQ(720, pla_rgb_u8.height());
}

TEST_F(MatTest, AtPositiveInput) {
    Mat gray_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_U8);
    for (int y = 0; y < gray_u8.height(); ++y) {
        for (int x = 0; x < gray_u8.width(); ++x) {
            gray_u8.at<unsigned char>(x, y) = y % 256;
            ASSERT_EQ((int)gray_u8.at<unsigned char>(x, y), y % 256);
        }
    }

    Mat gray_u16(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_U16);
    for (int y = 0; y < gray_u16.height(); ++y) {
        for (int x = 0; x < gray_u16.width(); ++x) {
            gray_u16.at<unsigned short>(x, y) = y % 256;
            ASSERT_EQ((int)gray_u16.at<unsigned short>(x, y), y % 256);
        }
    }

    Mat gray_s16(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_S16);
    for (int y = 0; y < gray_s16.height(); ++y) {
        for (int x = 0; x < gray_s16.width(); ++x) {
            gray_s16.at<signed short>(x, y) = y % 256;
            ASSERT_EQ((int)gray_s16.at<signed short>(x, y), y % 256);
        }
    }

    Mat gray_s32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_S32);
    for (int y = 0; y < gray_s32.height(); ++y) {
        for (int x = 0; x < gray_s32.width(); ++x) {
            gray_s32.at<int>(x, y) = y % 256;
            ASSERT_EQ((int)gray_s32.at<int>(x, y), y % 256);
        }
    }

    Mat gray_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_F32);
    for (int y = 0; y < gray_f32.height(); ++y) {
        for (int x = 0; x < gray_f32.width(); ++x) {
            gray_f32.at<float>(x, y) = y % 256;
            ASSERT_FLOAT_EQ(gray_f32.at<float>(x, y), y % 256);
        }
    }

    Mat gray_f64(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::GRAY_F64);
    for (int y = 0; y < gray_f64.height(); ++y) {
        for (int x = 0; x < gray_f64.width(); ++x) {
            gray_f64.at<double>(x, y) = y % 256;
            ASSERT_DOUBLE_EQ((double)gray_f64.at<double>(x, y), y % 256);
        }
    }

    Mat pla_bgr_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_BGR_U8);
    for (int y = 0; y < pla_bgr_u8.height(); ++y) {
        for (int x = 0; x < pla_bgr_u8.width(); ++x) {
            for (int c = 0; c < pla_bgr_u8.channels(); ++c) {
                pla_bgr_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_bgr_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_rgb_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_RGB_U8);
    for (int y = 0; y < pla_rgb_u8.height(); ++y) {
        for (int x = 0; x < pla_rgb_u8.width(); ++x) {
            for (int c = 0; c < pla_rgb_u8.channels(); ++c) {
                pla_rgb_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_rgb_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgr_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGR_U8);
    for (int y = 0; y < pkg_bgr_u8.height(); ++y) {
        for (int x = 0; x < pkg_bgr_u8.width(); ++x) {
            for (int c = 0; c < pkg_bgr_u8.channels(); ++c) {
                pkg_bgr_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pkg_bgr_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgb_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGB_U8);
    for (int y = 0; y < pkg_rgb_u8.height(); ++y) {
        for (int x = 0; x < pkg_rgb_u8.width(); ++x) {
            for (int c = 0; c < pkg_rgb_u8.channels(); ++c) {
                pkg_rgb_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pkg_rgb_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_bgra_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_BGRA_U8);
    for (int y = 0; y < pla_bgra_u8.height(); ++y) {
        for (int x = 0; x < pla_bgra_u8.width(); ++x) {
            for (int c = 0; c < pla_bgra_u8.channels(); ++c) {
                pla_bgra_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_bgra_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_rgba_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_RGBA_U8);
    for (int y = 0; y < pla_rgba_u8.height(); ++y) {
        for (int x = 0; x < pla_rgba_u8.width(); ++x) {
            for (int c = 0; c < pla_rgba_u8.channels(); ++c) {
                pla_rgba_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_rgba_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgra_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGRA_U8);
    for (int y = 0; y < pkg_bgra_u8.height(); ++y) {
        for (int x = 0; x < pkg_bgra_u8.width(); ++x) {
            for (int c = 0; c < pkg_bgra_u8.channels(); ++c) {
                pkg_bgra_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pkg_bgra_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgba_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGBA_U8);
    for (int y = 0; y < pkg_rgba_u8.height(); ++y) {
        for (int x = 0; x < pkg_rgba_u8.width(); ++x) {
            for (int c = 0; c < pkg_rgba_u8.channels(); ++c) {
                pkg_rgba_u8.at<unsigned char>(x, y, c) = y % 256;
                ASSERT_EQ((int)pkg_rgba_u8.at<unsigned char>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_bgr_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_BGR_F32);
    for (int y = 0; y < pla_bgr_f32.height(); ++y) {
        for (int x = 0; x < pla_bgr_f32.width(); ++x) {
            for (int c = 0; c < pla_bgr_f32.channels(); ++c) {
                pla_bgr_f32.at<float>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_bgr_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_rgb_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_RGB_F32);
    for (int y = 0; y < pla_rgb_f32.height(); ++y) {
        for (int x = 0; x < pla_rgb_f32.width(); ++x) {
            for (int c = 0; c < pla_rgb_f32.channels(); ++c) {
                pla_rgb_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pla_rgb_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgr_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGR_F32);
    for (int y = 0; y < pkg_bgr_f32.height(); ++y) {
        for (int x = 0; x < pkg_bgr_f32.width(); ++x) {
            for (int c = 0; c < pkg_bgr_f32.channels(); ++c) {
                pkg_bgr_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pkg_bgr_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgb_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGB_F32);
    for (int y = 0; y < pkg_rgb_f32.height(); ++y) {
        for (int x = 0; x < pkg_rgb_f32.width(); ++x) {
            for (int c = 0; c < pkg_rgb_f32.channels(); ++c) {
                pkg_rgb_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pkg_rgb_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_bgra_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_BGRA_F32);
    for (int y = 0; y < pla_bgra_f32.height(); ++y) {
        for (int x = 0; x < pla_bgra_f32.width(); ++x) {
            for (int c = 0; c < pla_bgra_f32.channels(); ++c) {
                pla_bgra_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pla_bgra_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pla_rgba_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PLA_RGBA_F32);
    for (int y = 0; y < pla_rgba_f32.height(); ++y) {
        for (int x = 0; x < pla_rgba_f32.width(); ++x) {
            for (int c = 0; c < pla_rgba_f32.channels(); ++c) {
                pla_rgba_f32.at<float>(x, y, c) = y % 256;
                ASSERT_EQ((int)pla_rgba_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgra_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGRA_F32);
    for (int y = 0; y < pkg_bgra_f32.height(); ++y) {
        for (int x = 0; x < pkg_bgra_f32.width(); ++x) {
            for (int c = 0; c < pkg_bgra_f32.channels(); ++c) {
                pkg_bgra_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pkg_bgra_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgba_f32(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGBA_F32);
    for (int y = 0; y < pkg_rgba_f32.height(); ++y) {
        for (int x = 0; x < pkg_rgba_f32.width(); ++x) {
            for (int c = 0; c < pkg_rgba_f32.channels(); ++c) {
                pkg_rgba_f32.at<float>(x, y, c) = y % 256;
                ASSERT_FLOAT_EQ(pkg_rgba_f32.at<float>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgr_f64(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGR_F64);
    for (int y = 0; y < pkg_bgr_f64.height(); ++y) {
        for (int x = 0; x < pkg_bgr_f64.width(); ++x) {
            for (int c = 0; c < pkg_bgr_f64.channels(); ++c) {
                pkg_bgr_f64.at<double>(x, y, c) = y % 256;
                ASSERT_DOUBLE_EQ(pkg_bgr_f64.at<double>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgb_f64(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGB_F64);
    for (int y = 0; y < pkg_rgb_f64.height(); ++y) {
        for (int x = 0; x < pkg_rgb_f64.width(); ++x) {
            for (int c = 0; c < pkg_rgb_f64.channels(); ++c) {
                pkg_rgb_f64.at<double>(x, y, c) = y % 256;
                ASSERT_DOUBLE_EQ(pkg_rgb_f64.at<double>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgra_f64(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGRA_F64);
    for (int y = 0; y < pkg_bgra_f64.height(); ++y) {
        for (int x = 0; x < pkg_bgra_f64.width(); ++x) {
            for (int c = 0; c < pkg_bgra_f64.channels(); ++c) {
                pkg_bgra_f64.at<double>(x, y, c) = y % 256;
                ASSERT_DOUBLE_EQ(pkg_bgra_f64.at<double>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_rgba_f64(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGBA_F64);
    for (int y = 0; y < pkg_rgba_f64.height(); ++y) {
        for (int x = 0; x < pkg_rgba_f64.width(); ++x) {
            for (int c = 0; c < pkg_rgba_f64.channels(); ++c) {
                pkg_rgba_f64.at<double>(x, y, c) = y % 256;
                ASSERT_DOUBLE_EQ(pkg_rgba_f64.at<double>(x, y, c), y % 256);
            }
        }
    }

    Mat pkg_bgr565_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_BGR565_U8);
    for (int y = 0; y < pkg_bgr565_u8.height(); ++y) {
        for (int x = 0; x < pkg_bgr565_u8.width(); ++x) {
            for (int c = 0; c < pkg_bgr565_u8.channels(); ++c) {
                pkg_bgr565_u8.at<unsigned char>(x, y) = y % 256;
                ASSERT_EQ((int)pkg_bgr565_u8.at<unsigned char>(x, y), y % 256);
            }
        }
    }

    Mat pkg_rgb565_u8(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::PKG_RGB565_U8);
    for (int y = 0; y < pkg_rgb565_u8.height(); ++y) {
        for (int x = 0; x < pkg_rgb565_u8.width(); ++x) {
            for (int c = 0; c < pkg_rgb565_u8.channels(); ++c) {
                pkg_rgb565_u8.at<unsigned char>(x, y) = y % 256;
                ASSERT_EQ((int)pkg_rgb565_u8.at<unsigned char>(x, y), y % 256);
            }
        }
    }

    Mat nv12(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::NV12);
    for (int y = 0; y < nv12.height(); ++y) {
        for (int x = 0; x < nv12.width(); ++x) {
            for (int c = 0; c < nv12.channels(); ++c) {
                nv12.at<unsigned char>(x, y) = y % 256;
                ASSERT_EQ((int)nv12.at<unsigned char>(x, y), y % 256);
            }
        }
    }

    Mat nv21(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::NV21);
    for (int y = 0; y < nv21.height(); ++y) {
        for (int x = 0; x < nv21.width(); ++x) {
            for (int c = 0; c < nv21.channels(); ++c) {
                nv21.at<unsigned char>(x, y) = y % 256;
                ASSERT_EQ((int)nv21.at<unsigned char>(x, y), y % 256);
            }
        }
    }

    Mat i420(IMG_480P_WIDTH, IMG_480P_HEIGHT, FCVImageType::I420);
    for (int y = 0; y < i420.height(); ++y) {
        for (int x = 0; x < i420.width(); ++x) {
            for (int c = 0; c < i420.channels(); ++c) {
                i420.at<unsigned char>(x, y) = y % 256;
                ASSERT_EQ((int)i420.at<unsigned char>(x, y), y % 256);
            }
        }
    }
}

TEST_F(MatTest, AtNV21PositiveInput) {
    Mat dst(nv21_src.width(), nv21_src.height(), FCVImageType::NV21);

    for (int y = 0; y < dst.height(); y++) {
        for (int x = 0; x < dst.width(); x++) {
            for (int c = 0; c < dst.channels(); c++) {
                dst.at<char>(x, y, c) = nv21_src.at<char>(x, y, c);
            }
        }
    }

    unsigned char* src_data = static_cast<unsigned char*>(nv21_src.data());
    unsigned char* dst_data = static_cast<unsigned char*>(dst.data());

    for (int i = 0; i < dst.width() * dst.height() * dst.channels() / 2; ++i) {
        ASSERT_EQ(src_data[i], dst_data[i]);
    }
}

TEST_F(MatTest, AtI420PositiveInput) {
    Mat dst(i420_src.width(), i420_src.height(), FCVImageType::I420);

    for (int y = 0; y < dst.height(); y++) {
        for (int x = 0; x < dst.width(); x++) {
            for (int c = 0; c < dst.channels(); c++) {
                dst.at<char>(x, y, c) = i420_src.at<char>(x, y, c);
            }
        }
    }

    unsigned char* src_data = static_cast<unsigned char*>(i420_src.data());
    unsigned char* dst_data = static_cast<unsigned char*>(dst.data());

    for (int i = 0; i < dst.width() * dst.height() * dst.channels() / 2; ++i) {
        ASSERT_EQ(src_data[i], dst_data[i]);
    }
}

TEST_F(MatTest, ConvertToPositiveInput) {
    Mat dst;
    pkg_bgr_u8_src.convert_to(dst, FCVImageType::PKG_BGR_F32, 0.5, 10);

    float* dst_data = reinterpret_cast<float*>(dst.data());
    std::vector<float> groundtruth = {10.0f, 51.0f, 33.5f,
            52.0f, 55.0f, 62.0f, 87.5f, 89.5f, 137.5f};

    for (size_t i = 0; i < C3_1280X720_IDX.size(); ++i) {
        ASSERT_FLOAT_EQ(groundtruth[i], dst_data[C3_1280X720_IDX[i]]);
    }
}

TEST_F(MatTest, ConvertToNegativeInput) {
    Mat dst_gray_u8;
    int status = pkg_bgr_u8_src.convert_to(dst_gray_u8, FCVImageType::GRAY_U8, 0.5, 10);
    EXPECT_NE(status, 0);

    Mat dst_gray_u16;
    status = pkg_bgr_u8_src.convert_to(dst_gray_u16, FCVImageType::GRAY_U16, 0.5, 10);
    EXPECT_NE(status, 0);

    Mat dst_gray_s32;
    status = pkg_bgr_u8_src.convert_to(dst_gray_s32, FCVImageType::GRAY_S32, 0.5, 10);
    EXPECT_NE(status, 0);

    Mat dst_pkg_rgb;
    status = pkg_bgr_u8_src.convert_to(dst_pkg_rgb, FCVImageType::PKG_RGB_U8, 0.5, 10);
    EXPECT_NE(status, 0);

    Mat dst_pla_bgra;
    status = pkg_bgr_u8_src.convert_to(dst_pla_bgra, FCVImageType::PLA_BGRA_U8, 0.5, 10);
    EXPECT_NE(status, 0);
}

TEST_F(MatTest, CopyToPositiveInput) {
    Mat dst;

    pkg_bgr_u8_src.copy_to(dst);
    unsigned char* src_data = static_cast<unsigned char*>(pkg_bgr_u8_src.data());
    unsigned char* dst_data = static_cast<unsigned char*>(dst.data());

   for (int i = 0; i < dst.width() * dst.height() * dst.channels(); ++i) {
        ASSERT_EQ(src_data[i], dst_data[i]);
    }
}

TEST_F(MatTest, CopyToWithMaskPositiveInput) {
    Mat mask(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
    for (int y = 0; y < mask.height(); y++) {
        for (int x = 0; x < mask.width(); x++) {
            mask.at<char>(x, y) = 1;
        }
    }

    Mat src_gray_u8(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::GRAY_U8);
    unsigned char min = 0;
    unsigned char max = 255;
    // init src Mat
    int num = IMG_720P_WIDTH * IMG_720P_HEIGHT;
    init_random((unsigned char *)src_gray_u8.data(), num * src_gray_u8.channels(), min, max);

    Mat gray_u8_dst;
    Mat pkg_bgr_u8_dst;

    src_gray_u8.copy_to(gray_u8_dst, mask);
    pkg_bgr_u8_src.copy_to(pkg_bgr_u8_dst, mask);

    unsigned char* src_data = static_cast<unsigned char*>(pkg_bgr_u8_src.data());
    unsigned char* dst_data = static_cast<unsigned char*>(pkg_bgr_u8_dst.data());

    for (int i = 0; i < pkg_bgr_u8_dst.width() * pkg_bgr_u8_dst.height() *
            pkg_bgr_u8_dst.channels(); ++i) {
        ASSERT_EQ(src_data[i], dst_data[i]);
    }

    src_data = static_cast<unsigned char*>(gray_u8_dst.data());
    dst_data = static_cast<unsigned char*>(gray_u8_dst.data());

    for (int i = 0; i < gray_u8_dst.width() * gray_u8_dst.height(); ++i) {
        ASSERT_EQ(src_data[i], dst_data[i]);
    }
}

TEST_F(MatTest, DotPositiveInput) {
    double result0 = gray_u8_src.dot(gray_u8_src);
    EXPECT_DOUBLE_EQ(result0, 23713082282);

    double result1 = gray_u16_src.dot(gray_u16_src);
    EXPECT_DOUBLE_EQ(result1, 23713082282);

    double result2 = pkg_bgr_u8_src.dot(pkg_bgr_u8_src);
    EXPECT_DOUBLE_EQ(result2, 70768231298);

    double result3 = pkg_bgr_f32_src.dot(pkg_bgr_f32_src);
    EXPECT_DOUBLE_EQ(result3, 70768231298);
}
