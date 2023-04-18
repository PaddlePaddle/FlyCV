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

TEST(FcvMatrixMulTest, GrayF32PositiveInput) {
    CMat* src0 = fcvCreateCMat(5, 4, CFCVImageType::GRAY_F32);
    CMat* src1 = fcvCreateCMat(10, 5, CFCVImageType::GRAY_F32);

    float* src0_data = (float*)src0->data;
    float* src1_data = (float*)src1->data;

    for (size_t i = 0; i < src0->total_byte_size / src0->type_byte_size; ++i) {
        src0_data[i] = 1.0f / (i + 1);
    }

    for (size_t i = 0; i < src1->total_byte_size / src1->type_byte_size; ++i) {
        src1_data[i] = 1.0f / (i + 1);
    }

    CMat* result = fcvMatrixMul(src0, src1);
    float* result_ptr = (float*)result->data;

    std::vector<float> groundtruth = {
            1.074270f, 0.569393f, 0.398515f, 0.311502f, 0.258254f,
            0.222029f, 0.195627f, 0.175428f, 0.159413f, 0.146361f,
            0.191629f, 0.106773f, 0.077672f, 0.062620f, 0.053254f,
            0.046774f, 0.041973f, 0.038241f, 0.035237f, 0.032754f,
            0.106078f, 0.059715f, 0.043773f, 0.035501f, 0.030337f,
            0.026752f, 0.024087f, 0.022009f, 0.020332f, 0.018941f,
            0.073410f, 0.041512f, 0.030531f, 0.024826f, 0.021259f,
            0.018779f, 0.016933f, 0.015491f, 0.014326f, 0.013359f};

    for (size_t i = 0; i < result->total_byte_size / result->type_byte_size; ++i) {
        ASSERT_NEAR(groundtruth[i], result_ptr[i], 0.0001f);
    }

    fcvReleaseCMat(src0);
    src0 = nullptr;
    fcvReleaseCMat(src1);
    src1 = nullptr;
    fcvReleaseCMat(result);
    result = nullptr;
}

TEST(FcvMatrixMulTest, GrayF64PositiveInput) {
    CMat* src0 = fcvCreateCMat(5, 4, CFCVImageType::GRAY_F64);
    CMat* src1 = fcvCreateCMat(10, 5, CFCVImageType::GRAY_F64);

    double* src0_data = (double*)src0->data;
    double* src1_data = (double*)src1->data;

    for (size_t i = 0; i < src0->total_byte_size / src0->type_byte_size; ++i) {
        src0_data[i] = 1.0f / (i + 1);
    }

    for (size_t i = 0; i < src1->total_byte_size / src1->type_byte_size; ++i) {
        src1_data[i] = 1.0f / (i + 1);
    }

    CMat* result = fcvMatrixMul(src0, src1);
    double* result_ptr = (double*)result->data;

    std::vector<double> groundtruth = {
            1.074270f, 0.569393f, 0.398515f, 0.311502f, 0.258254f,
            0.222029f, 0.195627f, 0.175428f, 0.159413f, 0.146361f,
            0.191629f, 0.106773f, 0.077672f, 0.062620f, 0.053254f,
            0.046774f, 0.041973f, 0.038241f, 0.035237f, 0.032754f,
            0.106078f, 0.059715f, 0.043773f, 0.035501f, 0.030337f,
            0.026752f, 0.024087f, 0.022009f, 0.020332f, 0.018941f,
            0.073410f, 0.041512f, 0.030531f, 0.024826f, 0.021259f,
            0.018779f, 0.016933f, 0.015491f, 0.014326f, 0.013359f};

    for (size_t i = 0; i < result->total_byte_size / result->type_byte_size; ++i) {
        ASSERT_NEAR(groundtruth[i], result_ptr[i], 0.0001f);
    }
}
