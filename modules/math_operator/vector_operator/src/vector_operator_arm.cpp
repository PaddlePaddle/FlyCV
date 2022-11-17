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

#include <cmath>
#include "modules/math_operator/vector_operator/interface/vector_operator.h"
#include "modules/math_operator/vector_operator/include/vector_operator_arm.h"
#include "modules/math_operator/vector_operator/include/vector_operator_common.h"

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

float get_l2_neon(int dimension, float* vector) {
    float nrm = 0.0;
    float32x4_t x4_sum = vdupq_n_f32(0.0f);
    int idx = 0;
    for (; idx + 4 <= dimension; idx += 4) {
        x4_sum = vmlaq_f32(x4_sum, vld1q_f32(vector + idx), vld1q_f32(vector + idx));
    }
    float32x2_t _ss = vadd_f32(vget_high_f32(x4_sum), vget_low_f32(x4_sum));
    nrm += vget_lane_f32(vpadd_f32(_ss, _ss), 0);
    for (; idx < dimension; idx++) {
        nrm += vector[idx] * vector[idx];
    }
    nrm = sqrtf(nrm);
    return nrm;
}

void scaler_neon(
        int dimension,
        float *vector1,
        float scaler,
        int8_t *vector2) {
    int idx = 0;
    for (; idx + 8 <= dimension; idx += 8) {
        float32x4_t _r0 = vmulq_n_f32(vld1q_f32(vector1 + idx), scaler);
        float32x4_t _l0 = vmulq_n_f32(vld1q_f32(vector1 + idx + 4), scaler);
        int32x4_t _r1 = vcvtq_s32_f32(_r0);
        int32x4_t _l1 = vcvtq_s32_f32(_l0);
        int16x4_t _r2 = vqmovn_s32(_r1);
        int16x4_t _l2 = vqmovn_s32(_l1);
        int8x8_t _s0 = vqmovn_s16(vcombine_s16(_r2, _l2));
        vst1_s8(vector2 + idx, _s0);
    }
    for (;idx < dimension; idx++) {
        int32_t temp = static_cast<int32_t> (vector1[idx] * scaler);
        if (temp < 0) {
            vector2[idx] = fcv_cast_s8(temp < SCHAR_MIN ? SCHAR_MIN : temp);
        } else {
            vector2[idx] = fcv_cast_s8(temp > SCHAR_MAX ? SCHAR_MAX : temp);
        }
    }
}

// TODO: dimension % 4 != 0
void normalize_vector_neon(int dimension, float* vector) {
    float nrm = get_l2(dimension, vector);
    if (nrm <= CALC_EPSINON) {
        vector[0] = 1.0;
        return;
    }
    float dnrm = 1.0 / nrm;

    float32x4_t _nrm = vdupq_n_f32(dnrm);
    int idx = 0;
    for (; idx + 4 <= dimension; idx += 4) {
        vst1q_f32(vector + idx, vmulq_f32(vld1q_f32(vector + idx), _nrm));
    }

    for (; idx < dimension; idx++) {
        vector[idx] *= dnrm;
    }
    return;
}

// TODO: dimension % 4 != 0
void dot_vectors_neon(
        int dimension,
        float *v0,
        float *v1,
        float *dot) {
    float sum = 0.0;
    float32x4_t x4_sum = vdupq_n_f32(0.0f);
    int idx = 0;
    for (; idx + 4 <= dimension; idx += 4) {
        // x4_sum = x4_v0[idx] * x4_v1[idx] + x4_sum or vfmaq_f32
        x4_sum = vmlaq_f32(x4_sum, vld1q_f32(v0 + idx), vld1q_f32(v1 + idx));
    }
    // TODO: *sum = vaddvq_f32(x4_sum) only surport a64
    float32x2_t _ss = vadd_f32(vget_high_f32(x4_sum), vget_low_f32(x4_sum));
    sum += vget_lane_f32(vpadd_f32(_ss, _ss), 0);
    for (; idx < dimension; idx++) {
        sum += v0[idx] * v1[idx];
    }
    *dot = sum;
}

