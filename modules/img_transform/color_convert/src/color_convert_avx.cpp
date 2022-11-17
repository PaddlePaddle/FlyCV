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

#include "modules/img_transform/color_convert/include/color_convert_avx.h"

#include <immintrin.h>

#include "modules/core/base/include/common_avx.h"
#include "modules/core/base/include/macro_utils.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/color_convert/include/color_convert_sse.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief de-interleave common bgr package data to spilt for yuv conversion
 * @param[in] data b0,g0,r0, b1,g1,r1, ..., b31,g31,r31, data-type: uint8  
 * @param[out] b0 b0,b2,b4,...,b30, data-type: int16, format: __m256i
 * @param[out] b1 b1,b3,b5,...,b31, data-type: int16, format: __m256i
 * @param[out] g0 g0,g2,g4,...,g30, data-type: int16, format: __m256i
 * @param[out] g1 g1,g3,g5,...,g31, data-type: int16, format: __m256i
 * @param[out] r0 r0,r2,r4,...,r30, data-type: int16, format: __m256i
 * @param[out] r1 r1,r3,r5,...,r31, data-type: int16, format: __m256i
 */
inline void deinterleave_pa_bgr(
        const uint8_t* data, 
        __m256i* b0, 
        __m256i* b1, 
        __m256i* g0, 
        __m256i* g1, 
        __m256i* r0, 
        __m256i* r1 ) {
    __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);

    __m128i vec_b_low; 
    __m128i vec_g_low; 
    __m128i vec_r_low;
    vld3_u8x16_avx(data, &vec_b_low, &vec_g_low, &vec_r_low);
    __m128i vec_b_low_split = _mm_shuffle_epi8(vec_b_low, split_mask);
    __m128i vec_g_low_split = _mm_shuffle_epi8(vec_g_low, split_mask);
    __m128i vec_r_low_split = _mm_shuffle_epi8(vec_r_low, split_mask);

    __m128i vec_b_high; 
    __m128i vec_g_high; 
    __m128i vec_r_high;
    vld3_u8x16_avx(data + 48, &vec_b_high, &vec_g_high, &vec_r_high);
    __m128i vec_b_high_split = _mm_shuffle_epi8(vec_b_high, split_mask);
    __m128i vec_g_high_split = _mm_shuffle_epi8(vec_g_high, split_mask);
    __m128i vec_r_high_split = _mm_shuffle_epi8(vec_r_high, split_mask);

    *b0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_b_low_split, vec_b_high_split));
    *b1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_b_low_split, vec_b_high_split));

    *g0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_g_low_split, vec_g_high_split));
    *g1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_g_low_split, vec_g_high_split));

    *r0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_r_low_split, vec_r_high_split));
    *r1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_r_low_split, vec_r_high_split));
    return;
}

/**
 * @brief de-interleave common bgra package data to spilt for yuv conversion
 * @param[in] data b0,g0,r0,a0, b1,g1,r1,a1 ..., b31,g31,r31,a31 data-type: uint8  
 * @param[out] b0 b0,b2,b4,...,b30, data-type: int16, format: __m256i
 * @param[out] b1 b1,b3,b5,...,b31, data-type: int16, format: __m256i
 * @param[out] g0 g0,g2,g4,...,g30, data-type: int16, format: __m256i
 * @param[out] g1 g1,g3,g5,...,g31, data-type: int16, format: __m256i
 * @param[out] r0 r0,r2,r4,...,r30, data-type: int16, format: __m256i
 * @param[out] r1 r1,r3,r5,...,r31, data-type: int16, format: __m256i
 * @param[out] a0 a0,a2,a4,...,a30, data-type: int16, format: __m256i
 * @param[out] a1 a1,a3,a5,...,a31, data-type: int16, format: __m256i
 */
inline void deinterleave_pa_bgra(
        const uint8_t* data, 
        __m256i* b0, 
        __m256i* b1, 
        __m256i* g0, 
        __m256i* g1, 
        __m256i* r0, 
        __m256i* r1,
        __m256i* a0,
        __m256i* a1) {
    __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);

    __m128i vec_b_low; 
    __m128i vec_g_low; 
    __m128i vec_r_low;
    __m128i vec_a_low;
    vld4_u8x16_avx(data, &vec_b_low, &vec_g_low, &vec_r_low, &vec_a_low);
    __m128i vec_b_low_split = _mm_shuffle_epi8(vec_b_low, split_mask);
    __m128i vec_g_low_split = _mm_shuffle_epi8(vec_g_low, split_mask);
    __m128i vec_r_low_split = _mm_shuffle_epi8(vec_r_low, split_mask);
    __m128i vec_a_low_split = _mm_shuffle_epi8(vec_a_low, split_mask);

    __m128i vec_b_high; 
    __m128i vec_g_high; 
    __m128i vec_r_high;
    __m128i vec_a_high;
    vld4_u8x16_avx(data + 64, &vec_b_high, &vec_g_high, &vec_r_high, &vec_a_high);
    __m128i vec_b_high_split = _mm_shuffle_epi8(vec_b_high, split_mask);
    __m128i vec_g_high_split = _mm_shuffle_epi8(vec_g_high, split_mask);
    __m128i vec_r_high_split = _mm_shuffle_epi8(vec_r_high, split_mask);
    __m128i vec_a_high_split = _mm_shuffle_epi8(vec_a_high, split_mask);

    *b0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_b_low_split, vec_b_high_split));
    *b1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_b_low_split, vec_b_high_split));

    *g0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_g_low_split, vec_g_high_split));
    *g1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_g_low_split, vec_g_high_split));

    *r0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_r_low_split, vec_r_high_split));
    *r1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_r_low_split, vec_r_high_split));

    *a0 = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(vec_a_low_split, vec_a_high_split));
    *a1 = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(vec_a_low_split, vec_a_high_split));
    return;
}

class PA_BGRA_NV21_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGRA_NV21_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w,
            int src_h,
            bool rgb_mode, 
            bool nv12_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _src_h(src_h),
            _rgb_mode(rgb_mode), 
            _nv12_mode(nv12_mode) {
        if (_rgb_mode) {
            _r_idx = 0;
            // g_idx = 1;
            _b_idx = 2;
        } else {
            _b_idx = 0;
            // g_idx = 1;
            _r_idx = 2;
        }
    }

    void operator() (const Range& range) const override {
        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;
        int doub_src_stride = _src_stride << 1;
        int doub_dst_stride = _dst_stride << 1;
        // set const params
        __m256i vec_yc = _mm256_set1_epi16(YC); 
        __m256i vec_yb = _mm256_set1_epi16(YB); 
        __m256i vec_yg = _mm256_set1_epi16(YG); 
        __m256i vec_yr = _mm256_set1_epi16(YR); 

        __m256i vec_ub = _mm256_set1_epi16(UB); 
        __m256i vec_ug = _mm256_set1_epi16(UG); 
        __m256i vec_ur = _mm256_set1_epi16(UR); 

        __m256i vec_vb = _mm256_set1_epi16(VB); 
        __m256i vec_vg = _mm256_set1_epi16(VG); 
        __m256i vec_vr = _mm256_set1_epi16(VR);

        __m256i vec_uvc = _mm256_set1_epi16(UVC);

        // __m256i vec_zero = _mm256_setzero_si256();
        // __m256i vec_255 = _mm256_set1_epi16(255);

        __m256i blend_mask = _mm256_setr_epi8(
                0,  8,  1,  9,  2, 10,  3, 11,  4, 12,  5, 13,  6, 14,  7, 15, 
                16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

        const uint8_t* src_bgra = _src_ptr + range.start() * doub_src_stride;
        uint8_t* dst_y = _dst_ptr + range.start() * doub_dst_stride;
        uint8_t* dst_uv = _dst_ptr + _src_h * _dst_stride + range.start() * _dst_stride;

        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_bgra_0 = src_bgra;
            const uint8_t* cur_bgra_1 = src_bgra + _src_stride;

            uint8_t* cur_y_0 = dst_y;
            uint8_t* cur_y_1 = dst_y + _dst_stride;
            uint8_t* cur_uv = dst_uv;

            for (int j = 0; j < width_align32; j += 32) {
                __m256i b00;
                __m256i b01;
                __m256i g00;
                __m256i g01;
                __m256i r00;
                __m256i r01;
                __m256i a00;
                __m256i a01;
                if (_rgb_mode) {
                    deinterleave_pa_bgra(cur_bgra_0, &r00, &r01, &g00, &g01, &b00, &b01, &a00, &a01);
                } else {
                    deinterleave_pa_bgra(cur_bgra_0, &b00, &b01, &g00, &g01, &r00, &r01, &a00, &a01);
                }
                __m256i b10;
                __m256i b11;
                __m256i g10;
                __m256i g11;
                __m256i r10;
                __m256i r11;
                __m256i a10;
                __m256i a11;
                if (_rgb_mode) {
                    deinterleave_pa_bgra(cur_bgra_1, &r10, &r11, &g10, &g11, &b10, &b11, &a10, &a11);
                } else {
                    deinterleave_pa_bgra(cur_bgra_1, &b10, &b11, &g10, &g11, &r10, &r11, &a10, &a11);
                }
                // y00
                __m256i product_temp_b = _mm256_mullo_epi16(b00, vec_yb);
                __m256i product_temp_g = _mm256_mullo_epi16(g00, vec_yg);
                __m256i product_temp_r = _mm256_mullo_epi16(r00, vec_yr);
                __m256i sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y00 = _mm256_srli_epi16(sum_temp, 8);
                // y01
                product_temp_b = _mm256_mullo_epi16(b01, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g01, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r01, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y01 = _mm256_srli_epi16(sum_temp, 8);
                // y10
                product_temp_b = _mm256_mullo_epi16(b10, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g10, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r10, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y10 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y10 = _mm256_srli_epi16(sum_temp, 8);
                // y11
                product_temp_b = _mm256_mullo_epi16(b11, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g11, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r11, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y11 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y11 = _mm256_srli_epi16(sum_temp, 8);
                // interpolation b,g,r for u v
                // __m256i avg_b = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(b00, b01), _mm256_add_epi16(b10, b11)), 
                //         2);
                // __m256i avg_g = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(g00, g01), _mm256_add_epi16(g10, g11)), 
                //         2);
                // __m256i avg_r = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(r00, r01), _mm256_add_epi16(r10, r11)), 
                //         2);
                __m256i avg_b = b00;
                __m256i avg_g = g00;
                __m256i avg_r = r00;
                // u
                product_temp_b = _mm256_mullo_epi16(avg_b, vec_ub);
                product_temp_g = _mm256_mullo_epi16(avg_g, vec_ug);
                product_temp_r = _mm256_mullo_epi16(avg_r, vec_ur);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_uvc);
                // __m256i vec_u0 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_u0 = _mm256_srli_epi16(sum_temp, 8);
                // v
                product_temp_b = _mm256_mullo_epi16(avg_b, vec_vb);
                product_temp_g = _mm256_mullo_epi16(avg_g, vec_vg);
                product_temp_r = _mm256_mullo_epi16(avg_r, vec_vr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_uvc);
                // __m256i vec_v0 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_v0 = _mm256_srli_epi16(sum_temp, 8);
                // s16 to u8 with blend
                __m256i final_y0 = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_y00, vec_y01), blend_mask);
                __m256i final_y1 = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_y10, vec_y11), blend_mask);
                __m256i final_vu;
                if (_nv12_mode) {
                    final_vu = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_u0, vec_v0), blend_mask);
                } else {
                    final_vu = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_v0, vec_u0), blend_mask);
                }
                // store final results
                _mm256_storeu_si256((__m256i*)cur_y_0, final_y0);
                _mm256_storeu_si256((__m256i*)cur_y_1, final_y1);
                _mm256_storeu_si256((__m256i*)cur_uv, final_vu);

                cur_bgra_0 += 128;
                cur_bgra_1 += 128;
                cur_y_0 += 32;
                cur_y_1 += 32;
                cur_uv += 32;
            }
            if (remain) {
                convert_to_yuv_one_row(cur_bgra_0, cur_y_0, cur_uv, _src_w,
                        _src_stride, width_align32, _nv12_mode, _b_idx, _r_idx, 4);
            }

            src_bgra += doub_src_stride;
            dst_y += doub_dst_stride;
            dst_uv += _dst_stride;
        }
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _rgb_mode;
    bool _nv12_mode;
    int _b_idx;
    int _r_idx;
};

static void bgra2nv21_avx(
        const Mat& src,
        Mat& dst,
        bool rgb_mode = false, 
        bool nv12_mode = false) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    int half_h = dst.height() / 2;
    PA_BGRA_NV21_Convert_AVX_ParallelTask task(src_stride, src_ptr,
            dst_stride, dst_ptr, src_w, src_h, rgb_mode, nv12_mode);
    parallel_run(Range(0, half_h), task);
}

