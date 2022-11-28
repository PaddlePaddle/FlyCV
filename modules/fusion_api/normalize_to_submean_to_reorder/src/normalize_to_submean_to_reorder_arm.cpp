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

#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_arm.h"

#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

// class NormPermute8uC3NeonParallelTask : public ParallelTask {
// public:
//     NormPermute8uC3NeonParallelTask (
//             const uint8_t* ptr_src,
//             float* ptr_dst_c0,
//             float* ptr_dst_c1,
//             float* ptr_dst_c2,
//             const float* mean_param,
//             const float* std_param,
//             const uint32_t* index)
//             : _ptr_src(ptr_src),
//             _ptr_dst_c0(ptr_dst_c0),
//             _ptr_dst_c1(ptr_dst_c1),
//             _ptr_dst_c2(ptr_dst_c2),
//             _mean_param(mean_param),
//             _std_param(std_param),
//             _index(index) {}

//     virtual void operator()(const Range& range) const {
//         const uint8_t* ptr_cur_src = _ptr_src + 24 * range.start();
//         float* ptr_cur_dst_c0 = _ptr_dst_c0 + 8 * range.start();
//         float* ptr_cur_dst_c1 = _ptr_dst_c1 + 8 * range.start();
//         float* ptr_cur_dst_c2 = _ptr_dst_c2 + 8 * range.start();

//         float32x4_t dst_register_f32[3][2];
//         float32x4_t* ptr_reorder_register_c[3] = {nullptr};
//         ptr_reorder_register_c[0] = dst_register_f32[_index[0]];
//         ptr_reorder_register_c[1] = dst_register_f32[_index[1]];
//         ptr_reorder_register_c[2] = dst_register_f32[_index[2]];

//         const float32x4_t sub_factor_0 = vld1q_dup_f32(_mean_param);
//         const float32x4_t sub_factor_1 = vld1q_dup_f32(_mean_param + 1);
//         const float32x4_t sub_factor_2 = vld1q_dup_f32(_mean_param + 2);

//         const float32x4_t mult_factor_0 = vld1q_dup_f32(_std_param);
//         const float32x4_t mult_factor_1 = vld1q_dup_f32(_std_param + 1);
//         const float32x4_t mult_factor_2 = vld1q_dup_f32(_std_param + 2);

//         for (int i = range.start(); i < range.end(); ++i) {
//         //for (int i = 0; i < LOOP_CNT; ++i) {
//             uint8x8x3_t v_src_data_u8 = vld3_u8(ptr_cur_src);
//             // u8 to u16
//             uint16x8_t src_c0_u16 = vmovl_u8(v_src_data_u8.val[0]);
//             uint16x8_t src_c1_u16 = vmovl_u8(v_src_data_u8.val[1]);
//             uint16x8_t src_c2_u16 = vmovl_u8(v_src_data_u8.val[2]);
//             // u16 to u32
//             uint32x4_t src_c0_u32_l = vmovl_u16(vget_low_u16(src_c0_u16));
//             uint32x4_t src_c0_u32_h = vmovl_u16(vget_high_u16(src_c0_u16));

//             uint32x4_t src_c1_u32_l = vmovl_u16(vget_low_u16(src_c1_u16));
//             uint32x4_t src_c1_u32_h = vmovl_u16(vget_high_u16(src_c1_u16));

//             uint32x4_t src_c2_u32_l = vmovl_u16(vget_low_u16(src_c2_u16));
//             uint32x4_t src_c2_u32_h = vmovl_u16(vget_high_u16(src_c2_u16));
//             // u32 to f32
//             float32x4_t src_c0_f32_l = vcvtq_f32_u32(src_c0_u32_l);
//             float32x4_t src_c0_f32_h = vcvtq_f32_u32(src_c0_u32_h);

//             float32x4_t src_c1_f32_l = vcvtq_f32_u32(src_c1_u32_l);
//             float32x4_t src_c1_f32_h = vcvtq_f32_u32(src_c1_u32_h);

//             float32x4_t src_c2_f32_l = vcvtq_f32_u32(src_c2_u32_l);
//             float32x4_t src_c2_f32_h = vcvtq_f32_u32(src_c2_u32_h);
//             // calculate
//             dst_register_f32[0][0] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_l, sub_factor_0));
//             dst_register_f32[0][1] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_h, sub_factor_0));

