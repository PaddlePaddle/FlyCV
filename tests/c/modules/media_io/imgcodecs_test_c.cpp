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

#include <fstream>

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

class FcvImwriteTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_pkg_bgr_u8_720p_cmat(&pkg_bgr_u8_src), 0);
    }

    void TearDown() override {
        release_cmat(pkg_bgr_u8_src);
        pkg_bgr_u8_src = nullptr;
    }

    CMat* pkg_bgr_u8_src = nullptr;
};

TEST_F(FcvImwriteTest, PackageBGRU8_PositiveInput) {
    const std::string jpeg_file_name = RESULTS_PATH + "PackageBGRU8_c.jpg";
    EXPECT_TRUE(fcvImwrite(jpeg_file_name.c_str(), pkg_bgr_u8_src, 95));

    const std::string png_file_name = RESULTS_PATH + "PackageBGRU8_c.png";
    EXPECT_TRUE(fcvImwrite(png_file_name.c_str(), pkg_bgr_u8_src, 95));
}

TEST(FcvImreadTest, PositiveInput) {
    CMat* img_jpg = fcvImread(JPG_1280X720.c_str(), 0);
    EXPECT_FALSE(img_jpg->total_byte_size == 0);

    /// read png
    CMat* img_png = fcvImread(PNG_1280X720.c_str(), 0);
    EXPECT_FALSE(img_png->total_byte_size == 0);
}

TEST(FcvImgCodecs, Imdecode) {
    std::ifstream fs_png(PNG_1280X720, std::ios::binary);
    EXPECT_TRUE(fs_png.is_open());
    fs_png.seekg(0, std::ios::end);
    size_t len = fs_png.tellg();
    fs_png.seekg(0, std::ios::beg);
    std::vector<char> png_data(len);
    fs_png.read(png_data.data(), len);
    fs_png.close();

    CMat* img_png = fcvImdecode((uint8_t*)png_data.data(), png_data.size(), 0);
    release_cmat(img_png);
    img_png = nullptr;

    std::ifstream fs_jpg(JPG_1280X720, std::ios::binary);
    EXPECT_TRUE(fs_jpg.is_open());
    fs_jpg.seekg(0, std::ios::end);
    size_t jpg_len = fs_jpg.tellg();
    fs_jpg.seekg(0, std::ios::beg);
    std::vector<char> jpg_data(jpg_len);
    fs_jpg.read(jpg_data.data(), jpg_len);
    fs_jpg.close();

    CMat* img_jpg = fcvImdecode((uint8_t*)jpg_data.data(), jpg_data.size(), 0);
    release_cmat(img_jpg);
    img_jpg = nullptr;
}

TEST(FcvImgCodecs, Imencode) {
    // Allocate src Mat
    CMat* src = create_cmat(IMG_720P_WIDTH, IMG_720P_HEIGHT, CFCVImageType::PKG_BGR_U8);

    // Read BGR data from binary file
    FILE* p_file = fopen(BGR_1280X720_U8_BIN.c_str(), "rb");
    EXPECT_NE(p_file, nullptr);
    size_t INPUT_SIZE = IMG_720P_WIDTH * IMG_720P_HEIGHT * 3;
    size_t count = fread(src->data, 1, INPUT_SIZE, p_file);
    fclose(p_file);
    EXPECT_EQ(count, INPUT_SIZE);

    unsigned char* jpg_data = nullptr;
    uint64_t jpg_data_len = 0;
    EXPECT_TRUE(fcvImencode(".jpg", src, &jpg_data, &jpg_data_len, 95));

    unsigned char* png_data = nullptr;
    uint64_t png_data_len = 0;
    EXPECT_TRUE(fcvImencode(".png", src, &png_data, &png_data_len, 75));
}