class PA_BGR_NV21_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_NV21_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w,
            int src_h,
            bool rgb_mode, 
            bool nv12_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _src_h(src_h),
            _rgb_mode(rgb_mode), 
            _nv12_mode(nv12_mode) {
        if (_rgb_mode) {
            _r_idx = 0;
            // g_idx = 1;
            _b_idx = 2;
        } else {
            _b_idx = 0;
            // g_idx = 1;
            _r_idx = 2;
        }
    }

    void operator() (const Range& range) const override {
        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;
        int doub_src_stride = _src_stride << 1;
        int doub_dst_stride = _dst_stride << 1;
        // set const params
        __m256i vec_yc = _mm256_set1_epi16(YC); 
        __m256i vec_yb = _mm256_set1_epi16(YB); 
        __m256i vec_yg = _mm256_set1_epi16(YG); 
        __m256i vec_yr = _mm256_set1_epi16(YR); 

        __m256i vec_ub = _mm256_set1_epi16(UB); 
        __m256i vec_ug = _mm256_set1_epi16(UG); 
        __m256i vec_ur = _mm256_set1_epi16(UR); 

        __m256i vec_vb = _mm256_set1_epi16(VB); 
        __m256i vec_vg = _mm256_set1_epi16(VG); 
        __m256i vec_vr = _mm256_set1_epi16(VR);

        __m256i vec_uvc = _mm256_set1_epi16(UVC);

        // __m256i vec_zero = _mm256_setzero_si256();
        // __m256i vec_255 = _mm256_set1_epi16(255);

        __m256i blend_mask = _mm256_setr_epi8(
                0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, 
                16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

        const uint8_t* src_bgr = _src_ptr + range.start() * doub_src_stride;
        uint8_t* dst_y = _dst_ptr + range.start() * doub_dst_stride;
        uint8_t* dst_uv = _dst_ptr + _src_h * _dst_stride + range.start() * _dst_stride;

        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_bgr_0 = src_bgr;
            const uint8_t* cur_bgr_1 = src_bgr + _src_stride;

            uint8_t* cur_y_0 = dst_y;
            uint8_t* cur_y_1 = dst_y + _dst_stride;
            uint8_t* cur_uv = dst_uv;

            for (int j = 0; j < width_align32; j += 32) {
                __m256i b00;
                __m256i b01;
                __m256i g00;
                __m256i g01;
                __m256i r00;
                __m256i r01;
                if (_rgb_mode) {
                    deinterleave_pa_bgr(cur_bgr_0, &r00, &r01, &g00, &g01, &b00, &b01);
                } else {
                    deinterleave_pa_bgr(cur_bgr_0, &b00, &b01, &g00, &g01, &r00, &r01);
                }
                __m256i b10;
                __m256i b11;
                __m256i g10;
                __m256i g11;
                __m256i r10;
                __m256i r11;
                if (_rgb_mode) {
                    deinterleave_pa_bgr(cur_bgr_1, &r10, &r11, &g10, &g11, &b10, &b11);
                } else {
                    deinterleave_pa_bgr(cur_bgr_1, &b10, &b11, &g10, &g11, &r10, &r11);
                }
                // y00
                __m256i product_temp_b = _mm256_mullo_epi16(b00, vec_yb);
                __m256i product_temp_g = _mm256_mullo_epi16(g00, vec_yg);
                __m256i product_temp_r = _mm256_mullo_epi16(r00, vec_yr);
                __m256i sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y00 = _mm256_srli_epi16(sum_temp, 8);
                // y01
                product_temp_b = _mm256_mullo_epi16(b01, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g01, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r01, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y01 = _mm256_srli_epi16(sum_temp, 8);
                // y10
                product_temp_b = _mm256_mullo_epi16(b10, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g10, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r10, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y10 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y10 = _mm256_srli_epi16(sum_temp, 8);
                // y11
                product_temp_b = _mm256_mullo_epi16(b11, vec_yb);
                product_temp_g = _mm256_mullo_epi16(g11, vec_yg);
                product_temp_r = _mm256_mullo_epi16(r11, vec_yr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_yc);
                // __m256i vec_y11 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_y11 = _mm256_srli_epi16(sum_temp, 8);
                // interpolation b,g,r for u v
                // __m256i avg_b = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(b00, b01), _mm256_add_epi16(b10, b11)), 
                //         2);
                // __m256i avg_g = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(g00, g01), _mm256_add_epi16(g10, g11)), 
                //         2);
                // __m256i avg_r = _mm256_srai_epi16(
                //         _mm256_add_epi16(_mm256_add_epi16(r00, r01), _mm256_add_epi16(r10, r11)), 
                //         2);
                __m256i avg_b = b00;
                __m256i avg_g = g00;
                __m256i avg_r = r00;
                // u
                product_temp_b = _mm256_mullo_epi16(avg_b, vec_ub);
                product_temp_g = _mm256_mullo_epi16(avg_g, vec_ug);
                product_temp_r = _mm256_mullo_epi16(avg_r, vec_ur);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_uvc);
                // __m256i vec_u0 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_u0 = _mm256_srli_epi16(sum_temp, 8);
                // v
                product_temp_b = _mm256_mullo_epi16(avg_b, vec_vb);
                product_temp_g = _mm256_mullo_epi16(avg_g, vec_vg);
                product_temp_r = _mm256_mullo_epi16(avg_r, vec_vr);
                sum_temp = _mm256_add_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(product_temp_b, product_temp_g), product_temp_r), 
                        vec_uvc);
                // __m256i vec_v0 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(sum_temp, 8), vec_255), vec_zero);
                __m256i vec_v0 = _mm256_srli_epi16(sum_temp, 8);
                // s16 to u8 with blend
                __m256i final_y0 = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_y00, vec_y01), blend_mask);
                __m256i final_y1 = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_y10, vec_y11), blend_mask);
                __m256i final_vu;
                if (_nv12_mode) {
                    final_vu = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_u0, vec_v0), blend_mask);
                } else {
                    final_vu = _mm256_shuffle_epi8(_mm256_packus_epi16(vec_v0, vec_u0), blend_mask);
                }
                // store final results
                _mm256_storeu_si256((__m256i*)cur_y_0, final_y0);
                _mm256_storeu_si256((__m256i*)cur_y_1, final_y1);
                _mm256_storeu_si256((__m256i*)cur_uv, final_vu);

                cur_bgr_0 += 96;
                cur_bgr_1 += 96;
                cur_y_0 += 32;
                cur_y_1 += 32;
                cur_uv += 32;
            }
            if (remain) {
                convert_to_yuv_one_row(cur_bgr_0, cur_y_0, cur_uv, _src_w,
                        _src_stride, width_align32, _nv12_mode, _b_idx, _r_idx, 3);
            }

            src_bgr += doub_src_stride;
            dst_y += doub_dst_stride;
            dst_uv += _dst_stride;
        }
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _rgb_mode;
    bool _nv12_mode;
    int _b_idx;
    int _r_idx;
};

static void bgr2nv21_avx(
        const Mat& src,
        Mat& dst,
        bool rgb_mode = false, 
        bool nv12_mode = false) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    int half_h = dst.height() / 2;
    PA_BGR_NV21_Convert_AVX_ParallelTask task(src_stride, src_ptr,
            dst_stride, dst_ptr, src_w, src_h, rgb_mode, nv12_mode);
    parallel_run(Range(0, half_h), task);
}

class PA_BGR_GRAY_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_GRAY_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w,
            int src_h,
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _src_h(src_h),
            _rgb_mode(rgb_mode) {}

    void operator() (const Range& range) const override {
        UN_USED(_src_h);
        int width_align16 = _src_w & (~16);
        int remain = _src_w - width_align16;
        // set const params
        __m256i vec_b_factor = _mm256_set1_epi16(B_RATION); 
        __m256i vec_g_factor = _mm256_set1_epi16(G_RATION); 
        __m256i vec_r_factor = _mm256_set1_epi16(R_RATION);

        int b_factor = B_RATION; 
        int g_factor = G_RATION; 
        int r_factor = R_RATION; 

        if (_rgb_mode) {
            vec_b_factor = _mm256_set1_epi16(R_RATION);
            vec_r_factor = _mm256_set1_epi16(B_RATION);
            b_factor = R_RATION;
            r_factor = B_RATION;
        }

        const __m256i VEC_HALF = _mm256_set1_epi16(128); 
        __m256i vec_zero = _mm256_setzero_si256();
        const int SPLIT_MASK = 0b01111000;

        const uint8_t* src_bgr = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_y = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t *y0_ptr = src_bgr;
            uint8_t *dst0_ptr = dst_y;

            for (int j = 0; j < width_align16; j += 16) {
                __m128i vec_b_u8;
                __m128i vec_g_u8;
                __m128i vec_r_u8;
                vld3_u8x16_avx(y0_ptr, &vec_b_u8, &vec_g_u8, &vec_r_u8);

                __m256i vec_b_u16 = _mm256_cvtepu8_epi16(vec_b_u8);
                __m256i vec_g_u16 = _mm256_cvtepu8_epi16(vec_g_u8);
                __m256i vec_r_u16 = _mm256_cvtepu8_epi16(vec_r_u8);

                __m256i vec_b_product = _mm256_mullo_epi16(vec_b_u16, vec_b_factor);
                __m256i vec_g_product = _mm256_mullo_epi16(vec_g_u16, vec_g_factor);
                __m256i vec_r_product = _mm256_mullo_epi16(vec_r_u16, vec_r_factor);

                __m256i vec_sum = _mm256_add_epi16(_mm256_add_epi16(vec_b_product, vec_g_product), vec_r_product);
                __m256i vec_y_u16 = _mm256_srli_epi16(_mm256_add_epi16(vec_sum, VEC_HALF), Q);
                // __m256i vec_y_u16 = _mm256_srli_epi16(vec_sum, Q);

                __m256i vec_blend_u8 = _mm256_packus_epi16(vec_y_u16, vec_zero);
                __m256i vec_split_u8 = _mm256_permute4x64_epi64(vec_blend_u8, SPLIT_MASK);

                // store final results
                _mm_storeu_si128((__m128i*)dst0_ptr, _mm256_castsi256_si128(vec_split_u8));

                y0_ptr += 48;
                dst0_ptr += 16;
            }
            for (int j = 0; j < remain; j++) {
                uint32_t value_0 = y0_ptr[0] * b_factor + y0_ptr[1] * g_factor + y0_ptr[2] * r_factor;
                dst0_ptr[0] = uint8_t((value_0 + 128) >> Q);
                // dst0_ptr[0] = uint8_t(value_0 >> Q);
                y0_ptr += 3;
                dst0_ptr += 1;
            }

            src_bgr += _src_stride;
            dst_y += _dst_stride;
        }
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _rgb_mode;
};

static void bgr2gray_avx(
        const Mat& src, 
        Mat& dst,
        bool rgb_mode = false) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    PA_BGR_GRAY_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w, src_h, rgb_mode);
    parallel_run(Range(0, src_h), task);
}

static inline void swap_element_u8(
        uint8_t* data,
        int index_a,
        int index_b) {
    uint8_t tmp = data[index_a];
    data[index_a] = data[index_b];
    data[index_b] = tmp;
}