//             dst_register_f32[1][0] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_l, sub_factor_1));
//             dst_register_f32[1][1] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_h, sub_factor_1));

//             dst_register_f32[2][0] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_l, sub_factor_2));
//             dst_register_f32[2][1] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_h, sub_factor_2));

//             // save result data
//             vst1q_f32(ptr_cur_dst_c0, ptr_reorder_register_c[0][0]);
//             vst1q_f32(ptr_cur_dst_c0 + 4, ptr_reorder_register_c[0][1]);

//             vst1q_f32(ptr_cur_dst_c1, ptr_reorder_register_c[1][0]);
//             vst1q_f32(ptr_cur_dst_c1 + 4, ptr_reorder_register_c[1][1]);

//             vst1q_f32(ptr_cur_dst_c2, ptr_reorder_register_c[2][0]);
//             vst1q_f32(ptr_cur_dst_c2 + 4, ptr_reorder_register_c[2][1]);

//             ptr_cur_src += 24;
//             ptr_cur_dst_c0 += 8;
//             ptr_cur_dst_c1 += 8;
//             ptr_cur_dst_c2 += 8;
//         }
//     }

// private:
//     const uint8_t* _ptr_src;
//     float* _ptr_dst_c0;
//     float* _ptr_dst_c1;
//     float* _ptr_dst_c2;
//     const float* _mean_param;
//     const float* _std_param;
//     const uint32_t* _index;
// };

/**
 * @note: Must allocate enough memory for the following parameters before call the function
 * @param src_hwc_8u3c_data   :   w * h * 3 * sizeof(uint8_t)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 float-type data array
 * @param std_param             : 3 float-type data array
 * @param channel_reorder_index : nullptr or uint32_t array whose element must be one of 0, 1, 2.
 * @param dst_chw_32f3c_data  : w * h * 3 * sizeof(float)
 */
