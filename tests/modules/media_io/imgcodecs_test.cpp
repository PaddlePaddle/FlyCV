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

#include <fstream>

#include "gtest/gtest.h"
#include "flycv.h"
#include "test_util.h"

using namespace g_fcv_ns;

class ImwriteTest : public ::testing::Test {
protected:
    void SetUp() override {
        int status = 0;
        pkg_bgr_u8_src = Mat(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
        status = read_binary_file(BGR_1280X720_U8_BIN, pkg_bgr_u8_src.data(),
                pkg_bgr_u8_src.width() * pkg_bgr_u8_src.height() * pkg_bgr_u8_src.channels());
        EXPECT_EQ(status, 0);
    }

    Mat pkg_bgr_u8_src;
};

TEST_F(ImwriteTest, PackageBGRU8_PositiveInput) {
    const std::string jpeg_file_name = RESULTS_PATH + "PackageBGRU8.jpg";
    EXPECT_TRUE(imwrite(jpeg_file_name, pkg_bgr_u8_src));

    const std::string png_file_name = RESULTS_PATH + "PackageBGRU8.png";
    EXPECT_TRUE(imwrite(png_file_name, pkg_bgr_u8_src));
}

TEST(ImreadTest, PositiveInput) {
    Mat img_jpg = imread(JPG_1280X720);
    EXPECT_FALSE(img_jpg.empty());

    /// read png
    Mat img_png = imread(PNG_1280X720);
    EXPECT_FALSE(img_png.empty());
}

TEST(ImgCodecs, Imdecode) {
    std::ifstream fs_png(PNG_1280X720, std::ios::binary);
    EXPECT_TRUE(fs_png.is_open());
    fs_png.seekg(0, std::ios::end);
    size_t len = fs_png.tellg();
    fs_png.seekg(0, std::ios::beg);
    std::vector<char> png_data(len);
    fs_png.read(png_data.data(), len);
    fs_png.close();
    Mat img = imdecode((uint8_t*)png_data.data(), png_data.size());
    imwrite(RESULTS_PATH + "imdecode.png", img);

    std::ifstream fs_jpg(JPG_1280X720, std::ios::binary);
    EXPECT_TRUE(fs_jpg.is_open());
    fs_jpg.seekg(0, std::ios::end);
    size_t jpg_len = fs_jpg.tellg();
    fs_jpg.seekg(0, std::ios::beg);
    std::vector<char> jpg_data(jpg_len);
    fs_jpg.read(jpg_data.data(), jpg_len);
    fs_jpg.close();
    Mat img_jpg = imdecode((uint8_t*)jpg_data.data(), jpg_data.size());
    imwrite(RESULTS_PATH + "imdecode.jpg", img_jpg);
}

TEST(ImgCodecs, Imencode) {
    // Allocate src Mat
    Mat src(IMG_720P_WIDTH, IMG_720P_HEIGHT, FCVImageType::PKG_BGR_U8);
    // Read BGR data from binary file
    FILE* p_file = fopen(BGR_1280X720_U8_BIN.c_str(), "rb");
    EXPECT_NE(p_file, nullptr);
    size_t INPUT_SIZE = IMG_720P_WIDTH * IMG_720P_HEIGHT * 3;
    size_t count = fread(src.data(), 1, INPUT_SIZE, p_file);
    fclose(p_file);
    EXPECT_EQ(count, INPUT_SIZE);

    std::vector<uint8_t> jpeg_data;
    EXPECT_TRUE(imencode(".jpg", src, jpeg_data));
    const std::string jpg_file_name = RESULTS_PATH + "imencode.jpg";
    std::ofstream fs_jpg(jpg_file_name, std::ios::binary);
    fs_jpg.write((char*)jpeg_data.data(), jpeg_data.size());
    fs_jpg.close();

    std::vector<uint8_t> png_data;
    EXPECT_TRUE(imencode(".png", src, png_data));
    const std::string png_file_name = RESULTS_PATH + "imencode.png";
    std::ofstream fs_png(png_file_name, std::ios::binary);
    fs_png.write((char*)png_data.data(), png_data.size());
    fs_png.close();
}