class PA_BGR_PA_RGB_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_PA_RGB_Convert_AVX_ParallelTask(
            int src_stride,
            const unsigned char* src_ptr,
            int dst_stride,
            unsigned char* dst_ptr,
            int parallel_size,
            int count,
            int remain)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _parallel_size(parallel_size),
            _count(count),
            _remain(remain) {}

    void operator() (const Range& range) const override {
        // special boundry data index : 15 - 17, 30 - 32, 63 - 65, 78 - 80
        int8_t mask_1[32] = {
                2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 15, 
                0, 1, 4, 3, 2, 7, 6, 5, 10, 9, 8, 13, 12, 11, 14, 15};
        int8_t mask_2[32] = {
                0, 3, 2, 1, 6, 5, 4, 9, 8, 7, 12, 11, 10, 15, 14, 13,
                2, 1, 0, 5, 4, 3, 8, 7, 6, 11, 10, 9, 14, 13, 12, 15};
        int8_t mask_3[32] = {
                0, 1, 4, 3, 2, 7, 6, 5, 10, 9, 8, 13, 12, 11, 14, 15,
                0, 3, 2, 1, 6, 5, 4, 9, 8, 7, 12, 11, 10, 15, 14, 13};
        __m256i vec_mask_1 = _mm256_loadu_si256((__m256i const*)mask_1);
        __m256i vec_mask_2 = _mm256_loadu_si256((__m256i const*)mask_2);
        __m256i vec_mask_3 = _mm256_loadu_si256((__m256i const*)mask_3);

        const uint8_t* src_bgr = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_rgb = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_bgr = src_bgr;
            uint8_t* ptr_cur_dst_rgb = dst_rgb;
            for (int j = 0; j < _count; j += 32) {
                __m256i vec_src_1 = _mm256_loadu_si256((__m256i const*)ptr_cur_src_bgr);
                __m256i vec_src_2 = _mm256_loadu_si256((__m256i const*)(ptr_cur_src_bgr + 32));
                __m256i vec_src_3 = _mm256_loadu_si256((__m256i const*)(ptr_cur_src_bgr + 64));

                __m256i vec_dst_1 = _mm256_shuffle_epi8(vec_src_1, vec_mask_1);
                __m256i vec_dst_2 = _mm256_shuffle_epi8(vec_src_2, vec_mask_2);
                __m256i vec_dst_3 = _mm256_shuffle_epi8(vec_src_3, vec_mask_3);

                _mm256_storeu_si256((__m256i*)(ptr_cur_dst_rgb), vec_dst_1);
                _mm256_storeu_si256((__m256i*)(ptr_cur_dst_rgb + 32), vec_dst_2);
                _mm256_storeu_si256((__m256i*)(ptr_cur_dst_rgb + 64), vec_dst_3);

                // Handling boundary data
                swap_element_u8(ptr_cur_dst_rgb, 15, 17);
                swap_element_u8(ptr_cur_dst_rgb, 30, 32);
                swap_element_u8(ptr_cur_dst_rgb, 63, 65);
                swap_element_u8(ptr_cur_dst_rgb, 78, 80);

                ptr_cur_src_bgr += 96;
                ptr_cur_dst_rgb += 96;
            }

            if (_remain) {
                const uint8_t* src_ptr0 = src_bgr + _parallel_size;
                uint8_t* dst_ptr0 = dst_rgb + _parallel_size;

                for (int j = 0; j < _remain; j++) {
                    uint8_t b00 = src_ptr0[0];
                    uint8_t g00 = src_ptr0[1];
                    uint8_t r00 = src_ptr0[2];

                    dst_ptr0[0] = r00;
                    dst_ptr0[1] = g00;
                    dst_ptr0[2] = b00;

                    src_ptr0 += 3;
                    dst_ptr0 += 3;
                }
            }
            src_bgr += _src_stride;
            dst_rgb += _dst_stride;
        }
    }

private:
    int _src_stride;
    const unsigned char* _src_ptr;
    int _dst_stride;
    unsigned char* _dst_ptr;
    int _parallel_size;
    int _count;
    int _remain;
};

void convert_bgr_to_rgb_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    int count = src_w & (~31);
    int remain = src_w - count;
    int parallel_size = count * 3;

    PA_BGR_PA_RGB_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, parallel_size, count, remain);

    parallel_run(Range(0, src_h), task);
}

class PA_BGR_PA_BGRA_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_PA_BGRA_Convert_AVX_ParallelTask(
            int src_stride,
            const unsigned char* src_ptr,
            int dst_stride,
            unsigned char* dst_ptr,
            int src_w,
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _rgb_mode(rgb_mode) {}

    void operator() (const Range& range) const override {
        int8_t mask_1[32] = {0, 1, 2, 12, 3, 4, 5, 13, 6, 7, 8, 14, 9, 10, 11, 15, 
                             0, 1, 2, 12, 3, 4, 5, 13, 6, 7, 8, 14, 9, 10, 11, 15};
        int8_t mask_2[32] = {12, 13, 14, 8, 15, 0, 1, 9, 2, 3, 4, 10, 5, 6, 7, 11,
                             12, 13, 14, 8, 15, 0, 1, 9, 2, 3, 4, 10, 5, 6, 7, 11};
        int8_t mask_3[32] = {8, 9, 10, 4, 11, 12, 13, 5, 14, 15, 0, 6, 1, 2, 3, 7,
                             8, 9, 10, 4, 11, 12, 13, 5, 14, 15, 0, 6, 1, 2, 3, 7};
        int8_t mask_4[32] = {4, 5, 6, 0, 7, 8, 9, 1, 10, 11, 12, 2, 13, 14, 15, 3,
                             4, 5, 6, 0, 7, 8, 9, 1, 10, 11, 12, 2, 13, 14, 15, 3};
        if (_rgb_mode) {
            int8_t rgb_mask_1[32] = {2, 1, 0, 12, 5, 4, 3, 13, 8, 7, 6, 14, 11, 10, 9, 15, 
                                     2, 1, 0, 12, 5, 4, 3, 13, 8, 7, 6, 14, 11, 10, 9, 15};
            int8_t rgb_mask_2[32] = {14, 13, 12, 8, 1, 0, 15, 9, 4, 3, 2, 10, 7, 6, 5, 11,
                                     14, 13, 12, 8, 1, 0, 15, 9, 4, 3, 2, 10, 7, 6, 5, 11};
            int8_t rgb_mask_3[32] = {10, 9, 8, 4, 13, 12, 11, 5, 0, 15, 14, 6, 3, 2, 1, 7,
                                     10, 9, 8, 4, 13, 12, 11, 5, 0, 15, 14, 6, 3, 2, 1, 7};
            int8_t rgb_mask_4[32] = {6, 5, 4, 0, 9, 8, 7, 1, 12, 11, 10, 2, 15, 14, 13, 3,
                                     6, 5, 4, 0, 9, 8, 7, 1, 12, 11, 10, 2, 15, 14, 13, 3};
            memcpy(mask_1, rgb_mask_1, 32);
            memcpy(mask_2, rgb_mask_2, 32);
            memcpy(mask_3, rgb_mask_3, 32);
            memcpy(mask_4, rgb_mask_4, 32);
        }
        __m256i vec_mask_1 = _mm256_loadu_si256((__m256i const*)mask_1);
        __m256i vec_mask_2 = _mm256_loadu_si256((__m256i const*)mask_2);
        __m256i vec_mask_3 = _mm256_loadu_si256((__m256i const*)mask_3);
        __m256i vec_mask_4 = _mm256_loadu_si256((__m256i const*)mask_4);

        __m256i vec_255 = _mm256_set1_epi8(char(255));

        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;

        const uint8_t* src_bgr = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_bgra = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_bgr = src_bgr;
            uint8_t* ptr_cur_dst_bgra = dst_bgra;
            for (int j = 0; j < width_align32; j += 32) {
                __m256i vec_src_1 = _mm256_loadu2_m128i((__m128i const*)(ptr_cur_src_bgr + 48), (__m128i const*)(ptr_cur_src_bgr));
                __m256i vec_src_2 = _mm256_loadu2_m128i((__m128i const*)(ptr_cur_src_bgr + 64), (__m128i const*)(ptr_cur_src_bgr + 16));
                __m256i vec_src_3 = _mm256_loadu2_m128i((__m128i const*)(ptr_cur_src_bgr + 80), (__m128i const*)(ptr_cur_src_bgr + 32));

                __m256i vec_dst_1 = _mm256_shuffle_epi8(_mm256_blend_epi32(vec_src_1, vec_255, 0b10001000), vec_mask_1);
                __m256i vec_layer_1_2 = _mm256_blend_epi32(vec_src_2, _mm256_blend_epi32(vec_255, vec_src_1, 0b10001000), 0b11001100);
                __m256i vec_dst_2 = _mm256_shuffle_epi8(vec_layer_1_2, vec_mask_2);
                __m256i vec_layer_2_3 = _mm256_blend_epi32(vec_src_2, _mm256_blend_epi32(vec_255, vec_src_3, 0b00010001), 0b00110011);
                __m256i vec_dst_3 = _mm256_shuffle_epi8(vec_layer_2_3, vec_mask_3);
                __m256i vec_dst_4 = _mm256_shuffle_epi8(_mm256_blend_epi32(vec_src_3, vec_255, 0b00010001), vec_mask_4);

                _mm256_storeu2_m128i((__m128i*)(ptr_cur_dst_bgra +  64), (__m128i*)(ptr_cur_dst_bgra +  0), vec_dst_1);
                _mm256_storeu2_m128i((__m128i*)(ptr_cur_dst_bgra +  80), (__m128i*)(ptr_cur_dst_bgra + 16), vec_dst_2);
                _mm256_storeu2_m128i((__m128i*)(ptr_cur_dst_bgra +  96), (__m128i*)(ptr_cur_dst_bgra + 32), vec_dst_3);
                _mm256_storeu2_m128i((__m128i*)(ptr_cur_dst_bgra + 112), (__m128i*)(ptr_cur_dst_bgra + 48), vec_dst_4);

                ptr_cur_src_bgr  +=  96;
                ptr_cur_dst_bgra += 128;
            }

            if (remain) {
                for (int j = 0; j < remain; j++) {
                    uint8_t b00 = ptr_cur_src_bgr[0];
                    uint8_t g00 = ptr_cur_src_bgr[1];
                    uint8_t r00 = ptr_cur_src_bgr[2];

                    if (_rgb_mode) {
                        ptr_cur_dst_bgra[0] = r00;
                        ptr_cur_dst_bgra[1] = g00;
                        ptr_cur_dst_bgra[2] = b00;
                    } else {
                        ptr_cur_dst_bgra[0] = b00;
                        ptr_cur_dst_bgra[1] = g00;
                        ptr_cur_dst_bgra[2] = r00;
                    }
                    ptr_cur_dst_bgra[3] = 255;

                    ptr_cur_src_bgr += 3;
                    ptr_cur_dst_bgra += 4;
                }
            }
            src_bgr += _src_stride;
            dst_bgra += _dst_stride;
        }
    }

private:
    int _src_stride;
    const unsigned char* _src_ptr;
    int _dst_stride;
    unsigned char* _dst_ptr;
    int _src_w;
    bool _rgb_mode;
};

static void convert_bgr_to_bgra_avx(const Mat& src, Mat& dst, bool rgb_mode) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGR_PA_BGRA_Convert_AVX_ParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w, rgb_mode);

    parallel_run(Range(0, src_h), task);
}

class PA_BGRA_PA_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGRA_PA_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const unsigned char* src_ptr,
            int dst_stride,
            unsigned char* dst_ptr,
            int src_w,
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _rgb_mode(rgb_mode) {}

    void operator() (const Range& range) const override {
        int8_t mask_4c_3c[32] = {0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1, 
                                 0, 1, 2, 4, 5, 6, 8, 9, 10, 12, 13, 14, -1, -1, -1, -1};
        int32_t mask_permute_low[8]  = {0, 1, 2, 4, 5, 6, 3, 7};
        int32_t mask_permute_high[8] = {2, 4, 5, 6, 3, 7, 0, 1};
        if (_rgb_mode) {
            int8_t rgb_mask_4c_3c[32] = {2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1, 
                                         2, 1, 0, 6, 5, 4, 10, 9, 8, 14, 13, 12, -1, -1, -1, -1};
            memcpy(mask_4c_3c, rgb_mask_4c_3c, 32);
        }
        __m256i vec_mask_4c_3c = _mm256_loadu_si256((__m256i const*)mask_4c_3c);
        __m256i vec_mask_permute_low = _mm256_loadu_si256((__m256i const*)mask_permute_low);
        __m256i vec_mask_permute_high = _mm256_loadu_si256((__m256i const*)mask_permute_high);

        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;

        const uint8_t* src_bgra = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_bgr = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_bgra = src_bgra;
            uint8_t* ptr_cur_dst_bgr = dst_bgr;
            for (int j = 0; j < width_align16; j += 16) {
                __m256i vec_src_1 = _mm256_loadu_si256((__m256i const*)(ptr_cur_src_bgra +  0));
                __m256i vec_src_2 = _mm256_loadu_si256((__m256i const*)(ptr_cur_src_bgra + 32));

                __m256i vec_layer_1_1 = _mm256_shuffle_epi8(vec_src_1, vec_mask_4c_3c);
                __m256i vec_layer_1_2 = _mm256_shuffle_epi8(vec_src_2, vec_mask_4c_3c);

                __m256i vec_layer_2_1 = _mm256_permutevar8x32_epi32(vec_layer_1_1, vec_mask_permute_low);
                __m256i vec_layer_2_2 = _mm256_permutevar8x32_epi32(vec_layer_1_2, vec_mask_permute_high);

                __m256i vec_dst_1 = _mm256_blend_epi32(vec_layer_2_1, vec_layer_2_2, 0b11000000);
                __m128i vec_dst_2 = _mm256_castsi256_si128(vec_layer_2_2);

                _mm256_storeu_si256((__m256i*)(ptr_cur_dst_bgr), vec_dst_1);
                _mm_storeu_si128((__m128i*)(ptr_cur_dst_bgr + 32), vec_dst_2);

                ptr_cur_src_bgra  +=  64;
                ptr_cur_dst_bgr += 48;
            }

            if (remain) {
                for (int j = 0; j < remain; j++) {
                    uint8_t b00 = ptr_cur_src_bgra[0];
                    uint8_t g00 = ptr_cur_src_bgra[1];
                    uint8_t r00 = ptr_cur_src_bgra[2];

                    if (_rgb_mode) {
                        ptr_cur_dst_bgr[0] = r00;
                        ptr_cur_dst_bgr[1] = g00;
                        ptr_cur_dst_bgr[2] = b00;
                    } else {
                        ptr_cur_dst_bgr[0] = b00;
                        ptr_cur_dst_bgr[1] = g00;
                        ptr_cur_dst_bgr[2] = r00;
                    }

                    ptr_cur_src_bgra += 4;
                    ptr_cur_dst_bgr += 3;
                }
            }
            src_bgra += _src_stride;
            dst_bgr += _dst_stride;
        }
    }