static int normalize_permute_8u3c_neon(
        const uint8_t* src_hwc_8u3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_chw_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    //const int CHANNELS = 3;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    const float32x4_t sub_factor_0 = vld1q_dup_f32(mean_param);
    const float32x4_t sub_factor_1 = vld1q_dup_f32(mean_param + 1);
    const float32x4_t sub_factor_2 = vld1q_dup_f32(mean_param + 2);

    const float32x4_t mult_factor_0 = vld1q_dup_f32(mult_param);
    const float32x4_t mult_factor_1 = vld1q_dup_f32(mult_param + 1);
    const float32x4_t mult_factor_2 = vld1q_dup_f32(mult_param + 2);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    float32x4_t dst_register_f32[3][2];
    float32x4_t* ptr_reorder_register_c[3] = {nullptr};
    ptr_reorder_register_c[0] = dst_register_f32[index[0]];
    ptr_reorder_register_c[1] = dst_register_f32[index[1]];
    ptr_reorder_register_c[2] = dst_register_f32[index[2]];

    constexpr const int STEP_SIZE = 8; // Calculate 8 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    const uint8_t* ptr_cur_src = src_hwc_8u3c_data;
    float* ptr_cur_dst_c0 = dst_chw_32f3c_data;
    float* ptr_cur_dst_c1 = ptr_cur_dst_c0 + TOTAL_PIXEL_NUM;
    float* ptr_cur_dst_c2 = ptr_cur_dst_c1 + TOTAL_PIXEL_NUM;

    // NormPermute8uC3NeonParallelTask parallel_task(ptr_cur_src,
    //         ptr_cur_dst_c0, ptr_cur_dst_c1, ptr_cur_dst_c2, mean_param, mult_param, index);
    // parallel_run(Range(0, LOOP_CNT), parallel_task);

    for (int i = 0; i < LOOP_CNT; ++i) {
       uint8x8x3_t v_src_data_u8 = vld3_u8(ptr_cur_src);
       // u8 to u16
       uint16x8_t src_c0_u16 = vmovl_u8(v_src_data_u8.val[0]);
       uint16x8_t src_c1_u16 = vmovl_u8(v_src_data_u8.val[1]);
       uint16x8_t src_c2_u16 = vmovl_u8(v_src_data_u8.val[2]);
       // u16 to u32
       uint32x4_t src_c0_u32_l = vmovl_u16(vget_low_u16(src_c0_u16));
       uint32x4_t src_c0_u32_h = vmovl_u16(vget_high_u16(src_c0_u16));

       uint32x4_t src_c1_u32_l = vmovl_u16(vget_low_u16(src_c1_u16));
       uint32x4_t src_c1_u32_h = vmovl_u16(vget_high_u16(src_c1_u16));

       uint32x4_t src_c2_u32_l = vmovl_u16(vget_low_u16(src_c2_u16));
       uint32x4_t src_c2_u32_h = vmovl_u16(vget_high_u16(src_c2_u16));
       // u32 to f32
       float32x4_t src_c0_f32_l = vcvtq_f32_u32(src_c0_u32_l);
       float32x4_t src_c0_f32_h = vcvtq_f32_u32(src_c0_u32_h);

       float32x4_t src_c1_f32_l = vcvtq_f32_u32(src_c1_u32_l);
       float32x4_t src_c1_f32_h = vcvtq_f32_u32(src_c1_u32_h);

       float32x4_t src_c2_f32_l = vcvtq_f32_u32(src_c2_u32_l);
       float32x4_t src_c2_f32_h = vcvtq_f32_u32(src_c2_u32_h);
       // calculate
       dst_register_f32[0][0] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_l, sub_factor_0));
       dst_register_f32[0][1] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_h, sub_factor_0));

       dst_register_f32[1][0] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_l, sub_factor_1));
       dst_register_f32[1][1] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_h, sub_factor_1));

       dst_register_f32[2][0] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_l, sub_factor_2));
       dst_register_f32[2][1] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_h, sub_factor_2));

       // save result data
       vst1q_f32(ptr_cur_dst_c0, ptr_reorder_register_c[0][0]);
       vst1q_f32(ptr_cur_dst_c0 + 4, ptr_reorder_register_c[0][1]);

       vst1q_f32(ptr_cur_dst_c1, ptr_reorder_register_c[1][0]);
       vst1q_f32(ptr_cur_dst_c1 + 4, ptr_reorder_register_c[1][1]);

       vst1q_f32(ptr_cur_dst_c2, ptr_reorder_register_c[2][0]);
       vst1q_f32(ptr_cur_dst_c2 + 4, ptr_reorder_register_c[2][1]);

       ptr_cur_src += 24;
       ptr_cur_dst_c0 += 8;
       ptr_cur_dst_c1 += 8;
       ptr_cur_dst_c2 += 8;
    }

    // Calculate the remain pixel.
    const int REMAINING_PIXELS = TOTAL_PIXEL_NUM % STEP_SIZE; // Number of remaining pixels.

    for (int i = 0; i < REMAINING_PIXELS; ++i) {
        float tmp_data[3];
        tmp_data[0] = (float(ptr_cur_src[0]) - mean_param[0]) * mult_param[0];
        tmp_data[1] = (float(ptr_cur_src[1]) - mean_param[1]) * mult_param[1];
        tmp_data[2] = (float(ptr_cur_src[2]) - mean_param[2]) * mult_param[2];
        // reoreder channel
        *ptr_cur_dst_c0 = tmp_data[index[0]];
        *ptr_cur_dst_c1 = tmp_data[index[1]];
        *ptr_cur_dst_c2 = tmp_data[index[2]];

        ptr_cur_src += 3;
        ++ptr_cur_dst_c0;
        ++ptr_cur_dst_c1;
        ++ptr_cur_dst_c2;
    }

    return 0;
}

/**
 * @note: Must allocate enough memory for the following parameters before call the function
 * @param src_8u3c_data   :   w * h * 3 * sizeof(uint8_t)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 float-type data array
 * @param std_param             : 3 float-type data array
 * @param channel_reorder_index : nullptr or uint32_t array whose element must be one of 0, 1, 2.
 * @param dst_32f3c_data  : w * h * 3 * sizeof(float)
 */
