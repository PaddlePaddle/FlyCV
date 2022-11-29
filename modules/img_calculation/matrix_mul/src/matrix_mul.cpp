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

#include "modules/img_calculation/matrix_mul/interface/matrix_mul.h"
#include "modules/img_transform/rotation/interface/rotation.h"

#ifdef HAVE_NEON
#include <arm_neon.h>
#endif

#ifdef HAVE_OPENCL
#include "modules/core/opencl/interface/opencl.h"
#include "modules/img_calculation/matrix_mul/include/matrix_mul_ocl.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
void matrix_multiply_conmmon(
        const Mat& src0,
        const Mat& src1,
        Mat& dst){
    const int M = src0.height();
    const int N = src1.width() * src0.channels();
    const int K = src0.width();
    const int stride0 = src0.stride() / sizeof(T);
    const int stride1 = src1.stride() / sizeof(T);
    const int stride2 = dst.stride() / sizeof(T);

    const T* src0_data = (const T*)src0.data();
    const T* src1_data = (const T*)src1.data();
    T* dst_data  = (T*)dst.data();

    for (int m = 0; m < M; m++) {
        T* dst_ptr = dst_data + m * stride2;
        const T* src1_ptr = src1_data;
        for (int k = 0; k < K; k++) {
            T tmp = 0.f;
            T tmp1 = src0_data[k];
            for (int n = 0; n < N; n ++) {
                if (k==0) {
                    dst_ptr[n] = tmp;
                }

                dst_ptr[n] += tmp1 * src1_ptr[n];
            }
            src1_ptr += stride1;
        }
        src0_data += stride0;
    }
}

#ifdef HAVE_NEON
void matrix_multiply_f32_neon(
        const Mat& src0,
        const Mat& src1,
        Mat& dst) {
    const int M = src0.height();
    const int N = src1.width();
    const int K = src0.width();

    Mat src1_t(src1.height(), src1.width(), src1.type());
    transpose(src1, src1_t);
    const int stride0 = src0.stride() / sizeof(float);
    const int stride1 = src1_t.stride() / sizeof(float);
    const int stride2 = dst.stride() / sizeof(float);

    const float* src0_data = (const float*)src0.data();
    const float* src1_data = (const float*)src1_t.data();
    float* dst_data  = (float*)dst.data();

    int n_align4 = N & (~3);
    int k_align4 = K & (~3);
    int n = 0, k = 0;
    for (int m = 0; m < M; m++) {
        const float* src0_ptr = src0_data + m * stride0;
        float* dst_ptr = dst_data + m * stride2;
        for (n = 0; n < n_align4; n += 4) {
            float32x4_t vtmp0 = vdupq_n_f32(0.f);
            float32x4_t vtmp1 = vdupq_n_f32(0.f);
            float32x4_t vtmp2 = vdupq_n_f32(0.f);
            float32x4_t vtmp3 = vdupq_n_f32(0.f);
            const float* src10_ptr = src1_data + (n + 0) * stride1;
            const float* src11_ptr = src1_data + (n + 1) * stride1;
            const float* src12_ptr = src1_data + (n + 2) * stride1;
            const float* src13_ptr = src1_data + (n + 3) * stride1;

            for (k = 0; k < k_align4; k += 4) {
                float32x4_t va  = vld1q_f32(src0_ptr + k);
                float32x4_t vb0 = vld1q_f32(src10_ptr + k);
                float32x4_t vb1 = vld1q_f32(src11_ptr + k);
                float32x4_t vb2 = vld1q_f32(src12_ptr + k);
                float32x4_t vb3 = vld1q_f32(src13_ptr + k);

                vtmp0 = vmlaq_f32(vtmp0, va, vb0);
                vtmp1 = vmlaq_f32(vtmp1, va, vb1);
                vtmp2 = vmlaq_f32(vtmp2, va, vb2);
                vtmp3 = vmlaq_f32(vtmp3, va, vb3);
            }

            float tmp0 = 0.f;
            float tmp1 = 0.f;
            float tmp2 = 0.f;
            float tmp3 = 0.f;
            for (; k < K; k++) {
                tmp0 += src0_ptr[k] * src10_ptr[k];
                tmp1 += src0_ptr[k] * src11_ptr[k];
                tmp2 += src0_ptr[k] * src12_ptr[k];
                tmp3 += src0_ptr[k] * src13_ptr[k];
            }
            dst_ptr[n + 0] = tmp0 + (vtmp0[0] + vtmp0[1] + vtmp0[2] + vtmp0[3]);
            dst_ptr[n + 1] = tmp1 + (vtmp1[0] + vtmp1[1] + vtmp1[2] + vtmp1[3]);
            dst_ptr[n + 2] = tmp2 + (vtmp2[0] + vtmp2[1] + vtmp2[2] + vtmp2[3]);
            dst_ptr[n + 3] = tmp3 + (vtmp3[0] + vtmp3[1] + vtmp3[2] + vtmp3[3]);
        }

        for (; n < N; n++) {
            const float* src10_ptr = src1_data + (n + 0) * stride1;
            float tmp = 0.f;
            for (int k = 0; k < K; k++) {
                tmp += src0_ptr[k] * src10_ptr[k];
            }
            dst_ptr[n] = tmp;
        }
    }
}
#endif

Mat matrix_mul(const Mat& src0, const Mat& src1) {
    if (src0.empty() || src1.empty()) {
        LOG_ERR("The input data is empty!");
        return Mat();
    }

    if (src0.width() != src1.height()) {
        LOG_ERR("The width of src0 should be the same with the height of src1!");
        return Mat();
    }

    if (src0.channels() != src1.channels()
             || src0.type() != src1.type()) {
        LOG_ERR("The channel and data type of matrix_mul should be the same!");
        return Mat();
    }

    if ((src0.type() == FCVImageType::GRAY_F32)) {
        Mat dst(src1.width(), src0.height(), src0.type());

#ifdef HAVE_OPENCL
        if (ocl::isUseOpenCL()) {
            int ret = matrix_mul_opencl(src0, src1, dst);
            if (ret == 0) {
                return dst;
            }
        }
#endif

#ifdef HAVE_NEON
        matrix_multiply_f32_neon(src0, src1, dst);
#else
        matrix_multiply_conmmon<float>(src0, src1, dst);
#endif
        return dst;
    } else if (src0.type() == FCVImageType::GRAY_F64) {
        Mat dst(src1.width(), src0.height(), src0.type());
        matrix_multiply_conmmon<double>(src0, src1, dst);
        return dst;
    } else {
        LOG_ERR("The src type is not supported!");
        return Mat();
    }
}

G_FCV_NAMESPACE1_END()