private:
    int _src_stride;
    const unsigned char* _src_ptr;
    int _dst_stride;
    unsigned char* _dst_ptr;
    int _src_w;
    bool _rgb_mode;
};

static void convert_bgra_to_bgr_avx(const Mat& src, Mat& dst, bool rgb_mode) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGRA_PA_BGR_Convert_AVX_ParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w, rgb_mode);

    parallel_run(Range(0, src_h), task);
}

class PA_BGRA_PA_RGBA_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGRA_PA_RGBA_Convert_AVX_ParallelTask(
            int src_stride,
            const unsigned char* src_ptr,
            int dst_stride,
            unsigned char* dst_ptr,
            int src_w)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w) {}

    void operator() (const Range& range) const override {
        int8_t mask_4c_3c[32] = {2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15, 
                                 2, 1, 0, 3, 6, 5, 4, 7, 10, 9, 8, 11, 14, 13, 12, 15};
        __m256i vec_mask_4c_3c = _mm256_loadu_si256((__m256i const*)mask_4c_3c);

        int width_align8 = _src_w & (~7);
        int remain = _src_w - width_align8;

        const uint8_t* src_bgra = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_rgba = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_bgra = src_bgra;
            uint8_t* ptr_cur_dst_rgba = dst_rgba;
            for (int j = 0; j < width_align8; j += 8) {
                __m256i vec_src = _mm256_loadu_si256((__m256i const*)(ptr_cur_src_bgra));
                _mm256_storeu_si256((__m256i*)(ptr_cur_dst_rgba), _mm256_shuffle_epi8(vec_src, vec_mask_4c_3c));
                ptr_cur_src_bgra += 32;
                ptr_cur_dst_rgba += 32;
            }
            if (remain) {
                for (int j = 0; j < remain; j++) {
                    uint8_t b00 = ptr_cur_src_bgra[0];
                    uint8_t g00 = ptr_cur_src_bgra[1];
                    uint8_t r00 = ptr_cur_src_bgra[2];
                    uint8_t a00 = ptr_cur_src_bgra[2];

                    ptr_cur_dst_rgba[0] = r00;
                    ptr_cur_dst_rgba[1] = g00;
                    ptr_cur_dst_rgba[2] = b00;
                    ptr_cur_dst_rgba[3] = a00;

                    ptr_cur_src_bgra += 4;
                    ptr_cur_dst_rgba += 4;
                }
            }
            src_bgra += _src_stride;
            dst_rgba += _dst_stride;
        }
    }
private:
    int _src_stride;
    const unsigned char* _src_ptr;
    int _dst_stride;
    unsigned char* _dst_ptr;
    int _src_w;
};

static void convert_bgra_to_rgba_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGRA_PA_RGBA_Convert_AVX_ParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w);

    parallel_run(Range(0, src_h), task);
}

class GRAY_PA_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    GRAY_PA_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w) {}

    void operator() (const Range& range) const override {
        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;

        const uint8_t* cur_src_ptr = _src_ptr + range.start() * _src_stride;
        uint8_t* cur_dst_ptr = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* src_gray = cur_src_ptr;
            uint8_t* dst_bgr = cur_dst_ptr;
            for (int j = 0; j < width_align32; j += 32) {
                __m256i vec_data = _mm256_loadu_si256((__m256i const*)src_gray);
                vst3_u8x32_avx(vec_data, vec_data, vec_data, dst_bgr);

                src_gray += 32;
                dst_bgr += 96;
            }

            for (int j = 0; j < remain; j++) {
                uint8_t left_src = src_gray[0];
                dst_bgr[0] = left_src;
                dst_bgr[1] = left_src;
                dst_bgr[2] = left_src;

                src_gray++;
                dst_bgr += 3;
            }

            cur_src_ptr += _src_stride;
            cur_dst_ptr += _dst_stride;
        }
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
};

static void convert_gray_to_bgr_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    GRAY_PA_BGR_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w);
    parallel_run(Range(0, src_h), task);
}

class GRAY_PA_BGRA_Convert_AVX_ParallelTask : public ParallelTask {
public:
    GRAY_PA_BGRA_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w) {}

    void operator() (const Range& range) const override {
        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;
        __m256i vec_255 = _mm256_set1_epi8(char(255));

        const uint8_t* ptr_src_gray = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst_bgra = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src_gray = ptr_src_gray;
            uint8_t* cur_dst_bgra = ptr_dst_bgra;
            for (int j = 0; j < width_align32; j += 32) {
                __m256i vec_data = _mm256_loadu_si256((__m256i const*)cur_src_gray);
                vst4_u8x32_avx(vec_data, vec_data, vec_data, vec_255, cur_dst_bgra);
                cur_src_gray += 32;
                cur_dst_bgra += 128;
            }
            for (int j = 0; j < remain; j++) {
                uint8_t left_src = cur_src_gray[0];
                cur_dst_bgra[0] = left_src;
                cur_dst_bgra[1] = left_src;
                cur_dst_bgra[2] = left_src;
                cur_dst_bgra[3] = left_src;
                ++cur_src_gray;
                cur_dst_bgra += 4;
            }
            ptr_src_gray += _src_stride;
            ptr_dst_bgra += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
};

static void convert_gray_to_bgra_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    GRAY_PA_BGRA_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w);
    parallel_run(Range(0, src_h), task);
}

class PA_BGR_PL_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_PL_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w, 
            int src_h)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _src_h(src_h) {}

    void operator() (const Range& range) const override {
        Vld3_U8x32_Avx de_interleave_tool;

        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;

        const uint8_t* src_pa = _src_ptr + range.start() * _src_stride;
        uint8_t* dst_pl_c0 = _dst_ptr + range.start() * _dst_stride;
        uint8_t* dst_pl_c1 = dst_pl_c0 + _src_h * _dst_stride;
        uint8_t* dst_pl_c2 = dst_pl_c1 + _src_h * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_pa = src_pa;
            uint8_t* ptr_cur_dst_pl_c0 = dst_pl_c0;
            uint8_t* ptr_cur_dst_pl_c1 = dst_pl_c1;
            uint8_t* ptr_cur_dst_pl_c2 = dst_pl_c2;
            for (int j = 0; j < width_align32; j += 32) {
                __m256i vec_b;
                __m256i vec_g;
                __m256i vec_r;
                de_interleave_tool.load(ptr_cur_src_pa, &vec_b, &vec_g, &vec_r);
                _mm256_storeu_si256((__m256i*)ptr_cur_dst_pl_c0, vec_b);
                _mm256_storeu_si256((__m256i*)ptr_cur_dst_pl_c1, vec_g);
                _mm256_storeu_si256((__m256i*)ptr_cur_dst_pl_c2, vec_r);
                ptr_cur_src_pa += 96;
                ptr_cur_dst_pl_c0 += 32;
                ptr_cur_dst_pl_c1 += 32;
                ptr_cur_dst_pl_c2 += 32;
            }
            if (remain) {
                for (int j = 0; j < remain; j++) {
                    *ptr_cur_dst_pl_c0 = ptr_cur_src_pa[0];
                    *ptr_cur_dst_pl_c1 = ptr_cur_src_pa[1];
                    *ptr_cur_dst_pl_c2 = ptr_cur_src_pa[2];

                    ptr_cur_src_pa += 3;
                    ++ptr_cur_dst_pl_c0;
                    ++ptr_cur_dst_pl_c1;
                    ++ptr_cur_dst_pl_c2;
                }
            }
            src_pa += _src_stride;
            dst_pl_c0 += _dst_stride;
            dst_pl_c1 += _dst_stride;
            dst_pl_c2 += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
};

static void convert_package_to_planar_c3_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGR_PL_BGR_Convert_AVX_ParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w, src_h);

    parallel_run(Range(0, src_h), task);
}

class PL_BGR_PA_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PL_BGR_PA_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w, 
            int src_h)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _src_h(src_h) {}

    void operator() (const Range& range) const override {
        int width_align32 = _src_w & (~31);
        int remain = _src_w - width_align32;

        const uint8_t* src_pl_c0 = _src_ptr + range.start() * _src_stride;
        const uint8_t* src_pl_c1 = src_pl_c0 + _src_h * _src_stride;
        const uint8_t* src_pl_c2 = src_pl_c1 + _src_h * _src_stride;
        uint8_t* dst_pa_bgr = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* ptr_cur_src_pl_c0 = src_pl_c0;
            const uint8_t* ptr_cur_src_pl_c1 = src_pl_c1;
            const uint8_t* ptr_cur_src_pl_c2 = src_pl_c2;
            uint8_t* ptr_cur_dst_pa_bgr = dst_pa_bgr;
            for (int j = 0; j < width_align32; j += 32) {
                __m256i vec_b = _mm256_loadu_si256((__m256i const*)ptr_cur_src_pl_c0);
                __m256i vec_g = _mm256_loadu_si256((__m256i const*)ptr_cur_src_pl_c1);
                __m256i vec_r = _mm256_loadu_si256((__m256i const*)ptr_cur_src_pl_c2);
                vst3_u8x32_avx(vec_b, vec_g, vec_r, ptr_cur_dst_pa_bgr);
                ptr_cur_src_pl_c0 += 32;
                ptr_cur_src_pl_c1 += 32;
                ptr_cur_src_pl_c2 += 32;
                ptr_cur_dst_pa_bgr += 96;
            }
            if (remain) {
                for (int j = 0; j < remain; j++) {
                    ptr_cur_dst_pa_bgr[0] = (*ptr_cur_src_pl_c0);
                    ptr_cur_dst_pa_bgr[1] = (*ptr_cur_src_pl_c1);
                    ptr_cur_dst_pa_bgr[2] = (*ptr_cur_src_pl_c2);
                    ++ptr_cur_src_pl_c0;
                    ++ptr_cur_src_pl_c1;
                    ++ptr_cur_src_pl_c2;
                    ptr_cur_dst_pa_bgr += 3;
                }
            }
            src_pl_c0 += _src_stride;
            src_pl_c1 += _src_stride;
            src_pl_c2 += _src_stride;
            dst_pa_bgr += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
};

static void convert_planar_to_package_c3_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PL_BGR_PA_BGR_Convert_AVX_ParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w, src_h);

    parallel_run(Range(0, src_h), task);
}

class GRAY_PA_BGR565_Convert_AVX_ParallelTask : public ParallelTask {
public:
    GRAY_PA_BGR565_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w) {}

    void operator() (const Range& range) const override {
        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;
        __m256i vec_mask_g = _mm256_set1_epi16(short(0x07E0));
        __m256i vec_mask_r = _mm256_set1_epi16(short(0xF800));

        const uint8_t* ptr_src_gray = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst_bgr = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src_gray = ptr_src_gray;
            uint8_t* cur_dst_bgr = ptr_dst_bgr;
            for (int j = 0; j < width_align16; j += 16) {
                __m256i vec_src_u16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i const*)cur_src_gray));
                __m256i vec_src_b = _mm256_srli_epi16(vec_src_u16, 3);
                __m256i vec_src_g = _mm256_and_si256(_mm256_slli_epi16(vec_src_u16, 3), vec_mask_g);
                __m256i vec_src_r = _mm256_and_si256(_mm256_slli_epi16(vec_src_u16, 8), vec_mask_r);
                __m256i vec_dst_bgr565 = _mm256_or_si256(_mm256_or_si256(vec_src_b, vec_src_g), vec_src_r);
                _mm256_storeu_si256((__m256i*)cur_dst_bgr, vec_dst_bgr565);
                cur_src_gray += 16;
                cur_dst_bgr += 32;
            }
            for (int j = 0; j < remain; j++) {
                uint16_t gray_val = uint16_t(cur_src_gray[0]);
                convertTo565(gray_val, gray_val, gray_val, cur_dst_bgr);
                ++cur_src_gray;
                cur_dst_bgr += 2;
            }
            ptr_src_gray += _src_stride;
            ptr_dst_bgr += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
};

static void convert_gray_to_bgr565_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    GRAY_PA_BGR565_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w);
    parallel_run(Range(0, src_h), task);
}