static int normalize_8u3c_neon(
        const uint8_t* src_8u3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    const int CHANNELS = 3;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    const float32x4_t sub_factor_0 = vld1q_dup_f32(mean_param);
    const float32x4_t sub_factor_1 = vld1q_dup_f32(mean_param + 1);
    const float32x4_t sub_factor_2 = vld1q_dup_f32(mean_param + 2);

    const float32x4_t mult_factor_0 = vld1q_dup_f32(mult_param);
    const float32x4_t mult_factor_1 = vld1q_dup_f32(mult_param + 1);
    const float32x4_t mult_factor_2 = vld1q_dup_f32(mult_param + 2);
    // Caculate each channel base address of dst data.
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }

    constexpr const int STEP_SIZE = 8; // Calculate 8 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    const uint8_t* ptr_src = src_8u3c_data;
    float* ptr_dst = dst_32f3c_data;
    for (int i = 0; i < LOOP_CNT; ++i) {
        uint8x8x3_t v_src_data_u8 = vld3_u8(ptr_src);
        // u8 to u16
        uint16x8_t src_c0_u16 = vmovl_u8(v_src_data_u8.val[0]);
        uint16x8_t src_c1_u16 = vmovl_u8(v_src_data_u8.val[1]);
        uint16x8_t src_c2_u16 = vmovl_u8(v_src_data_u8.val[2]);
        // u16 to u32
        uint32x4_t src_c0_u32_l = vmovl_u16(vget_low_u16(src_c0_u16));
        uint32x4_t src_c0_u32_h = vmovl_u16(vget_high_u16(src_c0_u16));

        uint32x4_t src_c1_u32_l = vmovl_u16(vget_low_u16(src_c1_u16));
        uint32x4_t src_c1_u32_h = vmovl_u16(vget_high_u16(src_c1_u16));

        uint32x4_t src_c2_u32_l = vmovl_u16(vget_low_u16(src_c2_u16));
        uint32x4_t src_c2_u32_h = vmovl_u16(vget_high_u16(src_c2_u16));
        // u32 to f32
        float32x4_t src_c0_f32_l = vcvtq_f32_u32(src_c0_u32_l);
        float32x4_t src_c0_f32_h = vcvtq_f32_u32(src_c0_u32_h);

        float32x4_t src_c1_f32_l = vcvtq_f32_u32(src_c1_u32_l);
        float32x4_t src_c1_f32_h = vcvtq_f32_u32(src_c1_u32_h);

        float32x4_t src_c2_f32_l = vcvtq_f32_u32(src_c2_u32_l);
        float32x4_t src_c2_f32_h = vcvtq_f32_u32(src_c2_u32_h);
        // calculate
        float32x4x3_t tmp_f32_l;
        float32x4x3_t tmp_f32_h;

        tmp_f32_l.val[0] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_l, sub_factor_0));
        tmp_f32_h.val[0] = vmulq_f32(mult_factor_0, vsubq_f32(src_c0_f32_h, sub_factor_0));

        tmp_f32_l.val[1] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_l, sub_factor_1));
        tmp_f32_h.val[1] = vmulq_f32(mult_factor_1, vsubq_f32(src_c1_f32_h, sub_factor_1));

        tmp_f32_l.val[2] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_l, sub_factor_2));
        tmp_f32_h.val[2] = vmulq_f32(mult_factor_2, vsubq_f32(src_c2_f32_h, sub_factor_2));
        // Reorder
        float32x4x3_t dst_f32_l;
        float32x4x3_t dst_f32_h;

        dst_f32_l.val[0] = tmp_f32_l.val[index[0]];
        dst_f32_l.val[1] = tmp_f32_l.val[index[1]];
        dst_f32_l.val[2] = tmp_f32_l.val[index[2]];

        dst_f32_h.val[0] = tmp_f32_h.val[index[0]];
        dst_f32_h.val[1] = tmp_f32_h.val[index[1]];
        dst_f32_h.val[2] = tmp_f32_h.val[index[2]];

        vst3q_f32(ptr_dst, dst_f32_l);
        vst3q_f32(ptr_dst + 4 * 3, dst_f32_h); // float32 x 4 register, 3 channel

        ptr_src += STEP_SIZE * CHANNELS;
        ptr_dst += STEP_SIZE * CHANNELS;
    }
    // Calculate the remain pixel.
    const int REMAINING_PIXELS = TOTAL_PIXEL_NUM % STEP_SIZE; // Number of remaining pixels.
    for (int i = 0; i < REMAINING_PIXELS; ++i) {
        float tmp_data[3];
        tmp_data[0] = (float(ptr_src[0]) - mean_param[0]) * mult_param[0];
        tmp_data[1] = (float(ptr_src[1]) - mean_param[1]) * mult_param[1];
        tmp_data[2] = (float(ptr_src[2]) - mean_param[2]) * mult_param[2];
        // Reoreder
        ptr_dst[0] = tmp_data[index[0]];
        ptr_dst[1] = tmp_data[index[1]];
        ptr_dst[2] = tmp_data[index[2]];

        ptr_src += CHANNELS;
        ptr_dst += CHANNELS;
    }
    return 0;
}

