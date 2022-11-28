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

#include "modules/fusion_api/normalize_to_submean_to_reorder/include/normalize_to_submean_to_reorder_arm.h"

#include <immintrin.h>

#include "modules/core/base/include/common_avx.h"
#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

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
static int normalize_permute_8u3c_avx(
        const uint8_t* src_hwc_8u3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_chw_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    constexpr const int CHANNELS = 3;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    __m256 sub_factor_0 = _mm256_set1_ps(mean_param[0]);
    __m256 sub_factor_1 = _mm256_set1_ps(mean_param[1]);
    __m256 sub_factor_2 = _mm256_set1_ps(mean_param[2]);

    __m256 mult_factor_0 = _mm256_set1_ps(mult_param[0]);
    __m256 mult_factor_1 = _mm256_set1_ps(mult_param[1]);
    __m256 mult_factor_2 = _mm256_set1_ps(mult_param[2]);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    __m256 tmp_product_f32[3];
    __m256& dst_product_f32_0 = tmp_product_f32[index[0]];
    __m256& dst_product_f32_1 = tmp_product_f32[index[1]];
    __m256& dst_product_f32_2 = tmp_product_f32[index[2]];

    constexpr const int STEP_SIZE = 16; // Calculate 8 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    const uint8_t* ptr_cur_src = src_hwc_8u3c_data;
    float* ptr_cur_dst_c0 = dst_chw_32f3c_data;
    float* ptr_cur_dst_c1 = ptr_cur_dst_c0 + TOTAL_PIXEL_NUM;
    float* ptr_cur_dst_c2 = ptr_cur_dst_c1 + TOTAL_PIXEL_NUM;
    for (int i = 0; i < LOOP_CNT; ++i) {
        __m128i v_src_b_u8;
        __m128i v_src_g_u8;
        __m128i v_src_r_u8;
        vld3_u8x16_avx(ptr_cur_src, &v_src_b_u8, &v_src_g_u8, &v_src_r_u8);
        // u8 to s32
        __m256i v_src_b_l_s32 = _mm256_cvtepu8_epi32(v_src_b_u8);
        __m256i v_src_b_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_b_u8, 0b01001110));

        __m256i v_src_g_l_s32 = _mm256_cvtepu8_epi32(v_src_g_u8);
        __m256i v_src_g_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_g_u8, 0b01001110));

        __m256i v_src_r_l_s32 = _mm256_cvtepu8_epi32(v_src_r_u8);
        __m256i v_src_r_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_r_u8, 0b01001110));
        // s32 to f32
        __m256 src_c0_f32_l = _mm256_cvtepi32_ps(v_src_b_l_s32);
        __m256 src_c0_f32_h = _mm256_cvtepi32_ps(v_src_b_h_s32);

        __m256 src_c1_f32_l = _mm256_cvtepi32_ps(v_src_g_l_s32);
        __m256 src_c1_f32_h = _mm256_cvtepi32_ps(v_src_g_h_s32);

        __m256 src_c2_f32_l = _mm256_cvtepi32_ps(v_src_r_l_s32);
        __m256 src_c2_f32_h = _mm256_cvtepi32_ps(v_src_r_h_s32);
        // process low half data
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0, _mm256_sub_ps(src_c0_f32_l, sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1, _mm256_sub_ps(src_c1_f32_l, sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2, _mm256_sub_ps(src_c2_f32_l, sub_factor_2));

        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c0, _mm256_castps_si256(dst_product_f32_0));
        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c1, _mm256_castps_si256(dst_product_f32_1));
        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c2, _mm256_castps_si256(dst_product_f32_2));
        // process high half data
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0, _mm256_sub_ps(src_c0_f32_h, sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1, _mm256_sub_ps(src_c1_f32_h, sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2, _mm256_sub_ps(src_c2_f32_h, sub_factor_2));

        _mm256_storeu_si256((__m256i*)(ptr_cur_dst_c0 + 8), _mm256_castps_si256(dst_product_f32_0));
        _mm256_storeu_si256((__m256i*)(ptr_cur_dst_c1 + 8), _mm256_castps_si256(dst_product_f32_1));
        _mm256_storeu_si256((__m256i*)(ptr_cur_dst_c2 + 8), _mm256_castps_si256(dst_product_f32_2));

        ptr_cur_src += STEP_SIZE * CHANNELS;
        ptr_cur_dst_c0 += STEP_SIZE;
        ptr_cur_dst_c1 += STEP_SIZE;
        ptr_cur_dst_c2 += STEP_SIZE;
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
static int normalize_8u3c_avx(
        const uint8_t* src_8u3c_data,
        const int w,
        const int h,
        const float mean_param[],
        const float std_param[],
        const uint32_t channel_reorder_index[],
        float dst_32f3c_data[]) {
    // Calculate the number of all pixels.
    const int TOTAL_PIXEL_NUM = w * h;
    constexpr const int CHANNELS = 3;
    // Convert the division operation to multiplication for neon optimization later.
    const float mult_param[3] = {1.0f / std_param[0], 1.0f / std_param[1], 1.0f / std_param[2]};
    // Load constant parameters to registers.
    __m256 sub_factor_0 = _mm256_set1_ps(mean_param[0]);
    __m256 sub_factor_1 = _mm256_set1_ps(mean_param[1]);
    __m256 sub_factor_2 = _mm256_set1_ps(mean_param[2]);

    __m256 mult_factor_0 = _mm256_set1_ps(mult_param[0]);
    __m256 mult_factor_1 = _mm256_set1_ps(mult_param[1]);
    __m256 mult_factor_2 = _mm256_set1_ps(mult_param[2]);
    // Caculate each channel base address of dst data.
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    __m256 tmp_product_f32[3];
    __m256& dst_product_f32_0 = tmp_product_f32[index[0]];
    __m256& dst_product_f32_1 = tmp_product_f32[index[1]];
    __m256& dst_product_f32_2 = tmp_product_f32[index[2]];

    constexpr const int STEP_SIZE = 16; // Calculate 16 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    Vst3_F32x8_Avx store_tool;

    const uint8_t* ptr_src = src_8u3c_data;
    float* ptr_dst = dst_32f3c_data;
    for (int i = 0; i < LOOP_CNT; ++i) {
        __m128i v_src_b_u8;
        __m128i v_src_g_u8;
        __m128i v_src_r_u8;
        vld3_u8x16_avx(ptr_src, &v_src_b_u8, &v_src_g_u8, &v_src_r_u8);
        // u8 to s32
        __m256i v_src_b_l_s32 = _mm256_cvtepu8_epi32(v_src_b_u8);
        __m256i v_src_b_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_b_u8, 0b01001110));

        __m256i v_src_g_l_s32 = _mm256_cvtepu8_epi32(v_src_g_u8);
        __m256i v_src_g_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_g_u8, 0b01001110));

        __m256i v_src_r_l_s32 = _mm256_cvtepu8_epi32(v_src_r_u8);
        __m256i v_src_r_h_s32 = _mm256_cvtepu8_epi32(_mm_shuffle_epi32(v_src_r_u8, 0b01001110));
        // s32 to f32
        __m256 src_c0_f32_l = _mm256_cvtepi32_ps(v_src_b_l_s32);
        __m256 src_c0_f32_h = _mm256_cvtepi32_ps(v_src_b_h_s32);

        __m256 src_c1_f32_l = _mm256_cvtepi32_ps(v_src_g_l_s32);
        __m256 src_c1_f32_h = _mm256_cvtepi32_ps(v_src_g_h_s32);

        __m256 src_c2_f32_l = _mm256_cvtepi32_ps(v_src_r_l_s32);
        __m256 src_c2_f32_h = _mm256_cvtepi32_ps(v_src_r_h_s32);
        // calculate
        // process low half data
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0, _mm256_sub_ps(src_c0_f32_l, sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1, _mm256_sub_ps(src_c1_f32_l, sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2, _mm256_sub_ps(src_c2_f32_l, sub_factor_2));

        store_tool.store(ptr_dst, dst_product_f32_0, dst_product_f32_1, dst_product_f32_2);
        // process high half data
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0, _mm256_sub_ps(src_c0_f32_h, sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1, _mm256_sub_ps(src_c1_f32_h, sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2, _mm256_sub_ps(src_c2_f32_h, sub_factor_2));

        store_tool.store(ptr_dst + 24, dst_product_f32_0, dst_product_f32_1, dst_product_f32_2);

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
static int normalize_permute_32f3c_avx(
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
    __m256 sub_factor_0 = _mm256_set1_ps(mean_param[0]);
    __m256 sub_factor_1 = _mm256_set1_ps(mean_param[1]);
    __m256 sub_factor_2 = _mm256_set1_ps(mean_param[2]);

    __m256 mult_factor_0 = _mm256_set1_ps(mult_param[0]);
    __m256 mult_factor_1 = _mm256_set1_ps(mult_param[1]);
    __m256 mult_factor_2 = _mm256_set1_ps(mult_param[2]);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    __m256 tmp_product_f32[3];
    __m256& dst_product_f32_0 = tmp_product_f32[index[0]];
    __m256& dst_product_f32_1 = tmp_product_f32[index[1]];
    __m256& dst_product_f32_2 = tmp_product_f32[index[2]];

    constexpr const int STEP_SIZE = 8; // Calculate 8 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    Vld3_F32x8_Avx load_tool;

    const float* ptr_cur_src = src_hwc_32f3c_data;
    float* ptr_cur_dst_c0 = dst_chw_32f3c_data;
    float* ptr_cur_dst_c1 = ptr_cur_dst_c0 + TOTAL_PIXEL_NUM;
    float* ptr_cur_dst_c2 = ptr_cur_dst_c1 + TOTAL_PIXEL_NUM;
    for (int i = 0; i < LOOP_CNT; ++i) {
        __m256i vec_src_c0;
        __m256i vec_src_c1;
        __m256i vec_src_c2;
        load_tool.load(ptr_cur_src, &vec_src_c0, &vec_src_c1, &vec_src_c2);
        // calculate
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c0), sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c1), sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c2), sub_factor_2));
        // save result data
        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c0, _mm256_castps_si256(dst_product_f32_0));
        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c1, _mm256_castps_si256(dst_product_f32_1));
        _mm256_storeu_si256((__m256i*)ptr_cur_dst_c2, _mm256_castps_si256(dst_product_f32_2));

        ptr_cur_src += STEP_SIZE * 3;
        ptr_cur_dst_c0 += STEP_SIZE;
        ptr_cur_dst_c1 += STEP_SIZE;
        ptr_cur_dst_c2 += STEP_SIZE;
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
static int normalize_32f3c_avx(
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
    __m256 sub_factor_0 = _mm256_set1_ps(mean_param[0]);
    __m256 sub_factor_1 = _mm256_set1_ps(mean_param[1]);
    __m256 sub_factor_2 = _mm256_set1_ps(mean_param[2]);

    __m256 mult_factor_0 = _mm256_set1_ps(mult_param[0]);
    __m256 mult_factor_1 = _mm256_set1_ps(mult_param[1]);
    __m256 mult_factor_2 = _mm256_set1_ps(mult_param[2]);
    // Set the order of registers used to store the results if needed.
    const uint32_t NATURAL_ORDER[3] = {0, 1, 2};
    const uint32_t* index = nullptr;
    if (channel_reorder_index == nullptr) {
        index = NATURAL_ORDER;
    } else {
        index = channel_reorder_index;
    }
    __m256 tmp_product_f32[3];
    __m256& dst_product_f32_0 = tmp_product_f32[index[0]];
    __m256& dst_product_f32_1 = tmp_product_f32[index[1]];
    __m256& dst_product_f32_2 = tmp_product_f32[index[2]];

    constexpr const int STEP_SIZE = 8; // Calculate 8 pixel data per cycle.
    int LOOP_CNT = TOTAL_PIXEL_NUM / STEP_SIZE; // Number of Cycles.

    Vld3_F32x8_Avx load_tool;
    Vst3_F32x8_Avx store_tool;

    // Caculate each channel base address of dst data.
    const float* ptr_src = src_32f3c_data;
    float* ptr_dst = dst_32f3c_data;
    for (int i = 0; i < LOOP_CNT; ++i) {
        __m256i vec_src_c0;
        __m256i vec_src_c1;
        __m256i vec_src_c2;
        load_tool.load(ptr_src, &vec_src_c0, &vec_src_c1, &vec_src_c2);
        // calculate
        tmp_product_f32[0] = _mm256_mul_ps(mult_factor_0,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c0), sub_factor_0));
        tmp_product_f32[1] = _mm256_mul_ps(mult_factor_1,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c1), sub_factor_1));
        tmp_product_f32[2] = _mm256_mul_ps(mult_factor_2,
                _mm256_sub_ps(_mm256_castsi256_ps(vec_src_c2), sub_factor_2));
        // save result data
        store_tool.store(ptr_dst, dst_product_f32_0, dst_product_f32_1, dst_product_f32_2);

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

int normalize_to_submean_to_reorder_avx(
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
            res = normalize_8u3c_avx(reinterpret_cast<const uint8_t*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        } else {
            res = normalize_permute_8u3c_avx(reinterpret_cast<const uint8_t*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        }
        break;
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        if (output_package) {
            res = normalize_32f3c_avx(reinterpret_cast<const float*>(src.data()),
                    src.width(), src.height(), mean_params.data(), std_params.data(),
                    ptr_order, reinterpret_cast<float*>(dst.data()));
        } else {
            res = normalize_permute_32f3c_avx(reinterpret_cast<const float*>(src.data()),
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