class PA_BGR_PA_BGR565_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGR_PA_BGR565_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w, 
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w), 
            _rgb_mode(rgb_mode) {}

    void operator() (const Range& range) const override {
        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;
        __m256i vec_mask_g = _mm256_set1_epi16(short(0x07E0));
        __m256i vec_mask_r = _mm256_set1_epi16(short(0xF800));

        const uint8_t* ptr_src_bgr = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst_565 = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src_bgr = ptr_src_bgr;
            uint8_t* cur_dst_565 = ptr_dst_565;
            for (int j = 0; j < width_align16; j += 16) {
                __m128i vec_b_u8;
                __m128i vec_g_u8;
                __m128i vec_r_u8;
                if (_rgb_mode) {
                    vld3_u8x16_avx(cur_src_bgr, &vec_r_u8, &vec_g_u8, &vec_b_u8);
                } else {
                    vld3_u8x16_avx(cur_src_bgr, &vec_b_u8, &vec_g_u8, &vec_r_u8);
                }
                __m256i vec_b_u16 = _mm256_cvtepu8_epi16(vec_b_u8);
                __m256i vec_g_u16 = _mm256_cvtepu8_epi16(vec_g_u8);
                __m256i vec_r_u16 = _mm256_cvtepu8_epi16(vec_r_u8);
                __m256i vec_b_5bit = _mm256_srli_epi16(vec_b_u16, 3);
                __m256i vec_g_6bit = _mm256_and_si256(_mm256_slli_epi16(vec_g_u16, 3), vec_mask_g);
                __m256i vec_r_5bit = _mm256_and_si256(_mm256_slli_epi16(vec_r_u16, 8), vec_mask_r);
                __m256i vec_dst_bgr565 = _mm256_or_si256(_mm256_or_si256(vec_b_5bit, vec_g_6bit), vec_r_5bit);
                _mm256_storeu_si256((__m256i*)cur_dst_565, vec_dst_bgr565);
                cur_src_bgr += 48;
                cur_dst_565 += 32;
            }
            for (int j = 0; j < remain; j++) {
                uint16_t b_val = uint16_t(cur_src_bgr[0]);
                uint16_t g_val = uint16_t(cur_src_bgr[1]);
                uint16_t r_val = uint16_t(cur_src_bgr[2]);
                convertTo565(b_val, g_val, r_val, cur_dst_565);
                cur_src_bgr += 3;
                cur_dst_565 += 2;
            }
            ptr_src_bgr += _src_stride;
            ptr_dst_565 += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    bool _rgb_mode;
};

static void convert_bgr_to_bgr565_avx(const Mat& src, Mat& dst, bool rgb_mode) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGR_PA_BGR565_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w, rgb_mode);
    parallel_run(Range(0, src_h), task);
}

class PA_BGRA_PA_BGR565_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_BGRA_PA_BGR565_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w, 
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w), 
            _rgb_mode(rgb_mode) {}

    void operator() (const Range& range) const override {
        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;
        __m256i vec_mask_g = _mm256_set1_epi16(short(0x07E0));
        __m256i vec_mask_r = _mm256_set1_epi16(short(0xF800));

        const uint8_t* ptr_src_bgra = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst_565 = _dst_ptr + range.start() * _dst_stride;

        Deinterleave_4c_Avx tool;

        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src_bgra = ptr_src_bgra;
            uint8_t* cur_dst_565 = ptr_dst_565;
            for (int j = 0; j < width_align16; j += 16) {
                __m128i vec_b_u8;
                __m128i vec_g_u8;
                __m128i vec_r_u8;
                // __m128i vec_a_u8;
                if (_rgb_mode) {
                    // vld4_u8x16_avx(cur_src_bgra, &vec_r_u8, &vec_g_u8, &vec_b_u8, &vec_a_u8);
                    tool.load(cur_src_bgra, &vec_r_u8, &vec_g_u8, &vec_b_u8);
                } else {
                    // vld4_u8x16_avx(cur_src_bgra, &vec_b_u8, &vec_g_u8, &vec_r_u8, &vec_a_u8);
                    tool.load(cur_src_bgra, &vec_b_u8, &vec_g_u8, &vec_r_u8);
                }
                __m256i vec_b_u16 = _mm256_cvtepu8_epi16(vec_b_u8);
                __m256i vec_g_u16 = _mm256_cvtepu8_epi16(vec_g_u8);
                __m256i vec_r_u16 = _mm256_cvtepu8_epi16(vec_r_u8);
                __m256i vec_b_5bit = _mm256_srli_epi16(vec_b_u16, 3);
                __m256i vec_g_6bit = _mm256_and_si256(_mm256_slli_epi16(vec_g_u16, 3), vec_mask_g);
                __m256i vec_r_5bit = _mm256_and_si256(_mm256_slli_epi16(vec_r_u16, 8), vec_mask_r);
                __m256i vec_dst_bgr565 = _mm256_or_si256(_mm256_or_si256(vec_b_5bit, vec_g_6bit), vec_r_5bit);
                _mm256_storeu_si256((__m256i*)cur_dst_565, vec_dst_bgr565);
                cur_src_bgra += 64;
                cur_dst_565 += 32;
            }
            for (int j = 0; j < remain; j++) {
                uint16_t b_val = uint16_t(cur_src_bgra[0]);
                uint16_t g_val = uint16_t(cur_src_bgra[1]);
                uint16_t r_val = uint16_t(cur_src_bgra[2]);
                convertTo565(b_val, g_val, r_val, cur_dst_565);
                cur_src_bgra += 4;
                cur_dst_565 += 2;
            }
            ptr_src_bgra += _src_stride;
            ptr_dst_565 += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    bool _rgb_mode;
};

static void convert_bgra_to_bgr565_avx(const Mat& src, Mat& dst, bool rgb_mode) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_BGRA_PA_BGR565_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w, rgb_mode);
    parallel_run(Range(0, src_h), task);
}

inline __m128i calc_mrgba(__m128i& vec_src, __m256i& vec_half, __m128i& mask) {
    __m128i tmp_a = _mm_and_si128(vec_src, mask);
    __m128i tmp_b = _mm_or_si128(tmp_a, _mm_srli_epi16(tmp_a, 8));
    __m128i tmp_c = _mm_or_si128(tmp_b, _mm_srli_epi32(tmp_b, 16));

    __m256i vec_a_s16x16 = _mm256_cvtepu8_epi16(tmp_c);
    __m256i vec_src_s16x16 = _mm256_cvtepu8_epi16(vec_src);

    __m256i vec_sum_s16x16 = _mm256_add_epi16(_mm256_mullo_epi16(vec_src_s16x16, vec_a_s16x16), vec_half);

    __m256i vec_dst_s16x16 = _mm256_srli_epi16(
            _mm256_add_epi16(vec_sum_s16x16, _mm256_srli_epi16(vec_sum_s16x16, 8)), 8);

    __m256i vec_dst_tmp_d = _mm256_packus_epi16(vec_dst_s16x16, vec_dst_s16x16);
    __m128i vec_dst_tmp_e = _mm256_castsi256_si128(_mm256_permute4x64_epi64(vec_dst_tmp_d, 0b00001000));
    return _mm_blendv_epi8(vec_dst_tmp_e, tmp_a, mask);
}

class PA_RGBA_MRGBA_Convert_AVX_ParallelTask : public ParallelTask {
public:
    PA_RGBA_MRGBA_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr,
            int src_w)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w) {}

    void operator() (const Range& range) const override {
        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;
        __m256i vec_128 = _mm256_set1_epi16(128 + 1);
        // __m256i u8_mask = _mm256_set_epi8(
        //     -1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0,
        //     -1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);

        __m256i blend_mask = _mm256_set_epi8(
            15, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0,
            15, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0);

        Deinterleave_4c_Avx tool;
        const uint8_t* ptr_src = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src = ptr_src;
            uint8_t* cur_dst = ptr_dst;
            for (int j = 0; j < width_align16; j += 16) {
                __m128i vec_r_u8;
                __m128i vec_g_u8;
                __m128i vec_b_u8;
                __m128i vec_a_u8;
                // vld4_u8x16_avx(cur_src, &vec_r_u8, &vec_g_u8, &vec_b_u8, &vec_a_u8);
                tool.load(cur_src, &vec_r_u8, &vec_g_u8, &vec_b_u8, &vec_a_u8);
                __m256i vec_r_u16 = _mm256_cvtepu8_epi16(vec_r_u8);
                __m256i vec_g_u16 = _mm256_cvtepu8_epi16(vec_g_u8);
                __m256i vec_b_u16 = _mm256_cvtepu8_epi16(vec_b_u8);
                __m256i vec_a_u16 = _mm256_cvtepu8_epi16(vec_a_u8);

                __m256i vec_tmp_r_u16 = _mm256_add_epi16(_mm256_mullo_epi16(vec_r_u16, vec_a_u16), vec_128);
                __m256i vec_tmp_g_u16 = _mm256_add_epi16(_mm256_mullo_epi16(vec_g_u16, vec_a_u16), vec_128);
                __m256i vec_tmp_b_u16 = _mm256_add_epi16(_mm256_mullo_epi16(vec_b_u16, vec_a_u16), vec_128);

                __m256i vec_dst_r_u16 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_srli_epi16(vec_tmp_r_u16, 8), vec_tmp_r_u16), 8);
                __m256i vec_dst_g_u16 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_srli_epi16(vec_tmp_g_u16, 8), vec_tmp_g_u16), 8);
                __m256i vec_dst_b_u16 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_srli_epi16(vec_tmp_b_u16, 8), vec_tmp_b_u16), 8);

                __m256i vec_dst_r_b = _mm256_packus_epi16(vec_dst_r_u16, vec_dst_b_u16);
                __m256i vec_dst_g_a = _mm256_packus_epi16(vec_dst_g_u16, vec_a_u16);

                __m256i blend_r_b = _mm256_shuffle_epi8(vec_dst_r_b, blend_mask);
                __m256i blend_g_a = _mm256_shuffle_epi8(vec_dst_g_a, blend_mask);

                __m256i bgra_0_2 = _mm256_unpacklo_epi8(blend_r_b, blend_g_a);
                __m256i bgra_1_3 = _mm256_unpackhi_epi8(blend_r_b, blend_g_a);

                _mm256_storeu_si256((__m256i*)cur_dst, _mm256_permute2f128_si256(bgra_0_2, bgra_1_3, 0b00100000));
                _mm256_storeu_si256((__m256i*)(cur_dst + 32), 
                        _mm256_permute2f128_si256(bgra_0_2, bgra_1_3, 0b00110001));

                // __m128i vec_dst_r_u8 = _mm256_castsi256_si128(_mm256_permute4x64_epi64(vec_dst_r_g, 0b00001000));
                // __m128i vec_dst_g_u8 = _mm256_castsi256_si128(_mm256_permute4x64_epi64(vec_dst_r_g, 0b00001101));
                // __m128i vec_dst_b_u8 = _mm256_castsi256_si128(
                //         _mm256_permute4x64_epi64(_mm256_packus_epi16(vec_dst_b_u16, vec_dst_b_u16), 0b00001000));
                // vst4_u8x16_avx(vec_dst_r_u8, vec_dst_g_u8, vec_dst_b_u8, vec_a_u8, cur_dst);
                cur_src += 64;
                cur_dst += 64;
            }
            for (int j = 0; j < remain; j++) {
                uint8_t r_val = cur_src[0];
                uint8_t g_val = cur_src[1];
                uint8_t b_val = cur_src[2];
                uint8_t a_val = cur_src[3];
                cur_dst[0] = (r_val * a_val + 128) / 255;
                cur_dst[1] = (g_val * a_val + 128) / 255;
                cur_dst[2] = (b_val * a_val + 128) / 255;
                cur_dst[3] = a_val;
                cur_src += 4;
                cur_dst += 4;
            }
            ptr_src += _src_stride;
            ptr_dst += _dst_stride;
        }
    }

    // void operator() (const Range& range) const override {
    void strategy (const Range& range) const {
        int width_align16 = _src_w & (~15);
        int remain = _src_w - width_align16;
        __m256i vec_128 = _mm256_set1_epi16(128 + 1);
        __m128i factor_mask = _mm_set1_epi32(0xff000000); 

        const uint8_t* ptr_src = _src_ptr + range.start() * _src_stride;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* cur_src = ptr_src;
            uint8_t* cur_dst = ptr_dst;
            for (int j = 0; j < width_align16; j += 16) {
                __m128i vec_src_0 = _mm_loadu_si128((__m128i const*)(cur_src +  0));
                __m128i vec_src_1 = _mm_loadu_si128((__m128i const*)(cur_src + 16));
                __m128i vec_src_2 = _mm_loadu_si128((__m128i const*)(cur_src + 32));
                __m128i vec_src_3 = _mm_loadu_si128((__m128i const*)(cur_src + 48));

                _mm_storeu_si128((__m128i*)(cur_dst +  0), calc_mrgba(vec_src_0, vec_128, factor_mask));
                _mm_storeu_si128((__m128i*)(cur_dst + 16), calc_mrgba(vec_src_1, vec_128, factor_mask));
                _mm_storeu_si128((__m128i*)(cur_dst + 32), calc_mrgba(vec_src_2, vec_128, factor_mask));
                _mm_storeu_si128((__m128i*)(cur_dst + 48), calc_mrgba(vec_src_3, vec_128, factor_mask));

                cur_src += 64;
                cur_dst += 64;
            }
            for (int j = 0; j < remain; j++) {
                uint8_t r_val = cur_src[0];
                uint8_t g_val = cur_src[1];
                uint8_t b_val = cur_src[2];
                uint8_t a_val = cur_src[3];
                cur_dst[0] = (r_val * a_val + 128) / 255;
                cur_dst[1] = (g_val * a_val + 128) / 255;
                cur_dst[2] = (b_val * a_val + 128) / 255;
                cur_dst[3] = a_val;
                cur_src += 4;
                cur_dst += 4;
            }
            ptr_src += _src_stride;
            ptr_dst += _dst_stride;
        }
    }