/**
 * @note: Must allocate enough memory for the following parameters before call the function
 * @param src_hwc_32f3c_data   :   w * h * 3 * sizeof(float)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 float-type data array
 * @param std_param             : 3 float-type data array
 * @param channel_reorder_index : nullptr or uint32_t array whose element must be one of 0, 1, 2.
 * @param dst_chw_32f3c_data  : w * h * 3 * sizeof(float)
 */
static int normalize_permute_32f3c_neon(
        const float* src_hwc_32f3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_chw_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    const float32x4_t sub_factor_0 = vld1q_dup_f32(mean_param);
    const float32x4_t sub_factor_1 = vld1q_dup_f32(mean_param + 1);
    const float32x4_t sub_factor_2 = vld1q_dup_f32(mean_param + 2);

    const float32x4_t mult_factor_0 = vld1q_dup_f32(mult_param);
    const float32x4_t mult_factor_1 = vld1q_dup_f32(mult_param + 1);
    const float32x4_t mult_factor_2 = vld1q_dup_f32(mult_param + 2);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    float32x4_t dst_register_f32[3];
    float32x4_t* ptr_reorder_register_c[3] = {nullptr};
    ptr_reorder_register_c[0] = dst_register_f32 + index[0];
    ptr_reorder_register_c[1] = dst_register_f32 + index[1];
    ptr_reorder_register_c[2] = dst_register_f32 + index[2];

    constexpr const int STEP_SIZE = 4; // Calculate 4 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    const float* ptr_cur_src = src_hwc_32f3c_data;
    float* ptr_cur_dst_c0 = dst_chw_32f3c_data;
    float* ptr_cur_dst_c1 = ptr_cur_dst_c0 + TOTAL_PIXEL_NUM;
    float* ptr_cur_dst_c2 = ptr_cur_dst_c1 + TOTAL_PIXEL_NUM;
    for (int i = 0; i < LOOP_CNT; ++i) {
        float32x4x3_t v_src_data_f32 = vld3q_f32(ptr_cur_src);
        // calculate
        dst_register_f32[0] = vmulq_f32(mult_factor_0,
                vsubq_f32(v_src_data_f32.val[0], sub_factor_0));
        dst_register_f32[1] = vmulq_f32(mult_factor_1,
                vsubq_f32(v_src_data_f32.val[1], sub_factor_1));
        dst_register_f32[2] = vmulq_f32(mult_factor_2,
                vsubq_f32(v_src_data_f32.val[2], sub_factor_2));
        // save result data
        vst1q_f32(ptr_cur_dst_c0, *ptr_reorder_register_c[0]);
        vst1q_f32(ptr_cur_dst_c1, *ptr_reorder_register_c[1]);
        vst1q_f32(ptr_cur_dst_c2, *ptr_reorder_register_c[2]);

        ptr_cur_src += 12;
        ptr_cur_dst_c0 += 4;
        ptr_cur_dst_c1 += 4;
        ptr_cur_dst_c2 += 4;
    }
    // Calculate the remain pixel.
    const int REMAINING_PIXELS = TOTAL_PIXEL_NUM % STEP_SIZE; // Number of remaining pixels.
    for (int i = 0; i < REMAINING_PIXELS; ++i) {
        float tmp_data[3];
        tmp_data[0] = ptr_cur_src[0] - mean_param[0] * mult_param[0];
        tmp_data[1] = ptr_cur_src[1] - mean_param[1] * mult_param[1];
        tmp_data[2] = ptr_cur_src[2] - mean_param[2] * mult_param[2];
        // Reorder output channel
        *ptr_cur_dst_c0 = tmp_data[index[0]];
        *ptr_cur_dst_c1 = tmp_data[index[1]];
        *ptr_cur_dst_c2 = tmp_data[index[2]];

        ptr_cur_src += 3;
        ++ptr_cur_dst_c0;
        ++ptr_cur_dst_c1;
        ++ptr_cur_dst_c2;
    }
    return 0;
}

/**
 * @note: Must allocate enough memory for the following parameters before call the function
 * @param src_32f3c_data   :   w * h * 3 * sizeof(float)
 * @param w                     :   > 0
 * @param h                     :   > 0
 * @param mean_param            : 3 float-type data array
 * @param std_param             : 3 float-type data array
 * @param channel_reorder_index : nullptr or uint32_t array whose element must be one of 0, 1, 2.
 * @param dst_32f3c_data  : w * h * 3 * sizeof(float)
 */