void dot_vectors_neon(
        int dimension,
        int8_t* v0,
        int8_t* v1,
        int32_t* dot) {
    int32_t sum = 0;
    int32x4_t x4_sum = vdupq_n_s32(0);
    int idx = 0;
#ifdef SUPPORT_VDOT
    for (; idx + 16 <= dimension; idx += 16) {
        // x4_sum = x4_v0[idx] * x4_v1[idx] + x4_sum or vfmaq_f32
        int32x4_t x4_ss = vdupq_n_s32(0);
        vdotq_s32(x4_ss, vld1q_s8(v0 + idx), vld1q_s8(v1 + idx));
        x4_sum = vaddq_s32(x4_sum, x4_ss);
    }
#else
    for (; idx + 8 <= dimension; idx += 8) {
        int16x8_t _r0 = vmull_s8(vld1_s8(v0 + idx), vld1_s8(v1 + idx));
        x4_sum = vaddw_s16(x4_sum, vget_low_s16(_r0));
        x4_sum = vaddw_s16(x4_sum, vget_high_s16(_r0));
    }
#endif
    // TODO: *sum = vaddvq_f32(x4_sum) only surport a64
    int32x2_t x2_ss = vadd_s32(vget_high_s32(x4_sum), vget_low_s32(x4_sum));
    sum += vget_lane_s32(vpadd_s32(x2_ss, x2_ss), 0);
    for (; idx < dimension; idx++) {
        sum += v0[idx] * v1[idx];
    }
    *dot = sum;
}

// TODO: dimension % 4 != 0
void axpy_vector_neon(
        int dimension,
        float scalar,
        float* vector_x,
        float* vector_y) {
    float32x4_t _scalar = vdupq_n_f32(scalar);
    int idx = 0;
    for (; idx + 4 <= dimension; idx += 4) {
        vst1q_f32(vector_y + idx, vmlaq_f32(vld1q_f32(vector_y + idx),
                _scalar, vld1q_f32(vector_x + idx)));
    }
    for (; idx < dimension; idx++) {
        vector_y[idx] += scalar * vector_x[idx];
    }
}

// TODO
void axpy_vector_neon(
        int dimension,
        float scalar,
        int8_t* vector_x,
        float* vector_y) {
    int idx = 0;
    for (; idx + 16 <= dimension; idx += 16) {
        int8x16_t _x0 = vld1q_s8(vector_x + idx);
        int16x8_t _s0 = vmovl_s8(vget_low_s8(_x0));
        int16x8_t _s1 = vmovl_s8(vget_high_s8(_x0));
        int32x4_t _l0 = vmovl_s16(vget_low_s16(_s0));
        int32x4_t _l1 = vmovl_s16(vget_high_s16(_s0));
        int32x4_t _l2 = vmovl_s16(vget_low_s16(_s1));
        int32x4_t _l3 = vmovl_s16(vget_high_s16(_s1));

        float32x4_t _r0 = vld1q_f32(vector_y + idx + 0);
        float32x4_t _r1 = vld1q_f32(vector_y + idx + 4);
        float32x4_t _r2 = vld1q_f32(vector_y + idx + 8);
        float32x4_t _r3 = vld1q_f32(vector_y + idx + 12);

        vst1q_f32(&vector_y[idx + 0], vmlaq_n_f32(_r0, vcvtq_f32_s32(_l0), (float32_t)scalar));
        vst1q_f32(&vector_y[idx + 4], vmlaq_n_f32(_r1, vcvtq_f32_s32(_l1), (float32_t)scalar));
        vst1q_f32(&vector_y[idx + 8], vmlaq_n_f32(_r2, vcvtq_f32_s32(_l2), (float32_t)scalar));
        vst1q_f32(&vector_y[idx + 12], vmlaq_n_f32(_r3, vcvtq_f32_s32(_l3), (float32_t)scalar));
    }

    for (; idx < dimension; idx++) {
        vector_y[idx] += scalar * vector_x[idx];
    }
}

G_FCV_NAMESPACE1_END()