private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
};

static void convert_rgba_to_mrgba_avx(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();

    PA_RGBA_MRGBA_Convert_AVX_ParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, src_w);
    parallel_run(Range(0, src_h), task);
}

static void convert_yuv420_to_bgr_avx(
        const Mat& src,
        Mat& src_u,
        Mat& src_v,
        Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    const int width_align32 = src_w & (~31);

    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;

    const uint8_t *u = (const uint8_t *)src_u.data();
    const uint8_t *v = (const uint8_t *)src_v.data();

    const __m256i  vc32  = _mm256_set1_epi16(32);
    const __m128i  vc16  = _mm_set1_epi8(16);
    const __m256i  vc149  = _mm256_set1_epi16(149 / 2);
    const __m256i  vc102  = _mm256_set1_epi16(102);
    const __m256i vc14216 = _mm256_set1_epi16(14216);
    const __m256i  vc52   = _mm256_set1_epi16(52);
    const __m256i  vc25   = _mm256_set1_epi16(25);
    const __m256i vc8696  = _mm256_set1_epi16(8696);
    const __m256i  vc129  = _mm256_set1_epi16(129);
    const __m256i vc17672 = _mm256_set1_epi16(17672);
    const __m256i zero_256 = _mm256_setzero_si256();
    const __m256i vc255   = _mm256_set1_epi16(255);

    __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);

    // __m128i bgr_blend_mask = _mm_setr_epi8(0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);
    __m256i bgr_blend_mask = _mm256_setr_epi8(
            0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15, 
            16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

    for (int i = 0; i < src_h; i += 2) {
        const uint8_t *y0_ptr = src_ptr;
        const uint8_t *y1_ptr = src_ptr + src_w;

        uint8_t *dst0_ptr = dst_ptr ;
        uint8_t *dst1_ptr = dst_ptr + dst_stride;
        const uint8_t *u0 = u;
        const uint8_t *v0 = v;
        
        for (int j = 0; j < width_align32; j += 32) {
            __m128i y0_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y0_ptr), vc16);
            __m128i y0_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y0_ptr + 16)), vc16);
            __m128i y0_low_split = _mm_shuffle_epi8(y0_low_u8, split_mask);
            __m128i y0_high_split = _mm_shuffle_epi8(y0_high_u8, split_mask);
            __m256i y0_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y0_low_split, y0_high_split));
            __m256i y0_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y0_low_split, y0_high_split));

            __m128i y1_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y1_ptr), vc16);
            __m128i y1_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y1_ptr + 16)), vc16);
            __m128i y1_low_split = _mm_shuffle_epi8(y1_low_u8, split_mask);
            __m128i y1_high_split = _mm_shuffle_epi8(y1_high_u8, split_mask);
            __m256i y1_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y1_low_split, y1_high_split));
            __m256i y1_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y1_low_split, y1_high_split));

            __m256i u_i16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i const*)u0));
            __m256i v_i16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i const*)v0));

            __m256i v_ub = _mm256_mullo_epi16(u_i16, vc129);
            __m256i v_vr = _mm256_mullo_epi16(v_i16, vc102);

            __m256i v_ug = _mm256_mullo_epi16(u_i16, vc25);
            __m256i v_vg = _mm256_mullo_epi16(v_i16, vc52);
            __m256i v_uvg = _mm256_add_epi16(v_ug, v_vg);

            __m256i ub = _mm256_sub_epi16(v_ub, vc17672);
            __m256i vr = _mm256_sub_epi16(v_vr, vc14216);
            __m256i uvg = _mm256_sub_epi16(vc8696, v_uvg);

            __m256i y00 = _mm256_mullo_epi16(y0_even, vc149);
            __m256i y01 = _mm256_mullo_epi16(y0_odd, vc149);
            __m256i y10 = _mm256_mullo_epi16(y1_even, vc149);
            __m256i y11 = _mm256_mullo_epi16(y1_odd, vc149);
            // first row
            __m256i v_b0 = _mm256_add_epi16(_mm256_add_epi16(y00, ub), vc32);
            __m256i v_g0 = _mm256_add_epi16(_mm256_add_epi16(y00, uvg), vc32);
            __m256i v_r0 = _mm256_add_epi16(_mm256_add_epi16(y00, vr), vc32);

            __m256i v_b1 = _mm256_add_epi16(_mm256_add_epi16(y01, ub), vc32);
            __m256i v_g1 = _mm256_add_epi16(_mm256_add_epi16(y01, uvg), vc32);
            __m256i v_r1 = _mm256_add_epi16(_mm256_add_epi16(y01, vr), vc32);

            __m256i b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
            __m256i g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
            __m256i r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

            __m256i b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
            __m256i g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
            __m256i r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

            __m256i b0 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
            __m256i g0 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
            __m256i r0 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);
            // second row
            v_b0 = _mm256_add_epi16(_mm256_add_epi16(y10, ub), vc32);
            v_g0 = _mm256_add_epi16(_mm256_add_epi16(y10, uvg), vc32);
            v_r0 = _mm256_add_epi16(_mm256_add_epi16(y10, vr), vc32);

            v_b1 = _mm256_add_epi16(_mm256_add_epi16(y11, ub), vc32);
            v_g1 = _mm256_add_epi16(_mm256_add_epi16(y11, uvg), vc32);
            v_r1 = _mm256_add_epi16(_mm256_add_epi16(y11, vr), vc32);

            b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
            g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
            r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

            b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
            g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
            r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

            __m256i b1 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
            __m256i g1 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
            __m256i r1 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);

            vst3_u8x32_avx(b0, g0, r0, dst0_ptr);
            vst3_u8x32_avx(b1, g1, r1, dst1_ptr);

            y0_ptr   += 32;
            dst0_ptr += 96;
            y1_ptr   += 32;
            dst1_ptr += 96;
            u0 += 16;
            v0 += 16;
        }

        if (dst_w - width_align32) {
            int uv_off = width_align32 >> 1;
            convet_yuv420_to_bgr_one_row(src_ptr + width_align32, dst_ptr + width_align32 * 3,
                    u + uv_off, v + uv_off, dst_w, src_stride, dst_stride, width_align32, 3);
        }

        src_ptr += doub_src_stride;
        dst_ptr += doub_dst_stride;
        u += (src_stride >> 1);
        v += (src_stride >> 1);
    }
}

static void convert_yuv420p_bgr_random(
        const uint8_t* src_y, 
        const uint8_t* src_u, 
        const uint8_t* src_v, 
        uint8_t* dst_bgr, 
        int y_stride, 
        int bgr_stride, 
        int process_pixel_num, 
        bool rgb_mode) {
    const uint8_t* ptr_y0 = src_y;
    const uint8_t* ptr_y1 = src_y + y_stride;
    const uint8_t* ptr_u0 = src_u;
    const uint8_t* ptr_v0 = src_v;
    uint8_t* ptr_dst0 = dst_bgr;
    uint8_t* ptr_dst1 = dst_bgr + bgr_stride;
    int b_idx = rgb_mode ? 2 : 0;
    int r_idx = rgb_mode ? 0 : 2;
    for (int i = 0; i < process_pixel_num; i += 2) {
        uint8_t y00 = ptr_y0[0];
        uint8_t y01 = ptr_y0[1];
        uint8_t y10 = ptr_y1[0];
        uint8_t y11 = ptr_y1[1];

        uint8_t u0 = ptr_u0[0];
        uint8_t v0 = ptr_v0[0];

        ptr_y0 += 2;
        ptr_y1 += 2;
        ++u0;
        ++v0;

        int vr = v0 * 102 - 14216;
        int ub = 129 * u0 - 17672;
        int uvg = 8696 - 52 * v0 - 25 * u0;

        convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, ptr_dst0, b_idx, r_idx, 3);
        ptr_dst0 += 3;
        convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, ptr_dst0, b_idx, r_idx, 3);
        ptr_dst0 += 3;
        convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, ptr_dst1, b_idx, r_idx, 3);
        ptr_dst1 += 3;
        convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, ptr_dst1, b_idx, r_idx, 3);
        ptr_dst1 += 3;
    }
    return;
}

class I420_PA_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    I420_PA_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr, 
            int src_w,
            int src_h,
            bool yv12_mode, 
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w), 
            _src_h(src_h), 
            _yv12_mode(yv12_mode), 
            _rgb_mode(rgb_mode) {
        if (_rgb_mode) {
            _r_idx = 0;
            // g_idx = 1;
            _b_idx = 2;
        } else {
            _b_idx = 0;
            // g_idx = 1;
            _r_idx = 2;
        }
    }

    void operator() (const Range& range) const override {
        const __m128i  vc16  = _mm_set1_epi8(16);
        const __m256i  vc149  = _mm256_set1_epi16(149);
        const __m256i  vc102  = _mm256_set1_epi16(102);
        const __m256i vc14248 = _mm256_set1_epi16(14248);
        const __m256i  vc52   = _mm256_set1_epi16(52);
        const __m256i  vc25   = _mm256_set1_epi16(25);
        const __m256i vc8696  = _mm256_set1_epi16(8696);
        const __m256i  vc129  = _mm256_set1_epi16(129);
        const __m256i vc17672 = _mm256_set1_epi16(17672);
        const __m256i zero_256 = _mm256_setzero_si256();
        const __m256i vc255   = _mm256_set1_epi16(255);

        __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
        __m256i bgr_blend_mask = _mm256_setr_epi8(
                0,  8,  1,  9,  2, 10,  3, 11,  4, 12,  5, 13,  6, 14,  7, 15, 
                16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

        const uint8_t* base_y_data = _src_ptr;
        const uint8_t* base_p1_data = base_y_data + _src_stride * _src_h;
        const uint8_t* base_p2_data = base_p1_data + (_src_stride >> 1) * (_src_h >> 1) ;
        const uint8_t* base_u_data = _yv12_mode ? base_p2_data : base_p1_data;
        const uint8_t* base_v_data = _yv12_mode ? base_p1_data : base_p2_data;
        uint8_t* base_bgr_data = _dst_ptr;
        // each range step handle 2 rows data
        int src_double_stride = _src_stride << 1;
        int dst_double_stride = _dst_stride << 1;
        int src_half_stride = _src_stride >> 1;
        const uint8_t* cur_y_data = base_y_data + range.start() * src_double_stride;
        const uint8_t* cur_u_data = base_u_data + range.start() * src_half_stride;
        const uint8_t* cur_v_data = base_v_data + range.start() * src_half_stride;
        uint8_t* cur_bgr_data = base_bgr_data + range.start() * dst_double_stride;

        const int width_align32 = _src_w & (~31);
        const int remain = _src_w - width_align32;

        for (int i = range.start(); i < range.end(); i++) {
            const uint8_t* y0_ptr = cur_y_data;
            const uint8_t* y1_ptr = cur_y_data + _src_stride;
            const uint8_t* u_ptr = cur_u_data;
            const uint8_t* v_ptr = cur_v_data;
            uint8_t *dst0_ptr = cur_bgr_data;
            uint8_t *dst1_ptr = cur_bgr_data + _dst_stride;

            for (int j = 0; j < width_align32; j += 32) {
                __m128i y0_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y0_ptr), vc16);
                __m128i y0_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y0_ptr + 16)), vc16);
                __m128i y0_low_split = _mm_shuffle_epi8(y0_low_u8, split_mask);
                __m128i y0_high_split = _mm_shuffle_epi8(y0_high_u8, split_mask);
                __m256i y0_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y0_low_split, y0_high_split));
                __m256i y0_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y0_low_split, y0_high_split));

                __m128i y1_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y1_ptr), vc16);
                __m128i y1_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y1_ptr + 16)), vc16);
                __m128i y1_low_split = _mm_shuffle_epi8(y1_low_u8, split_mask);
                __m128i y1_high_split = _mm_shuffle_epi8(y1_high_u8, split_mask);
                __m256i y1_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y1_low_split, y1_high_split));
                __m256i y1_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y1_low_split, y1_high_split));

                __m256i u_i16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i const*)u_ptr));
                __m256i v_i16 = _mm256_cvtepu8_epi16(_mm_loadu_si128((__m128i const*)v_ptr));

                __m256i v_ub = _mm256_mullo_epi16(u_i16, vc129);
                __m256i v_vr = _mm256_mullo_epi16(v_i16, vc102);

                __m256i v_ug = _mm256_mullo_epi16(u_i16, vc25);
                __m256i v_vg = _mm256_mullo_epi16(v_i16, vc52);
                __m256i v_uvg = _mm256_add_epi16(v_ug, v_vg);

                __m256i ub = _mm256_sub_epi16(v_ub, vc17672);
                __m256i vr = _mm256_sub_epi16(v_vr, vc14248);
                __m256i uvg = _mm256_sub_epi16(vc8696, v_uvg);

                __m256i y00 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_even, vc149), 1);
                __m256i y01 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_odd,  vc149), 1);
                __m256i y10 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_even, vc149), 1);
                __m256i y11 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_odd,  vc149), 1);
                // first row
                __m256i v_b0 = _mm256_add_epi16(y00,  ub);
                __m256i v_g0 = _mm256_add_epi16(y00, uvg);
                __m256i v_r0 = _mm256_add_epi16(y00,  vr);

                __m256i v_b1 = _mm256_add_epi16(y01,  ub);
                __m256i v_g1 = _mm256_add_epi16(y01, uvg);
                __m256i v_r1 = _mm256_add_epi16(y01,  vr);

                __m256i b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                __m256i g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                __m256i r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                __m256i b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                __m256i g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                __m256i r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b0 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g0 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r0 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);
                // second row
                v_b0 = _mm256_add_epi16(y10,  ub);
                v_g0 = _mm256_add_epi16(y10, uvg);
                v_r0 = _mm256_add_epi16(y10,  vr);

                v_b1 = _mm256_add_epi16(y11,  ub);
                v_g1 = _mm256_add_epi16(y11, uvg);
                v_r1 = _mm256_add_epi16(y11,  vr);

                b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b1 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g1 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r1 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);

                if (_rgb_mode) {
                    vst3_u8x32_avx(r0, g0, b0, dst0_ptr);
                    vst3_u8x32_avx(r1, g1, b1, dst1_ptr);
                } else {
                    vst3_u8x32_avx(b0, g0, r0, dst0_ptr);
                    vst3_u8x32_avx(b1, g1, r1, dst1_ptr);
                }

                y0_ptr += 32;
                y1_ptr += 32;
                u_ptr += 16;
                v_ptr += 16;
                dst0_ptr += 96;
                dst1_ptr += 96;
            }

            if (remain) {
                convert_yuv420p_bgr_random(y0_ptr, u_ptr, v_ptr, dst0_ptr,
                        _src_stride, _dst_stride, remain, _rgb_mode);
            }

            cur_y_data += src_double_stride;
            cur_u_data += src_half_stride;
            cur_v_data += src_half_stride;
            cur_bgr_data += dst_double_stride;
        }        
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _yv12_mode;
    bool _rgb_mode;
    int _b_idx;
    int _r_idx;
};