static int normalize_32f3c_neon(
        const float* src_32f3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    const int CHANNELS = 3;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    const float32x4_t sub_factor_0 = vld1q_dup_f32(mean_param);
    const float32x4_t sub_factor_1 = vld1q_dup_f32(mean_param + 1);
    const float32x4_t sub_factor_2 = vld1q_dup_f32(mean_param + 2);

    const float32x4_t mult_factor_0 = vld1q_dup_f32(mult_param);
    const float32x4_t mult_factor_1 = vld1q_dup_f32(mult_param + 1);
    const float32x4_t mult_factor_2 = vld1q_dup_f32(mult_param + 2);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }

    constexpr const int STEP_SIZE = 4; // Calculate 4 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    // Caculate each channel base address of dst data.
    const float* ptr_src = src_32f3c_data;
    float* ptr_dst = dst_32f3c_data;
    for (int i = 0; i < LOOP_CNT; ++i) {
        float32x4x3_t v_src_data_f32 = vld3q_f32(ptr_src);
        // calculate
        float32x4x3_t v_tmp_data_f32;
        v_tmp_data_f32.val[0] = vmulq_f32(mult_factor_0,
                vsubq_f32(v_src_data_f32.val[0], sub_factor_0));
        v_tmp_data_f32.val[1] = vmulq_f32(mult_factor_1,
                vsubq_f32(v_src_data_f32.val[1], sub_factor_1));
        v_tmp_data_f32.val[2] = vmulq_f32(mult_factor_2,
                vsubq_f32(v_src_data_f32.val[2], sub_factor_2));
        // Reoreder
        float32x4x3_t v_dst_data_f32;
        v_dst_data_f32.val[0] = v_tmp_data_f32.val[index[0]];
        v_dst_data_f32.val[1] = v_tmp_data_f32.val[index[1]];
        v_dst_data_f32.val[2] = v_tmp_data_f32.val[index[2]];
        // save result data
        vst3q_f32(ptr_dst, v_dst_data_f32);

        ptr_src += CHANNELS * STEP_SIZE;
        ptr_dst += CHANNELS * STEP_SIZE;
    }
    // Calculate the remain pixel.
    const int REMAINING_PIXELS = TOTAL_PIXEL_NUM % STEP_SIZE; // Number of remaining pixels.
    for (int i = 0; i < REMAINING_PIXELS; ++i) {
        float tmp_data[3];
        tmp_data[0] = (ptr_src[0] - mean_param[0]) * mult_param[0];
        tmp_data[1] = (ptr_src[1] - mean_param[1]) * mult_param[1];
        tmp_data[2] = (ptr_src[2] - mean_param[2]) * mult_param[2];
        // Reorder
        ptr_dst[0] = tmp_data[index[0]];
        ptr_dst[1] = tmp_data[index[1]];
        ptr_dst[2] = tmp_data[index[2]];

        ptr_src += CHANNELS;
        ptr_dst += CHANNELS;
    }
    return 0;
}

int normalize_to_submean_to_reorder_neon(
        const Mat& src,
        const std::vector<float>& mean_params,
        const std::vector<float>& std_params,
        const std::vector<uint32_t>& channel_reorder_index,
        Mat& dst,
        bool output_package) {
    int res = -1;
    const uint32_t* ptr_order = nullptr;
    if (!channel_reorder_index.empty()
            && channel_reorder_index.size() == 3
            && channel_reorder_index[0] < 3
            && channel_reorder_index[1] < 3
            && channel_reorder_index[2] < 3) {
        ptr_order = channel_reorder_index.data();
    }

    switch (src.type()) {
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_U8:
        if (output_package) {
            res = normalize_8u3c_neon(reinterpret_cast<const unsigned char*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        } else {
            res = normalize_permute_8u3c_neon(reinterpret_cast<const unsigned char*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        }
        break;
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        if (output_package) {
            res = normalize_32f3c_neon(reinterpret_cast<const float*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        } else {
            res = normalize_permute_32f3c_neon(reinterpret_cast<const float*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        }
        break;
    default:
        LOG_ERR("The src type is not supported!");
        res = -1;
        break;
    }

    return res;
}

G_FCV_NAMESPACE1_END()
