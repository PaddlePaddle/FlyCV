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

class FcvCropTest : public ::testing::Test {
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

TEST_F(FcvCropTest, PkgBGRU8PositiveInput) {
    CRect rect;
    rect.x = int(IMG_720P_WIDTH / 4);
    rect.y = int(IMG_720P_HEIGHT / 4);
    rect.width = int(IMG_720P_WIDTH / 2);
    rect.height = int(IMG_720P_HEIGHT / 2);

    CMat* dst = create_cmat(rect.width, rect.height, pkg_bgr_u8_src->type);
    fcvCrop(pkg_bgr_u8_src, dst, &rect);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    std::vector<int> index = {0, 1, 2, 345600, 345601, 345602, 691197, 691198, 691199};
    std::vector<int> groundtruth = {184, 235, 255, 139, 133, 126, 135, 143, 172};

    for (size_t i = 0; i < index.size(); ++i) {
        ASSERT_EQ(groundtruth[i], (int)dst_data[index[i]]);
    }
}