static void convert_yuv420_to_bgr_avx(
        const Mat& src,
        Mat& dst,
        bool yv12_mode,
        bool rgb_mode) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    int half_h = dst.height() / 2;
    I420_PA_BGR_Convert_AVX_ParallelTask task(src_stride, src_ptr,
            dst_stride, dst_ptr, src_w, src_h, yv12_mode, rgb_mode);
    parallel_run(Range(0, half_h), task);
}

class NV21_PA_BGR_Convert_AVX_ParallelTask : public ParallelTask {
public:
    NV21_PA_BGR_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr, 
            int src_w,
            int src_h,
            bool nv12_mode, 
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w), 
            _src_h(src_h), 
            _nv12_mode(nv12_mode), 
            _rgb_mode(rgb_mode) {
        if (_rgb_mode) {
            _r_idx = 0;
            // g_idx = 1;
            _b_idx = 2;
        } else {
            _b_idx = 0;
            // g_idx = 1;
            _r_idx = 2;
        }
    }

    void operator() (const Range& range) const override {
        const __m128i  vc16  = _mm_set1_epi8(16);
        const __m256i  vc149  = _mm256_set1_epi16(149);
        const __m256i  vc102  = _mm256_set1_epi16(102);
        const __m256i vc14248 = _mm256_set1_epi16(14248);
        const __m256i  vc52   = _mm256_set1_epi16(52);
        const __m256i  vc25   = _mm256_set1_epi16(25);
        const __m256i vc8696  = _mm256_set1_epi16(8696);
        const __m256i  vc129  = _mm256_set1_epi16(129);
        const __m256i vc17672 = _mm256_set1_epi16(17672);
        const __m256i zero_256 = _mm256_setzero_si256();
        const __m256i vc255   = _mm256_set1_epi16(255);

        __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
        __m256i bgr_blend_mask = _mm256_setr_epi8(
                 0,  8,  1,  9,  2, 10,  3, 11,  4, 12,  5, 13,  6, 14,  7, 15, 
                16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

        __m256i de_interleave_vu_mask = _mm256_setr_epi8(
                0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15,
                0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);

        const uint8_t* base_y_data = _src_ptr;
        const uint8_t* base_uv_data = _src_ptr + _src_stride * _src_h;
        uint8_t* base_bgr_data = _dst_ptr;
        // each range step handle 2 rows data
        int src_double_stride = _src_stride << 1;
        int dst_double_stride = _dst_stride << 1;
        const uint8_t* cur_y_data = base_y_data + range.start() * src_double_stride;
        const uint8_t* cur_uv_data = base_uv_data + range.start() * _src_stride;
        uint8_t* cur_bgr_data = base_bgr_data + range.start() * dst_double_stride;

        const int width_align32 = _src_w & (~31);
        const int remain = _src_w - width_align32;

        for (int i = range.start(); i < range.end(); i++) {
            const uint8_t* y0_ptr = cur_y_data;
            const uint8_t* y1_ptr = cur_y_data + _src_stride;
            const uint8_t* uv_ptr = cur_uv_data;
            uint8_t *dst0_ptr = cur_bgr_data;
            uint8_t *dst1_ptr = cur_bgr_data + _dst_stride;

            for (int j = 0; j < width_align32; j += 32) {
                __m128i y0_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y0_ptr), vc16);
                __m128i y0_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y0_ptr + 16)), vc16);
                __m128i y0_low_split = _mm_shuffle_epi8(y0_low_u8, split_mask);
                __m128i y0_high_split = _mm_shuffle_epi8(y0_high_u8, split_mask);
                __m256i y0_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y0_low_split, y0_high_split));
                __m256i y0_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y0_low_split, y0_high_split));

                __m128i y1_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y1_ptr), vc16);
                __m128i y1_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y1_ptr + 16)), vc16);
                __m128i y1_low_split = _mm_shuffle_epi8(y1_low_u8, split_mask);
                __m128i y1_high_split = _mm_shuffle_epi8(y1_high_u8, split_mask);
                __m256i y1_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y1_low_split, y1_high_split));
                __m256i y1_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y1_low_split, y1_high_split));

                __m256i vu_blend = _mm256_loadu_si256((__m256i const*)uv_ptr);
                __m256i vu_spilit = _mm256_permute4x64_epi64(_mm256_shuffle_epi8(vu_blend, de_interleave_vu_mask), 0b11011000);

                __m256i v_i16;
                __m256i u_i16;

                if (_nv12_mode) {
                    u_i16 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vu_spilit));
                    v_i16 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vu_spilit, 1));
                } else {
                    v_i16 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vu_spilit));
                    u_i16 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vu_spilit, 1));
                }

                __m256i v_ub = _mm256_mullo_epi16(u_i16, vc129);
                __m256i v_vr = _mm256_mullo_epi16(v_i16, vc102);

                __m256i v_ug = _mm256_mullo_epi16(u_i16, vc25);
                __m256i v_vg = _mm256_mullo_epi16(v_i16, vc52);
                __m256i v_uvg = _mm256_add_epi16(v_ug, v_vg);

                __m256i ub = _mm256_sub_epi16(v_ub, vc17672);
                __m256i vr = _mm256_sub_epi16(v_vr, vc14248);
                __m256i uvg = _mm256_sub_epi16(vc8696, v_uvg);

                __m256i y00 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_even, vc149), 1);
                __m256i y01 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_odd,  vc149), 1);
                __m256i y10 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_even, vc149), 1);
                __m256i y11 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_odd,  vc149), 1);
                // first row
                __m256i v_b0 = _mm256_add_epi16(y00,  ub);
                __m256i v_g0 = _mm256_add_epi16(y00, uvg);
                __m256i v_r0 = _mm256_add_epi16(y00,  vr);

                __m256i v_b1 = _mm256_add_epi16(y01,  ub);
                __m256i v_g1 = _mm256_add_epi16(y01, uvg);
                __m256i v_r1 = _mm256_add_epi16(y01,  vr);

                __m256i b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                __m256i g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                __m256i r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                __m256i b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                __m256i g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                __m256i r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b0 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g0 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r0 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);
                // second row
                v_b0 = _mm256_add_epi16(y10,  ub);
                v_g0 = _mm256_add_epi16(y10, uvg);
                v_r0 = _mm256_add_epi16(y10,  vr);

                v_b1 = _mm256_add_epi16(y11,  ub);
                v_g1 = _mm256_add_epi16(y11, uvg);
                v_r1 = _mm256_add_epi16(y11,  vr);

                b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b1 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g1 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r1 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);

                if (_rgb_mode) {
                    vst3_u8x32_avx(r0, g0, b0, dst0_ptr);
                    vst3_u8x32_avx(r1, g1, b1, dst1_ptr);
                } else {
                    vst3_u8x32_avx(b0, g0, r0, dst0_ptr);
                    vst3_u8x32_avx(b1, g1, r1, dst1_ptr);
                }

                y0_ptr += 32;
                y1_ptr += 32;
                uv_ptr += 32;
                dst0_ptr += 96;
                dst1_ptr += 96;
            }

            if (remain) {
                convet_yuv_to_one_row(y0_ptr, dst0_ptr, uv_ptr, _src_w, width_align32,
                        _src_stride, _dst_stride, _nv12_mode, _b_idx, _r_idx, 3);
            }

            cur_y_data += src_double_stride;
            cur_uv_data += _src_stride;
            cur_bgr_data += dst_double_stride;
        }        
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _nv12_mode;
    bool _rgb_mode;
    int _b_idx;
    int _r_idx;
};

static void convert_nv21_to_rgb_avx(
        const Mat& src,
        Mat& dst,
        bool nv12_mode,
        bool rgb_mode) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    int half_h = dst.height() / 2;
    NV21_PA_BGR_Convert_AVX_ParallelTask task(src_stride, src_ptr,
            dst_stride, dst_ptr, src_w, src_h, nv12_mode, rgb_mode);
    parallel_run(Range(0, half_h), task);
}

