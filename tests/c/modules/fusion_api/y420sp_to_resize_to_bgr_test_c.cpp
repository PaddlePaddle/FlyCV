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

class FcvY420SPToResizeToBGRTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_EQ(prepare_nv12_720p_cmat(&nv12_src), 0);
        ASSERT_EQ(prepare_nv21_720p_cmat(&nv21_src), 0);
    }

    void TearDown() override {
        release_cmat(nv12_src);
        nv12_src = nullptr;
        release_cmat(nv21_src);
        nv21_src = nullptr;
    }

    CMat* nv12_src = nullptr;
    CMat* nv21_src = nullptr;
};

TEST_F(FcvY420SPToResizeToBGRTest, NV12ToResizeToBGRPositiveInput) {
    CMat* dst = create_cmat(640, 360, CFCVImageType::PKG_BGR_U8);
    int status = fcvNv12ToResizeToBgr(nv12_src, dst, CInterpolationType::INTER_LINEAR);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    std::vector<int> groundtruth = {3, 86, 51, 85, 114, 125, 184, 158, 254};

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[C3_640X360_IDX[i]], 1);
    }

    release_cmat(dst);
    dst = nullptr;
}

TEST_F(FcvY420SPToResizeToBGRTest, NV21ToResizeToBGRPositiveInput) {
    CMat* dst = create_cmat(640, 360, CFCVImageType::PKG_BGR_U8);
    int status = fcvNv21ToResizeToBgr(nv21_src, dst, CInterpolationType::INTER_LINEAR);
    ASSERT_EQ(status, 0);

    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst->data);
    std::vector<int> groundtruth = {3, 86, 51, 85, 114, 125, 184, 158, 254};

    for (size_t i = 0; i < C3_640X360_IDX.size(); ++i) {
        ASSERT_NEAR(groundtruth[i], (int)dst_data[C3_640X360_IDX[i]], 1);
    }
}
