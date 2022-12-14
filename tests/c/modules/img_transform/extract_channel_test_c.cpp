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

#include "flycv.h"
#include "gtest/gtest.h"
#include "test_util.h"

using namespace g_fcv_ns;

class FcvExtractChannelTest : public ::testing::Test {
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

TEST_F(FcvExtractChannelTest, PkgBGRU8PositiveInput) {
    CMat* dst0 = create_cmat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::GRAY_U8);
    CMat* dst1 = create_cmat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::GRAY_U8);
    CMat* dst2 = create_cmat(pkg_bgr_u8_src->width,
            pkg_bgr_u8_src->height, CFCVImageType::GRAY_U8);

    fcvExtractChannel(pkg_bgr_u8_src, dst0, 0);
    fcvExtractChannel(pkg_bgr_u8_src, dst1, 1);
    fcvExtractChannel(pkg_bgr_u8_src, dst2, 2);

    std::vector<CMat*> dst_ptr = {dst0, dst1, dst2};
    std::vector<int> index = {0, 1, 2, 921597, 921598, 921599};
    std::vector<std::vector<unsigned char>> groundtruth = {
        {0, 0, 3, 159, 159, 159},
        {82, 84, 85, 184, 184, 184},
        {47, 49, 50, 255, 255, 255}};

    for (int n = 0; n < pkg_bgr_u8_src->channels; n++) {
        unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst_ptr[n]->data);

        for (size_t i = 0; i < index.size(); ++i) {
            ASSERT_EQ(groundtruth[n][i], (int)dst_data[index[i]]);
        }
    }

    release_cmat(dst0);
    dst0 = nullptr;
    release_cmat(dst1);
    dst1 = nullptr;
    release_cmat(dst2);
    dst2 = nullptr;
}