class NV21_PA_BGRA_Convert_AVX_ParallelTask : public ParallelTask {
public:
    NV21_PA_BGRA_Convert_AVX_ParallelTask(
            int src_stride,
            const uint8_t* src_ptr,
            int dst_stride,
            uint8_t* dst_ptr, 
            int src_w,
            int src_h,
            bool nv12_mode, 
            bool rgb_mode)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w), 
            _src_h(src_h), 
            _nv12_mode(nv12_mode), 
            _rgb_mode(rgb_mode) {
        if (_rgb_mode) {
            _r_idx = 0;
            // g_idx = 1;
            _b_idx = 2;
        } else {
            _b_idx = 0;
            // g_idx = 1;
            _r_idx = 2;
        }
    }

    void operator() (const Range& range) const override {
        const __m128i  vc16  = _mm_set1_epi8(16);
        const __m256i  vc149  = _mm256_set1_epi16(149);
        const __m256i  vc102  = _mm256_set1_epi16(102);
        const __m256i vc14248 = _mm256_set1_epi16(14248);
        const __m256i  vc52   = _mm256_set1_epi16(52);
        const __m256i  vc25   = _mm256_set1_epi16(25);
        const __m256i vc8696  = _mm256_set1_epi16(8696);
        const __m256i  vc129  = _mm256_set1_epi16(129);
        const __m256i vc17672 = _mm256_set1_epi16(17672);
        const __m256i zero_256 = _mm256_setzero_si256();
        const __m256i vc255   = _mm256_set1_epi16(255);
        const __m256i vec_a = _mm256_set1_epi8(char(255));

        __m128i split_mask = _mm_setr_epi8(0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);
        __m256i bgr_blend_mask = _mm256_setr_epi8(
                0,  8,  1,  9,  2, 10,  3, 11,  4, 12,  5, 13,  6, 14,  7, 15, 
                16, 24, 17, 25, 18, 26, 19, 27, 20, 28, 21, 29, 22, 30, 23, 31);

        __m256i de_interleave_vu_mask = _mm256_setr_epi8(
                0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15,
                0, 2, 4, 6, 8, 10, 12, 14, 1, 3, 5, 7, 9, 11, 13, 15);

        const uint8_t* base_y_data = _src_ptr;
        const uint8_t* base_uv_data = _src_ptr + _src_stride * _src_h;
        uint8_t* base_bgr_data = _dst_ptr;
        // each range step handle 2 rows data
        int src_double_stride = _src_stride << 1;
        int dst_double_stride = _dst_stride << 1;
        const uint8_t* cur_y_data = base_y_data + range.start() * src_double_stride;
        const uint8_t* cur_uv_data = base_uv_data + range.start() * _src_stride;
        uint8_t* cur_bgr_data = base_bgr_data + range.start() * dst_double_stride;

        const int width_align32 = _src_w & (~31);
        const int remain = _src_w - width_align32;

        for (int i = range.start(); i < range.end(); i++) {
            const uint8_t* y0_ptr = cur_y_data;
            const uint8_t* y1_ptr = cur_y_data + _src_stride;
            const uint8_t* uv_ptr = cur_uv_data;
            uint8_t* dst0_ptr = cur_bgr_data;
            uint8_t* dst1_ptr = cur_bgr_data + _dst_stride;

            for (int j = 0; j < width_align32; j += 32) {
                __m128i y0_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y0_ptr), vc16);
                __m128i y0_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y0_ptr + 16)), vc16);
                __m128i y0_low_split = _mm_shuffle_epi8(y0_low_u8, split_mask);
                __m128i y0_high_split = _mm_shuffle_epi8(y0_high_u8, split_mask);
                __m256i y0_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y0_low_split, y0_high_split));
                __m256i y0_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y0_low_split, y0_high_split));

                __m128i y1_low_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)y1_ptr), vc16);
                __m128i y1_high_u8 = _mm_max_epu8(_mm_loadu_si128((__m128i const*)(y1_ptr + 16)), vc16);
                __m128i y1_low_split = _mm_shuffle_epi8(y1_low_u8, split_mask);
                __m128i y1_high_split = _mm_shuffle_epi8(y1_high_u8, split_mask);
                __m256i y1_even = _mm256_cvtepu8_epi16(_mm_unpacklo_epi64(y1_low_split, y1_high_split));
                __m256i y1_odd = _mm256_cvtepu8_epi16(_mm_unpackhi_epi64(y1_low_split, y1_high_split));

                __m256i vu_blend = _mm256_loadu_si256((__m256i const*)uv_ptr);
                __m256i vu_spilit = _mm256_permute4x64_epi64(
                        _mm256_shuffle_epi8(vu_blend, de_interleave_vu_mask), 0b11011000);

                __m256i v_i16;
                __m256i u_i16;

                if (_nv12_mode) {
                    u_i16 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vu_spilit));
                    v_i16 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vu_spilit, 1));
                } else {
                    v_i16 = _mm256_cvtepu8_epi16(_mm256_castsi256_si128(vu_spilit));
                    u_i16 = _mm256_cvtepu8_epi16(_mm256_extracti128_si256(vu_spilit, 1));
                }

                __m256i v_ub = _mm256_mullo_epi16(u_i16, vc129);
                __m256i v_vr = _mm256_mullo_epi16(v_i16, vc102);

                __m256i v_ug = _mm256_mullo_epi16(u_i16, vc25);
                __m256i v_vg = _mm256_mullo_epi16(v_i16, vc52);
                __m256i v_uvg = _mm256_add_epi16(v_ug, v_vg);

                __m256i ub = _mm256_sub_epi16(v_ub, vc17672);
                __m256i vr = _mm256_sub_epi16(v_vr, vc14248);
                __m256i uvg = _mm256_sub_epi16(vc8696, v_uvg);

                __m256i y00 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_even, vc149), 1);
                __m256i y01 = _mm256_srli_epi16(_mm256_mullo_epi16(y0_odd,  vc149), 1);
                __m256i y10 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_even, vc149), 1);
                __m256i y11 = _mm256_srli_epi16(_mm256_mullo_epi16(y1_odd,  vc149), 1);
                // first row
                __m256i v_b0 = _mm256_add_epi16(y00,  ub);
                __m256i v_g0 = _mm256_add_epi16(y00, uvg);
                __m256i v_r0 = _mm256_add_epi16(y00,  vr);

                __m256i v_b1 = _mm256_add_epi16(y01,  ub);
                __m256i v_g1 = _mm256_add_epi16(y01, uvg);
                __m256i v_r1 = _mm256_add_epi16(y01,  vr);

                __m256i b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                __m256i g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                __m256i r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                __m256i b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                __m256i g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                __m256i r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b0 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g0 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r0 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);
                // second row
                v_b0 = _mm256_add_epi16(y10,  ub);
                v_g0 = _mm256_add_epi16(y10, uvg);
                v_r0 = _mm256_add_epi16(y10,  vr);

                v_b1 = _mm256_add_epi16(y11,  ub);
                v_g1 = _mm256_add_epi16(y11, uvg);
                v_r1 = _mm256_add_epi16(y11,  vr);

                b00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b0, 6), vc255), zero_256);
                g00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g0, 6), vc255), zero_256);
                r00 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r0, 6), vc255), zero_256);

                b01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_b1, 6), vc255), zero_256);
                g01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_g1, 6), vc255), zero_256);
                r01 = _mm256_max_epi16(_mm256_min_epi16(_mm256_srai_epi16(v_r1, 6), vc255), zero_256);

                __m256i b1 = _mm256_shuffle_epi8(_mm256_packus_epi16(b00, b01), bgr_blend_mask);
                __m256i g1 = _mm256_shuffle_epi8(_mm256_packus_epi16(g00, g01), bgr_blend_mask);
                __m256i r1 = _mm256_shuffle_epi8(_mm256_packus_epi16(r00, r01), bgr_blend_mask);

                if (_rgb_mode) {
                    vst4_u8x32_avx(r0, g0, b0, vec_a, dst0_ptr);
                    vst4_u8x32_avx(r1, g1, b1, vec_a, dst1_ptr);
                } else {
                    vst4_u8x32_avx(b0, g0, r0, vec_a, dst0_ptr);
                    vst4_u8x32_avx(b1, g1, r1, vec_a, dst1_ptr);
                }

                y0_ptr += 32;
                y1_ptr += 32;
                uv_ptr += 32;
                dst0_ptr += 128;
                dst1_ptr += 128;
            }

            if (remain) {
                convet_yuv_to_one_row(y0_ptr, dst0_ptr, uv_ptr, _src_w, width_align32,
                        _src_stride, _dst_stride, _nv12_mode, _b_idx, _r_idx, 4);
            }

            cur_y_data += src_double_stride;
            cur_uv_data += _src_stride;
            cur_bgr_data += dst_double_stride;
        }        
    }

private:
    int _src_stride;
    const uint8_t* _src_ptr;
    int _dst_stride;
    uint8_t* _dst_ptr;
    int _src_w;
    int _src_h;
    bool _nv12_mode;
    bool _rgb_mode;
    int _b_idx;
    int _r_idx;
};

static void convert_nv21_to_bgra_avx(
        const Mat& src,
        Mat& dst,
        bool nv12_mode,
        bool rgb_mode) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    uint8_t *dst_ptr = (uint8_t*)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    int half_h = dst.height() / 2;
    NV21_PA_BGRA_Convert_AVX_ParallelTask task(src_stride, src_ptr,
            dst_stride, dst_ptr, src_w, src_h, nv12_mode, rgb_mode);
    parallel_run(Range(0, half_h), task);
}

void convert_bgr_to_yuv420_avx(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();

    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    CHECK_CVT_SIZE((src_w == dst_w) && (src_h == dst_h));

    switch (cvt_type) {
    case ColorConvertType::CVT_PA_BGRA2NV12:
        bgra2nv21_avx(src, dst, false, true);
        break;
    case ColorConvertType::CVT_PA_BGRA2NV21:
        bgra2nv21_avx(src, dst, false, false);
        break;
    case ColorConvertType::CVT_PA_RGBA2NV12:
        bgra2nv21_avx(src, dst, true, true);
        break;
    case ColorConvertType::CVT_PA_RGBA2NV21:
        bgra2nv21_avx(src, dst, true, false);
        break;
    case ColorConvertType::CVT_PA_BGR2NV12:
        bgr2nv21_avx(src, dst, false, true);
        break;
    case ColorConvertType::CVT_PA_BGR2NV21:
        bgr2nv21_avx(src, dst, false, false);
        break;
    case ColorConvertType::CVT_PA_RGB2NV12:
        bgr2nv21_avx(src, dst, true, true);
        break;
    case ColorConvertType::CVT_PA_RGB2NV21:
        bgr2nv21_avx(src, dst, true, false);
        break;
    default:
        LOG_ERR("cvt type not support yet!");
        break;
    };
}

int cvt_color_avx(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_NV212PA_BGR:
        convert_nv21_to_rgb_avx(src, dst, false, false);
        break;
    case ColorConvertType::CVT_NV122PA_BGR:
        convert_nv21_to_rgb_avx(src, dst, true, false);
        break;
    case ColorConvertType::CVT_NV212PA_RGB:
        convert_nv21_to_rgb_avx(src, dst, false, true);
        break;
    case ColorConvertType::CVT_NV122PA_RGB:
        convert_nv21_to_rgb_avx(src, dst, true, true);
        break;

    case ColorConvertType::CVT_NV212PA_BGRA:
        convert_nv21_to_bgra_avx(src, dst, false, false);
        break;
    case ColorConvertType::CVT_NV122PA_BGRA:
        convert_nv21_to_bgra_avx(src, dst, true, false);
        break;
    case ColorConvertType::CVT_NV212PA_RGBA:
        convert_nv21_to_bgra_avx(src, dst, false, true);
        break;
    case ColorConvertType::CVT_NV122PA_RGBA:
        convert_nv21_to_bgra_avx(src, dst, true, true);
        break;

    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_avx(src, dst, false, false);
        break;

    case ColorConvertType::CVT_PA_BGR2NV12:
    case ColorConvertType::CVT_PA_BGR2NV21:
    case ColorConvertType::CVT_PA_RGB2NV12:
    case ColorConvertType::CVT_PA_RGB2NV21:
    case ColorConvertType::CVT_PA_BGRA2NV12:
    case ColorConvertType::CVT_PA_BGRA2NV21:
    case ColorConvertType::CVT_PA_RGBA2NV12:
    case ColorConvertType::CVT_PA_RGBA2NV21:
        convert_bgr_to_yuv420_avx(src, dst, cvt_type);
        break;
    case ColorConvertType::CVT_PA_BGR2GRAY:
        bgr2gray_avx(src, dst, false);
        break;
    case ColorConvertType::CVT_PA_RGB2GRAY:
        bgr2gray_avx(src, dst, true);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_RGB:
    case ColorConvertType::CVT_PA_RGB2PA_BGR:
        convert_bgr_to_rgb_avx(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_BGRA:
    case ColorConvertType::CVT_PA_RGB2PA_RGBA:
        convert_bgr_to_bgra_avx(src, dst, false);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_RGBA:
    case ColorConvertType::CVT_PA_RGB2PA_BGRA:
        convert_bgr_to_bgra_avx(src, dst, true);
        break;

    case ColorConvertType::CVT_PA_BGRA2PA_BGR:
    case ColorConvertType::CVT_PA_RGBA2PA_RGB:
        convert_bgra_to_bgr_avx(src, dst, false);
        break;
    case ColorConvertType::CVT_PA_BGRA2PA_RGB:
    case ColorConvertType::CVT_PA_RGBA2PA_BGR:
        convert_bgra_to_bgr_avx(src, dst, true);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_BGRA:
    case ColorConvertType::CVT_PA_BGRA2PA_RGBA:
        convert_bgra_to_rgba_avx(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_RGB:
    case ColorConvertType::CVT_GRAY2PA_BGR:
        convert_gray_to_bgr_avx(src, dst);
        break;
    case ColorConvertType::CVT_GRAY2PA_RGBA:
    case ColorConvertType::CVT_GRAY2PA_BGRA:
        convert_gray_to_bgra_avx(src, dst);
        break;

    case ColorConvertType::CVT_PL_BGR2PA_BGR:
        convert_planar_to_package_c3_avx(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGR2PL_BGR:
        convert_package_to_planar_c3_avx(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_BGR565:
        convert_gray_to_bgr565_avx(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGR2PA_BGR565:
        convert_bgr_to_bgr565_avx(src, dst, false);
        break;
    case ColorConvertType::CVT_PA_RGB2PA_BGR565:
        convert_bgr_to_bgr565_avx(src, dst, true);
        break;
    case ColorConvertType::CVT_PA_BGRA2PA_BGR565:
        convert_bgra_to_bgr565_avx(src, dst, false);
        break;
    case ColorConvertType::CVT_PA_RGBA2PA_BGR565:
        convert_bgra_to_bgr565_avx(src, dst, true);
        break;
    case ColorConvertType::CVT_PA_RGBA2PA_mRGBA:
        convert_rgba_to_mrgba_avx(src, dst);
        break;
    default:
        LOG_ERR("cvt type not support yet!");
        break;
    };

    return 0;
}

int cvt_color_avx(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_avx(src_y, src_u, src_v, dst);
        break;
    default:
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
