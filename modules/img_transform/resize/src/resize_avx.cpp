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

#include "modules/img_transform/resize/include/resize_avx.h"

#include <cmath>
#include <immintrin.h>
#ifdef ELDER_COMPILER
#include "immintrin_extend.h"
#endif

#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/utils.h"
#include "modules/core/base/include/type_info.h"
#include "modules/img_transform/resize/include/resize_common.h"
G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void vertical_resize_bilinear_u16_avx(
        const int* row0,
        const int* row1,
        int src_width_align8,
        int src_w,
        int b0,
        int b1,
        uint8_t* dst) {
    __m256i v_mask = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0, 
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0);
    __m256i v_half = _mm256_set1_epi32(int(1 << 17));
    __m256i v_b0 = _mm256_set1_epi32(b0);
    __m256i v_b1 = _mm256_set1_epi32(b1);
    int dx = 0;
    for (; dx < src_width_align8; dx += 8) {
        __m256i v_s00_s32 = _mm256_srli_epi32(_mm256_loadu_si256((__m256i const*)row0), 4);
        __m256i v_s01_s32 = _mm256_srli_epi32(_mm256_loadu_si256((__m256i const*)row1), 4);

        __m256i v_product = _mm256_add_epi32(_mm256_mullo_epi32(v_s00_s32, v_b0), _mm256_mullo_epi32(v_s01_s32, v_b1));
        __m256i rows_u32 = _mm256_srli_epi32(_mm256_add_epi32(v_product, v_half), 18);

        __m256i rows_u8 = _mm256_shuffle_epi8(rows_u32, v_mask);
        _mm_storeu_si32(dst, _mm256_castsi256_si128(rows_u8));
        _mm_storeu_si32(dst + 4, _mm256_extractf128_si256(rows_u8, 1));
        dst  += 8;
        row0 += 8;
        row1 += 8;
    }
    for (; dx < src_w; dx++) {
        *dst++ = (*(row0++) * b0 + *(row1++) * b1 + (1 << 17)) >> 22;
    }
}

class ResizeBilinearC1AVXParallelTask : public ParallelTask {
public:
    ResizeBilinearC1AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_w & (~7);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint16_t* alpha = (const uint16_t*)(yofs + _dst_h);
        const uint16_t* beta  = (const uint16_t*)(alpha + _dst_w + _dst_w);

        int* rows = (int*)malloc(_dst_stride * sizeof(int) * 2);
        int* rows0 = rows;
        int* rows1 = rows + _dst_stride;

        int prev_sy1 = -1;
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint16_t* cur_beta = beta + (range.start() << 1);
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint16_t *alphap = alpha;
            if (sy0 == prev_sy1 && valid_rows1) {
                int* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                int dx = 0;
                for (; dx < dst_width_align8; dx += 8) {
                    __m256i v_a_s16 = _mm256_loadu_si256((__m256i const*)alphap);
                    alphap += 16;

                    __m128i v_s1_u8 = _mm_set_epi16(
                            *((uint16_t*)(src1 + xofs[dx + 7])), 
                            *((uint16_t*)(src1 + xofs[dx + 6])), 
                            *((uint16_t*)(src1 + xofs[dx + 5])), 
                            *((uint16_t*)(src1 + xofs[dx + 4])), 
                            *((uint16_t*)(src1 + xofs[dx + 3])), 
                            *((uint16_t*)(src1 + xofs[dx + 2])), 
                            *((uint16_t*)(src1 + xofs[dx + 1])), 
                            *((uint16_t*)(src1 + xofs[dx])));
                    __m256i v_s1_s16 = _mm256_cvtepu8_epi16(v_s1_u8);

                    __m256i v_rows1_s32 = _mm256_madd_epi16(v_s1_s16, v_a_s16);
                    _mm256_storeu_si256((__m256i*)(rows1 + dx), v_rows1_s32);
                }
                for (; dx < _dst_w; dx++) {
                    int a0 = int(*alphap++);
                    int a1 = int(*alphap++);
                    int data_0 = int(*(src1 + xofs[dx]));
                    int data_1 = int(*(src1 + xofs[dx] + 1));
                    rows1[dx] = data_0 * a0 + data_1 * a1;
                }
            } else {
                // hresize two rows
                const uint8_t *src0 = _src_ptr + sy_off;
                const uint8_t *src1 = _src_ptr + sy_off + _src_stride;
                int dx = 0;
                for (; dx < dst_width_align8; dx += 8) {
                    __m256i v_a_s16 = _mm256_loadu_si256((__m256i const*)alphap);
                    alphap += 16;

                    __m128i v_s0_u8 = _mm_set_epi16(
                            *((uint16_t*)(src0 + xofs[dx + 7])), 
                            *((uint16_t*)(src0 + xofs[dx + 6])), 
                            *((uint16_t*)(src0 + xofs[dx + 5])), 
                            *((uint16_t*)(src0 + xofs[dx + 4])), 
                            *((uint16_t*)(src0 + xofs[dx + 3])), 
                            *((uint16_t*)(src0 + xofs[dx + 2])), 
                            *((uint16_t*)(src0 + xofs[dx + 1])), 
                            *((uint16_t*)(src0 + xofs[dx])));
                    __m256i v_s0_s16 = _mm256_cvtepu8_epi16(v_s0_u8);
                    __m256i v_rows0_s32 = _mm256_madd_epi16(v_s0_s16, v_a_s16);
                    _mm256_storeu_si256((__m256i*)(rows0 + dx), v_rows0_s32);

                    __m128i v_s1_u8 = _mm_set_epi16(
                            *((uint16_t*)(src1 + xofs[dx + 7])), 
                            *((uint16_t*)(src1 + xofs[dx + 6])), 
                            *((uint16_t*)(src1 + xofs[dx + 5])), 
                            *((uint16_t*)(src1 + xofs[dx + 4])), 
                            *((uint16_t*)(src1 + xofs[dx + 3])), 
                            *((uint16_t*)(src1 + xofs[dx + 2])), 
                            *((uint16_t*)(src1 + xofs[dx + 1])), 
                            *((uint16_t*)(src1 + xofs[dx])));
                    __m256i v_s1_s16 = _mm256_cvtepu8_epi16(v_s1_u8);
                    __m256i v_rows1_s32 = _mm256_madd_epi16(v_s1_s16, v_a_s16);
                    _mm256_storeu_si256((__m256i*)(rows1 + dx), v_rows1_s32);
                }
                for (; dx < _dst_w; dx++) {
                    int a0 = int(*alphap++);
                    int a1 = int(*alphap++);

                    int data_0 = int(*(src0 + xofs[dx]));
                    int data_1 = int(*(src0 + xofs[dx] + 1));
                    rows0[dx] = data_0 * a0 + data_1 * a1;
                    
                    int data_2 = int(*(src1 + xofs[dx]));
                    int data_3 = int(*(src1 + xofs[dx] + 1));
                    rows1[dx] = data_2 * a0 + data_3 * a1;
                }
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            int b0 = *cur_beta++;
            int b1 = *cur_beta++;
            vertical_resize_bilinear_u16_avx(rows0, rows1,
                    dst_width_align8, _dst_w, b0, b1, ptr_dst);
            ptr_dst += _dst_stride;
        }
        free(rows);
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c1_comm_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 1, &buf);

    ResizeBilinearC1AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

class HalfResizeBilinearC1AVXParallelTask : public ParallelTask {
public:
    HalfResizeBilinearC1AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;
        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        __m256i v_half = _mm256_set1_epi16(2);
        __m256i v_mask = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0,
                                         -1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);
        int dst_aligned = _dst_w & (~15);
        __m256i v_shift_mask = _mm256_set1_epi16(0x00ff);
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_aligned; dx += 16) {
                __m256i v_s0_s8 = _mm256_loadu_si256((__m256i const*)S00);
                __m256i v_s1_s8 = _mm256_loadu_si256((__m256i const*)S01);

                __m256i v_s00_s16 = _mm256_and_si256(v_s0_s8, v_shift_mask);
                __m256i v_s01_s16 = _mm256_srli_epi16(v_s0_s8, 8);
                __m256i v_s10_s16 = _mm256_and_si256(v_s1_s8, v_shift_mask);
                __m256i v_s11_s16 = _mm256_srli_epi16(v_s1_s8, 8);

                __m256i v_sum0_s16 = _mm256_add_epi16(_mm256_add_epi16(v_s00_s16, v_s01_s16), v_half);
                __m256i v_sum_s16 = _mm256_add_epi16(_mm256_add_epi16(v_s10_s16, v_s11_s16), v_sum0_s16);
                __m256i v_res_s16 = _mm256_srli_epi16(v_sum_s16, 2);
                __m256i v_res_s8 = _mm256_permute4x64_epi64(_mm256_shuffle_epi8(v_res_s16, v_mask), 0b00001000);
                _mm_storeu_si128((__m128i*)(dst0 + dx), _mm256_castsi256_si128(v_res_s8));
                S00 += 32;
                S01 += 32;
            }
            for (; dx < _dst_w; dx++) {
                dst0[dx] = uint8_t((S00[0] + S00[1] + S01[0] + S01[1] + 2) >> 2);
                S00 += 2;
                S01 += 2;
            }
            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c1_dn2x_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();
    HalfResizeBilinearC1AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c1_avx(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;
    const double diff = 1e-6;
    if (fabs(scale_x - scale_y) < diff) {
        if (fabs(scale_x - 2.f) < diff) {
            return resize_bilinear_c1_dn2x_avx(src, dst);
        }
    }
    return resize_bilinear_c1_comm_avx(src, dst);
}

class ResizeBilinearC3AVXParallelTask : public ParallelTask {
public:
    ResizeBilinearC3AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        int dst_w_1 = _dst_w - 1;
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint16_t* alpha = (const uint16_t*)(yofs + _dst_h);
        const uint16_t* beta  = (const uint16_t*)(alpha + _dst_w + _dst_w);

        int* rows = (int*)malloc(_dst_stride * sizeof(int) * 2);
        int* rows0 = rows;
        int* rows1 = rows + _dst_stride;

        int prev_sy1 = -1;
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint16_t* cur_beta = beta + (range.start() << 1);
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint16_t *alphap = alpha;
            if (sy0 == prev_sy1 && valid_rows1) {
                int* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                int dx = 0;
                for (; dx < dst_w_1; ++dx) {
                    int idx = dx * 3;
                    const uint8_t* S1p = src1 + xofs[dx];
                    __m128i v_a_s16 = _mm_set1_epi32(*((int*)alphap));
                    alphap += 2;
                    __m128i v_s1_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S1p)));
                    __m128i v_s1_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S1p + 3))));
                    __m128i v_s1_s16 = _mm_or_si128(_mm_slli_epi32(v_s1_high_s32, 16), v_s1_low_s32);
                    __m128i v_rows1_s32 = _mm_madd_epi16(v_s1_s16, v_a_s16);
                    _mm_storeu_si128((__m128i*)(rows1 + idx), v_rows1_s32);
                }
                int idx = dst_w_1 + (dst_w_1 << 1);
                uint16_t a0 = *alphap++;
                uint16_t a1 = *alphap++;
                const uint8_t* S1p = src1 + xofs[dst_w_1];
                rows1[idx] = S1p[0] * a0 + S1p[3] * a1;
                rows1[idx + 1] = S1p[1] * a0 + S1p[4] * a1;
                rows1[idx + 2] = S1p[2] * a0 + S1p[5] * a1;
            } else {
                // hresize two rows
                const uint8_t *src0 = _src_ptr + sy_off;
                const uint8_t *src1 = _src_ptr + sy_off + _src_stride;

                int* rows0p = rows0;
                int* rows1p = rows1;
                int dx = 0;
                for (; dx < dst_w_1; ++dx) {
                    int idx = dx + (dx << 1);
                    const uint8_t* S0p = src0 + xofs[dx];
                    const uint8_t* S1p = src1 + xofs[dx];
                    __m128i v_a_s16 = _mm_set1_epi32(*((int*)alphap));
                    alphap += 2;

                    __m128i v_s0_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S0p)));
                    __m128i v_s0_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S0p + 3))));
                    __m128i v_s0_s16 = _mm_or_si128(_mm_slli_epi32(v_s0_high_s32, 16), v_s0_low_s32);
                    __m128i v_rows0_s32 = _mm_madd_epi16(v_s0_s16, v_a_s16);

                    __m128i v_s1_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S1p)));
                    __m128i v_s1_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S1p + 3))));
                    __m128i v_s1_s16 = _mm_or_si128(_mm_slli_epi32(v_s1_high_s32, 16), v_s1_low_s32);
                    __m128i v_rows1_s32 = _mm_madd_epi16(v_s1_s16, v_a_s16);

                    _mm_storeu_si128((__m128i*)(rows0p + idx), v_rows0_s32);
                    _mm_storeu_si128((__m128i*)(rows1p + idx), v_rows1_s32);
                }
                int idx = dst_w_1 + (dst_w_1 << 1);
                uint16_t a0 = *alphap++;
                uint16_t a1 = *alphap++;
                const uint8_t* S0p = src0 + xofs[dst_w_1];
                const uint8_t* S1p = src1 + xofs[dst_w_1];
                rows0p[idx] = S0p[0] * a0 + S0p[3] * a1;
                rows0p[idx + 1] = S0p[1] * a0 + S0p[4] * a1;
                rows0p[idx + 2] = S0p[2] * a0 + S0p[5] * a1;

                rows1p[idx] = S1p[0] * a0 + S1p[3] * a1;
                rows1p[idx + 1] = S1p[1] * a0 + S1p[4] * a1;
                rows1p[idx + 2] = S1p[2] * a0 + S1p[5] * a1;
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            int b0 = *cur_beta++;
            int b1 = *cur_beta++;
            vertical_resize_bilinear_u16_avx(rows0, rows1,
                    dst_width_align8, _dst_stride, b0, b1, ptr_dst);
            ptr_dst += _dst_stride;
        }
        free(rows);
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c3_comm_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 3, &buf);

    ResizeBilinearC3AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

class HalfResizeBilinearC3AVXParallelTask : public ParallelTask {
public:
    HalfResizeBilinearC3AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;
        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        __m256i v_half = _mm256_set1_epi16(2);
        __m256i v_mask_0_0 = _mm256_set_epi8(-1,  9, -1,  6, -1,  5, -1,  2, -1,  4, -1,  1, -1,  3, -1,  0, 
                -1,  9, -1,  6, -1,  5, -1,  2, -1,  4, -1,  1, -1,  3, -1,  0); // bgr0 - v0
        __m256i v_mask_0_1 = _mm256_set_epi8(-1, -1, -1, 13, -1, 15, -1, 12, -1, 11, -1,  8, -1, 10, -1,  7,
                -1, -1, -1, 13, -1, 15, -1, 12, -1, 11, -1,  8, -1, 10, -1,  7); // bgr0 - v1
        __m256i v_mask_1_1 = _mm256_set_epi8(-1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1); // bgr1 - v1
        __m256i v_mask_0_2 = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14); // bgr0 - v2
        __m256i v_mask_1_2 = _mm256_set_epi8(-1,  7, -1,  4, -1,  6, -1,  3, -1,  5, -1,  2, -1,  1, -1, -1,
                -1,  7, -1,  4, -1,  6, -1,  3, -1,  5, -1,  2, -1,  1, -1, -1); // bgr1 - v2
        __m256i v_mask_1_3 = _mm256_set_epi8(-1, -1, -1, 14, -1, 13, -1, 10, -1, 12, -1,  9, -1, 11, -1,  8,
                -1, -1, -1, 14, -1, 13, -1, 10, -1, 12, -1,  9, -1, 11, -1,  8); // bgr1 - v3
        __m256i v_mask_2_3 = _mm256_set_epi8(-1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1); // bgr2 - v3
        __m256i v_mask_1_4 = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15,
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15); // bgr1 - v4
        __m256i v_mask_2_4 = _mm256_set_epi8(-1,  8, -1,  5, -1,  7, -1,  4, -1,  3, -1,  0, -1,  2, -1, -1,
                -1,  8, -1,  5, -1,  7, -1,  4, -1,  3, -1,  0, -1,  2, -1, -1); // bgr2 - v4
        __m256i v_mask_2_5 = _mm256_set_epi8(-1, 15, -1, 12, -1, 14, -1, 11, -1, 13, -1, 10, -1,  9, -1,  6,
                -1, 15, -1, 12, -1, 14, -1, 11, -1, 13, -1, 10, -1,  9, -1,  6); // bgr2 - v5

        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            int dst_aligned = _dst_stride & (~47);
            for (; dx < dst_aligned; dx += 48) {
                // line 1
                __m256i bgr_u8_00 = _mm256_loadu2_m128i((__m128i const*)(S00 + 48), (__m128i const*)(S00 +  0));
                __m256i bgr_u8_01 = _mm256_loadu2_m128i((__m128i const*)(S00 + 64), (__m128i const*)(S00 + 16));
                __m256i bgr_u8_02 = _mm256_loadu2_m128i((__m128i const*)(S00 + 80), (__m128i const*)(S00 + 32));

                __m256i bgr_u16_00 = _mm256_shuffle_epi8(bgr_u8_00, v_mask_0_0); 
                __m256i bgr_u16_01 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_00, v_mask_0_1), _mm256_shuffle_epi8(bgr_u8_01, v_mask_1_1)); 
                __m256i bgr_u16_02 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_00, v_mask_0_2), _mm256_shuffle_epi8(bgr_u8_01, v_mask_1_2)); 
                __m256i bgr_u16_03 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_01, v_mask_1_3), _mm256_shuffle_epi8(bgr_u8_02, v_mask_2_3)); 
                __m256i bgr_u16_04 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_01, v_mask_1_4), _mm256_shuffle_epi8(bgr_u8_02, v_mask_2_4)); 
                __m256i bgr_u16_05 = _mm256_shuffle_epi8(bgr_u8_02, v_mask_2_5); 

                __m256i sum_u16_00 = _mm256_hadd_epi16(bgr_u16_00, bgr_u16_01);
                __m256i sum_u16_01 = _mm256_hadd_epi16(bgr_u16_02, bgr_u16_03);
                __m256i sum_u16_02 = _mm256_hadd_epi16(bgr_u16_04, bgr_u16_05);
                // line 2
                __m256i bgr_u8_10 = _mm256_loadu2_m128i((__m128i const*)(S01 + 48), (__m128i const*)(S01 +  0));
                __m256i bgr_u8_11 = _mm256_loadu2_m128i((__m128i const*)(S01 + 64), (__m128i const*)(S01 + 16));
                __m256i bgr_u8_12 = _mm256_loadu2_m128i((__m128i const*)(S01 + 80), (__m128i const*)(S01 + 32));

                __m256i bgr_u16_10 = _mm256_shuffle_epi8(bgr_u8_10, v_mask_0_0); 
                __m256i bgr_u16_11 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_10, v_mask_0_1), _mm256_shuffle_epi8(bgr_u8_11, v_mask_1_1)); 
                __m256i bgr_u16_12 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_10, v_mask_0_2), _mm256_shuffle_epi8(bgr_u8_11, v_mask_1_2)); 
                __m256i bgr_u16_13 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_11, v_mask_1_3), _mm256_shuffle_epi8(bgr_u8_12, v_mask_2_3)); 
                __m256i bgr_u16_14 = _mm256_or_si256(
                        _mm256_shuffle_epi8(bgr_u8_11, v_mask_1_4), _mm256_shuffle_epi8(bgr_u8_12, v_mask_2_4)); 
                __m256i bgr_u16_15 = _mm256_shuffle_epi8(bgr_u8_12, v_mask_2_5); 

                __m256i sum_u16_10 = _mm256_hadd_epi16(bgr_u16_10, bgr_u16_11);
                __m256i sum_u16_11 = _mm256_hadd_epi16(bgr_u16_12, bgr_u16_13);
                __m256i sum_u16_12 = _mm256_hadd_epi16(bgr_u16_14, bgr_u16_15);
                // calc
                __m256i res_u16_0_3 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(sum_u16_00, sum_u16_10), v_half), 2);
                __m256i res_u16_1_4 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(sum_u16_01, sum_u16_11), v_half), 2);
                __m256i res_u16_2_5 = _mm256_srli_epi16(
                        _mm256_add_epi16(_mm256_add_epi16(sum_u16_02, sum_u16_12), v_half), 2);

                __m256i res_u8_0_1_3_4 = _mm256_packus_epi16(res_u16_0_3, res_u16_1_4);
                __m256i res_u8_2_2_5_5 = _mm256_packus_epi16(res_u16_2_5, res_u16_2_5);

                _mm256_storeu2_m128i((__m128i*)(dst0 + 24), (__m128i*)dst0, res_u8_0_1_3_4);
                _mm_storeu_si64(dst0 + 16, _mm256_castsi256_si128(res_u8_2_2_5_5));
                _mm_storeu_si64(dst0 + 40, _mm256_extracti128_si256(res_u8_2_2_5_5, 1));
                
                S00 += 96;
                S01 += 96;
                dst0 += 48;
            }

            for (; dx < _dst_w; dx++) {
                dst0[0] = uint8_t((S00[0] + S00[3] + S01[0] + S01[3] + 2) >> 2);
                dst0[1] = uint8_t((S00[1] + S00[4] + S01[1] + S01[4] + 2) >> 2);
                dst0[2] = uint8_t((S00[2] + S00[5] + S01[2] + S01[5] + 2) >> 2);
                S00 += 6;
                S01 += 6;
                dst0 += 3;
            }
            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c3_dn2x_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();
    HalfResizeBilinearC3AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c3_avx(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;
    const double diff = 1e-6;
    if (fabs(scale_x - scale_y) < diff) {
        if (fabs(scale_x - 2.f) < diff) {
            return resize_bilinear_c3_dn2x_avx(src, dst);
        }
    }
    return resize_bilinear_c3_comm_avx(src, dst);
}

class ResizeBilinearC4AVXParallelTask : public ParallelTask {
public:
    ResizeBilinearC4AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        int dst_w_aligned = _dst_w & (~1);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint16_t* alpha = (const uint16_t*)(yofs + _dst_h);
        const uint16_t* beta  = (const uint16_t*)(alpha + _dst_w + _dst_w);

        int* rows = (int*)malloc(_dst_stride * sizeof(int) * 2);
        int* rows0 = rows;
        int* rows1 = rows + _dst_stride;

        int prev_sy1 = -1;
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint16_t* cur_beta = beta + (range.start() << 1);
        const __m256i v_s_mask_u16 = _mm256_set_epi8(-1, 7, -1, 3, -1, 6, -1, 2, -1, 5, -1, 1, -1, 4, -1, 0, 
                                                     -1, 7, -1, 3, -1, 6, -1, 2, -1, 5, -1, 1, -1, 4, -1, 0);
        const __m256i v_a_mask_u16 = _mm256_set_epi8(7, 6, 5, 4, 7, 6, 5, 4, 7, 6, 5, 4, 7, 6, 5, 4, 
                                                     3, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0, 3, 2, 1, 0);
    for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint16_t *alphap = alpha;
            if (sy0 == prev_sy1 && valid_rows1) {
                int* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                int dx = 0;
                // SSE implement block
                /* for (; dx < _dst_w; ++dx) {
                    int idx = dx << 2;
                    const uint8_t* S1p = src1 + xofs[dx];
                    __m128i v_a_s16 = _mm_set1_epi32(*((int*)alphap));
                    alphap += 2;
                    __m128i v_s1_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S1p)));
                    __m128i v_s1_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S1p + 4))));
                    __m128i v_s1_s16 = _mm_or_si128(_mm_slli_epi32(v_s1_high_s32, 16), v_s1_low_s32);
                    __m128i v_rows1_s32 = _mm_madd_epi16(v_s1_s16, v_a_s16);
                    _mm_storeu_si128((__m128i*)(rows1 + idx), v_rows1_s32);
                } */
                for (; dx < dst_w_aligned; dx += 2) {
                    int idx = dx << 2;
                    __m256i v_a_u8 = _mm256_set1_epi64x(*((__int64_t*)alphap));
                    alphap += 4;
                    __m256i v_a_u16 = _mm256_shuffle_epi8(v_a_u8, v_a_mask_u16);

                    const __int64_t* sp10 = (__int64_t*)(src1 + xofs[dx]);
                    const __int64_t* sp12 = (__int64_t*)(src1 + xofs[dx + 1]);
                    __m256i v_s1_u8 = _mm256_set_epi64x(0, *sp12, 0, *sp10);
                    __m256i v_s1_u16 = _mm256_shuffle_epi8(v_s1_u8, v_s_mask_u16);

                    __m256i v_rows1_s32 = _mm256_madd_epi16(v_s1_u16, v_a_u16);
                    _mm256_storeu_si256((__m256i*)(rows1 + idx), v_rows1_s32);
                }
                for (; dx < _dst_w; dx++) {
                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;
                    int idx = dx << 2;

                    const uint8_t* sp10 = src1 + xofs[dx];
                    rows1[idx] = a0 * sp10[0] + a1 + sp10[4]; 
                    rows1[idx + 1] = a0 * sp10[1] + a1 + sp10[5]; 
                    rows1[idx + 2] = a0 * sp10[2] + a1 + sp10[6]; 
                    rows1[idx + 3] = a0 * sp10[3] + a1 + sp10[7]; 
                }    
            } else {
                // hresize two rows
                const uint8_t *src0 = _src_ptr + sy_off;
                const uint8_t *src1 = _src_ptr + sy_off + _src_stride;

                int* rows0p = rows0;
                int* rows1p = rows1;
                int dx = 0;
                // SSE implement block
                /* for (; dx < _dst_w; ++dx) {
                    int idx = dx << 2;
                    const uint8_t* S0p = src0 + xofs[dx];
                    const uint8_t* S1p = src1 + xofs[dx];
                    __m128i v_a_s16 = _mm_set1_epi32(*((int*)alphap));
                    alphap += 2;

                    __m128i v_s0_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S0p)));
                    __m128i v_s0_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S0p + 4))));
                    __m128i v_s0_s16 = _mm_or_si128(_mm_slli_epi32(v_s0_high_s32, 16), v_s0_low_s32);
                    __m128i v_rows0_s32 = _mm_madd_epi16(v_s0_s16, v_a_s16);

                    __m128i v_s1_low_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)S1p)));
                    __m128i v_s1_high_s32 = _mm_cvtepu8_epi32(_mm_cvtsi32_si128(*((int*)(S1p + 4))));
                    __m128i v_s1_s16 = _mm_or_si128(_mm_slli_epi32(v_s1_high_s32, 16), v_s1_low_s32);
                    __m128i v_rows1_s32 = _mm_madd_epi16(v_s1_s16, v_a_s16);

                    _mm_storeu_si128((__m128i*)(rows0p + idx), v_rows0_s32);
                    _mm_storeu_si128((__m128i*)(rows1p + idx), v_rows1_s32);
                } */
                for (; dx < dst_w_aligned; dx += 2) {
                    int idx = dx << 2;
                    __m256i v_a_u8 = _mm256_set1_epi64x(*((__int64_t*)alphap));
                    alphap += 4;
                    __m256i v_a_u16 = _mm256_shuffle_epi8(v_a_u8, v_a_mask_u16);

                    const __int64_t* sp00 = (__int64_t*)(src0 + xofs[dx]);
                    const __int64_t* sp02 = (__int64_t*)(src0 + xofs[dx + 1]);
                    __m256i v_s0_u8 = _mm256_set_epi64x(0, *sp02, 0, *sp00);
                    __m256i v_s0_u16 = _mm256_shuffle_epi8(v_s0_u8, v_s_mask_u16);
                    __m256i v_rows0_s32 = _mm256_madd_epi16(v_s0_u16, v_a_u16);
                    _mm256_storeu_si256((__m256i*)(rows0p + idx), v_rows0_s32);

                    const __int64_t* sp10 = (__int64_t*)(src1 + xofs[dx]);
                    const __int64_t* sp12 = (__int64_t*)(src1 + xofs[dx + 1]);
                    __m256i v_s1_u8 = _mm256_set_epi64x(0, *sp12, 0, *sp10);
                    __m256i v_s1_u16 = _mm256_shuffle_epi8(v_s1_u8, v_s_mask_u16);
                    __m256i v_rows1_s32 = _mm256_madd_epi16(v_s1_u16, v_a_u16);
                    _mm256_storeu_si256((__m256i*)(rows1p + idx), v_rows1_s32);
                }
                for (; dx < _dst_w; dx++) {
                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;
                    int idx = dx << 2;

                    const uint8_t* sp00 = src0 + xofs[dx];
                    rows0p[idx] = a0 * sp00[0] + a1 + sp00[4]; 
                    rows0p[idx + 1] = a0 * sp00[1] + a1 + sp00[5]; 
                    rows0p[idx + 2] = a0 * sp00[2] + a1 + sp00[6]; 
                    rows0p[idx + 3] = a0 * sp00[3] + a1 + sp00[7]; 

                    const uint8_t* sp10 = src1 + xofs[dx];
                    rows1p[idx] = a0 * sp10[0] + a1 + sp10[4]; 
                    rows1p[idx + 1] = a0 * sp10[1] + a1 + sp10[5]; 
                    rows1p[idx + 2] = a0 * sp10[2] + a1 + sp10[6]; 
                    rows1p[idx + 3] = a0 * sp10[3] + a1 + sp10[7]; 
                }
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            int b0 = *cur_beta++;
            int b1 = *cur_beta++;
            vertical_resize_bilinear_u16_avx(rows0, rows1,
                    dst_width_align8, _dst_stride, b0, b1, ptr_dst);
            ptr_dst += _dst_stride;
        }
        free(rows);
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c4_comm_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);

    ResizeBilinearC4AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

class HalfResizeBilinearC4AVXParallelTask : public ParallelTask {
public:
    HalfResizeBilinearC4AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;
        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        __m256i v_half = _mm256_set1_epi16(2);
        __m256i v_mask = _mm256_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0,
                                         -1, -1, -1, -1, -1, -1, -1, -1, 14, 12, 10, 8, 6, 4, 2, 0);
        int dst_aligned = _dst_w & (~3);
        __m256i v_mask_even = _mm256_set_epi8(-1, 11, -1, 10, -1, 9, -1, 8, -1, 3, -1, 2, -1, 1, -1, 0,
                                              -1, 11, -1, 10, -1, 9, -1, 8, -1, 3, -1, 2, -1, 1, -1, 0);
        __m256i v_mask_odd = _mm256_set_epi8(-1, 15, -1, 14, -1, 13, -1, 12, -1, 7, -1, 6, -1, 5, -1, 4,
                                             -1, 15, -1, 14, -1, 13, -1, 12, -1, 7, -1, 6, -1, 5, -1, 4);
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_aligned; dx += 4) {
                int idx = dx << 2;
                __m256i v_s0_s8 = _mm256_loadu_si256((__m256i const*)S00);
                __m256i v_s1_s8 = _mm256_loadu_si256((__m256i const*)S01);

                __m256i v_s00_s16 = _mm256_shuffle_epi8(v_s0_s8, v_mask_even);
                __m256i v_s01_s16 = _mm256_shuffle_epi8(v_s0_s8, v_mask_odd);
                __m256i v_s10_s16 = _mm256_shuffle_epi8(v_s1_s8, v_mask_even);
                __m256i v_s11_s16 = _mm256_shuffle_epi8(v_s1_s8, v_mask_odd);

                __m256i v_sum0_s16 = _mm256_add_epi16(_mm256_add_epi16(v_s00_s16, v_s01_s16), v_half);
                __m256i v_sum_s16 = _mm256_add_epi16(_mm256_add_epi16(v_s10_s16, v_s11_s16), v_sum0_s16);
                __m256i v_res_s16 = _mm256_srli_epi16(v_sum_s16, 2);
                __m256i v_res_s8 = _mm256_permute4x64_epi64(_mm256_shuffle_epi8(v_res_s16, v_mask), 0b00001000);
                _mm_storeu_si128((__m128i*)(dst0 + idx), _mm256_castsi256_si128(v_res_s8));
                S00 += 32;
                S01 += 32;
            }
            for (; dx < _dst_w; dx++) {
                int idx = dx << 2;
                dst0[idx + 0] = uint8_t((S00[0] + S00[4] + S01[0] + S01[4] + 2) >> 2);
                dst0[idx + 1] = uint8_t((S00[1] + S00[5] + S01[1] + S01[5] + 2) >> 2);
                dst0[idx + 2] = uint8_t((S00[2] + S00[6] + S01[2] + S01[6] + 2) >> 2);
                dst0[idx + 3] = uint8_t((S00[3] + S00[7] + S01[3] + S01[7] + 2) >> 2);
                S00 += 8;
                S01 += 8;
            }
            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c4_dn2x_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();
    HalfResizeBilinearC4AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c4_avx(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;
    const double diff = 1e-6;
    if (fabs(scale_x - scale_y) < diff) {
        if (fabs(scale_x - 2.f) < diff) {
            return resize_bilinear_c4_dn2x_avx(src, dst);
        }
    }
    return resize_bilinear_c4_comm_avx(src, dst);
}

int resize_bilinear_avx(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_bilinear_c1_avx(src, dst);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_bilinear_c3_avx(src, dst);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        resize_bilinear_c4_avx(src, dst);
        break;
    // case FCVImageType::NV21:
    // case FCVImageType::NV12:
        // resize_bilinear_yuv_comm_neon(src, dst);
        // break;
    default:
        resize_bilinear_common(src, dst);
        // LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

void vertical_resize_cubic_u16_avx(
        const int *row0,
        const int *row1,
        const int *row2,
        const int *row3,
        int src_width_align8,
        int src_w,
        int b0,
        int b1,
        int b2,
        int b3,
        uint8_t* dst) {
    __m256i vec_0  = _mm256_set1_epi32(0);
    __m256i vec_255  = _mm256_set1_epi32(255);
    __m256i vec_half  = _mm256_set1_epi32(int(1 << 21));

    __m256i v_b0 = _mm256_set1_epi32(b0);
    __m256i v_b1 = _mm256_set1_epi32(b1);
    __m256i v_b2 = _mm256_set1_epi32(b2);
    __m256i v_b3 = _mm256_set1_epi32(b3);

    __m256i v_mask = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0, 
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0);

    int dx = 0;
    for (; dx < src_width_align8; dx += 8) {
        __m256i v_s0_s32 = _mm256_loadu_si256((__m256i const*)row0);
        __m256i v_s1_s32 = _mm256_loadu_si256((__m256i const*)row1);
        __m256i v_s2_s32 = _mm256_loadu_si256((__m256i const*)row2);
        __m256i v_s3_s32 = _mm256_loadu_si256((__m256i const*)row3);

        __m256i v_d0_s32 = _mm256_mullo_epi32(v_s0_s32, v_b0);
        __m256i v_d1_s32 = _mm256_mullo_epi32(v_s1_s32, v_b1);
        __m256i v_d2_s32 = _mm256_mullo_epi32(v_s2_s32, v_b2);
        __m256i v_d3_s32 = _mm256_mullo_epi32(v_s3_s32, v_b3);

        __m256i v_dst_s32 = _mm256_add_epi32(
                _mm256_add_epi32(v_d0_s32, v_d1_s32), _mm256_add_epi32(v_d2_s32, v_d3_s32));
        v_dst_s32 = _mm256_srli_epi32(_mm256_add_epi32(_mm256_max_epi32(v_dst_s32, vec_0), vec_half), 22);
        v_dst_s32 = _mm256_min_epi32(_mm256_max_epi32(v_dst_s32, vec_0), vec_255);
        __m256i v_dst_u8 = _mm256_shuffle_epi8(v_dst_s32, v_mask);

        _mm_storeu_si32(dst, _mm256_castsi256_si128(v_dst_u8));
        _mm_storeu_si32(dst + 4, _mm256_extractf128_si256(v_dst_u8, 1));

        dst += 8;
        row0 += 8;
        row1 += 8;
        row2 += 8;
        row3 += 8;
    }

    for (; dx < src_w; dx++) {
        int res = (*(row0++) * b0 + *(row1++) * b1 + *(row2++) * b2 + *(row3++) * b3);
        *(dst++) = fcv_cast_u8((FCV_MAX(res, 0) + (1 << 21)) >> 22);
    }
}

void vertical_resize_cubic_f32_avx(
        const int *row0,
        const int *row1,
        const int *row2,
        const int *row3,
        int src_width_align8,
        int src_w,
        int b0,
        int b1,
        int b2,
        int b3,
        uint8_t* dst) {
    __m256i vec_0 = _mm256_set1_epi32(0);
    __m256i vec_255 = _mm256_set1_epi32(255);

    constexpr const float SCALE = float(1 << 22);

    __m256 v_b0 = _mm256_set1_ps(float(b0) / SCALE);
    __m256 v_b1 = _mm256_set1_ps(float(b1) / SCALE);
    __m256 v_b2 = _mm256_set1_ps(float(b2) / SCALE);
    __m256 v_b3 = _mm256_set1_ps(float(b3) / SCALE);

    __m256i v_mask = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0, 
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0);

    int dx = 0;
    for (; dx < src_width_align8; dx += 8) {
        __m256 v_s0_f32 = _mm256_cvtepi32_ps(_mm256_loadu_si256((__m256i const*)row0));
        __m256 v_s1_f32 = _mm256_cvtepi32_ps(_mm256_loadu_si256((__m256i const*)row1));
        __m256 v_s2_f32 = _mm256_cvtepi32_ps(_mm256_loadu_si256((__m256i const*)row2));
        __m256 v_s3_f32 = _mm256_cvtepi32_ps(_mm256_loadu_si256((__m256i const*)row3));

        __m256 v_d_f32 = _mm256_mul_ps(v_s0_f32, v_b0);
        v_d_f32 = _mm256_fmadd_ps(v_s1_f32, v_b1, v_d_f32);
        v_d_f32 = _mm256_fmadd_ps(v_s2_f32, v_b2, v_d_f32);
        v_d_f32 = _mm256_fmadd_ps(v_s3_f32, v_b3, v_d_f32);

        __m256i v_dst_s32 = _mm256_min_epi32(_mm256_max_epi32(_mm256_cvtps_epi32(v_d_f32), vec_0), vec_255);
        __m256i v_dst_u8 = _mm256_shuffle_epi8(v_dst_s32, v_mask);

        _mm_storeu_si32(dst, _mm256_castsi256_si128(v_dst_u8));
        _mm_storeu_si32(dst + 4, _mm256_extractf128_si256(v_dst_u8, 1));

        dst += 8;
        row0 += 8;
        row1 += 8;
        row2 += 8;
        row3 += 8;
    }

    for (; dx < src_w; dx++) {
        int res = (*(row0++) * b0 + *(row1++) * b1 + *(row2++) * b2 + *(row3++) * b3);
        *(dst++) = fcv_cast_u8((FCV_MAX(res, 0) + (1 << 21)) >> 22);
    }
}

class ResizeCubicC1AVXParallelTask : public ParallelTask {
public:
    ResizeCubicC1AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const int16_t* alpha = (const int16_t*)(yofs + _dst_h);
        const int16_t* beta  = (const int16_t*)(alpha + (_dst_w << 2));

        int* rows = (int*)malloc(_dst_stride * 4 * sizeof(int));
        int* rows0 = rows;
        int* rows1 = rows0 + _dst_stride;
        int* rows2 = rows1 + _dst_stride;
        int* rows3 = rows2 + _dst_stride;

        int prev_sy = -5;
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const int16_t *alphap = alpha;

            if (sy0 == prev_sy && valid_rows1) {
                // hresize one row
                int* rows0_tmp = rows0;
                rows0 = rows1;
                rows1 = rows2;
                rows2 = rows3;
                rows3 = rows0_tmp;

                const uint8_t* src3 = _src_ptr + sy_off + (_src_stride * 3);
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int sx = xofs[dx];

                    int16_t a0 = *alphap++;
                    int16_t a1 = *alphap++;
                    int16_t a2 = *alphap++;
                    int16_t a3 = *alphap++;

                    *rows3p++ = (src3[sx] * a0 + src3[sx + 1] * a1 +
                            src3[sx + 2] * a2 + src3[sx + 3] * a3);
                }
            } else if (sy0 == prev_sy + 1 && valid_rows1) {
                // hresize two row
                int* rows0_tmp = rows0;
                int* rows1_tmp = rows1;
                rows0 = rows2;
                rows1 = rows3;
                rows2 = rows0_tmp;
                rows3 = rows1_tmp;

                const uint8_t* src2 = _src_ptr + sy_off + (_src_stride << 1);
                const uint8_t* src3 = src2 + _src_stride;

                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int sx = xofs[dx];
                    int16_t a0 = *alphap++;
                    int16_t a1 = *alphap++;
                    int16_t a2 = *alphap++;
                    int16_t a3 = *alphap++;

                    *rows2p++ = (src2[sx] * a0 + src2[sx + 1] * a1 +
                            src2[sx + 2] * a2 + src2[sx + 3] * a3);
                    *rows3p++ = (src3[sx] * a0 + src3[sx + 1] * a1 +
                            src3[sx + 2] * a2 + src3[sx + 3] * a3);
                }

            } else if (sy0 == prev_sy + 2 && valid_rows1) {
                // hresize three row
                int* rows0_tmp = rows0;
                rows0 = rows3;
                rows3 = rows2;
                rows2 = rows1;
                rows1 = rows0_tmp;

                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                const uint8_t* src2 = src1 + _src_stride;
                const uint8_t* src3 = src2 + _src_stride;

                int* rows1p = rows1;
                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int sx = xofs[dx];

                    int16_t a0 = *alphap++;
                    int16_t a1 = *alphap++;
                    int16_t a2 = *alphap++;
                    int16_t a3 = *alphap++;

                    *rows1p++ = (src1[sx] * a0 + src1[sx + 1] * a1 +
                            src1[sx + 2] * a2 + src1[sx + 3] * a3);
                    *rows2p++ = (src2[sx] * a0 + src2[sx + 1] * a1 +
                            src2[sx + 2] * a2 + src2[sx + 3] * a3);
                    *rows3p++ = (src3[sx] * a0 + src3[sx + 1] * a1 +
                            src3[sx + 2] * a2 + src3[sx + 3] * a3);
                }
            } else if (sy0 > prev_sy + 2) {
                // hresize four rows
                const uint8_t* src0 = _src_ptr + sy_off;
                const uint8_t* src1 = src0 + _src_stride;
                const uint8_t* src2 = src1 + _src_stride;;
                const uint8_t* src3 = src2 + _src_stride;;

                int* rows0p = rows0;
                int* rows1p = rows1;
                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int sx = xofs[dx];
                    int16_t a0 = *alphap++;
                    int16_t a1 = *alphap++;
                    int16_t a2 = *alphap++;
                    int16_t a3 = *alphap++;

                    *rows0p++ = (src0[sx] * a0 + src0[sx + 1] * a1 +
                            src0[sx + 2] * a2 + src0[sx + 3] * a3);
                    *rows1p++ = (src1[sx] * a0 + src1[sx + 1] * a1 +
                            src1[sx + 2] * a2 + src1[sx + 3] * a3);
                    *rows2p++ = (src2[sx] * a0 + src2[sx + 1] * a1 +
                            src2[sx + 2] * a2 + src2[sx + 3] * a3);
                    *rows3p++ = (src3[sx] * a0 + src3[sx + 1] * a1 +
                            src3[sx + 2] * a2 + src3[sx + 3] * a3);
                }
            }
            valid_rows1 = true;
            prev_sy = sy0 + 1;
            int16_t b0 = beta[i * 4];
            int16_t b1 = beta[i * 4 + 1];
            int16_t b2 = beta[i * 4 + 2];
            int16_t b3 = beta[i * 4 + 3];

            vertical_resize_cubic_f32_avx(rows0, rows1, rows2, rows3,
                    dst_width_align8, _dst_w, b0, b1, b2, b3, ptr_dst);

            ptr_dst += _dst_stride;
        }

        free(rows);
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

void resize_cubic_c1_avx(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int buf_size = (dst_h + dst_w) * sizeof(int) + (dst_h + dst_w) * 4 * sizeof(short);
    int* buf = (int*)malloc(buf_size);
    get_resize_cubic_buf(src_w, src_h, dst_w, dst_h, 1, &buf);

    ResizeCubicC1AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

class ResizeCubicC3AVXParallelTask : public ParallelTask {
public:
    ResizeCubicC3AVXParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const int16_t* alpha = (const int16_t*)(yofs + _dst_h);
        const int16_t* beta  = (const int16_t*)(alpha + (_dst_w << 2));

        int* rows = (int*)malloc(_dst_stride * 4 * sizeof(int));
        int* rows0 = rows;
        int* rows1 = rows0 + _dst_stride;
        int* rows2 = rows1 + _dst_stride;
        int* rows3 = rows2 + _dst_stride;

        int prev_sy = -5;
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const int16_t *alphap = alpha;

            if (sy0 == prev_sy && valid_rows1) {
                // hresize one row
                int* rows0_tmp = rows0;
                rows0 = rows1;
                rows1 = rows2;
                rows2 = rows3;
                rows3 = rows0_tmp;

                const uint8_t* src3 = _src_ptr + sy_off + (_src_stride * 3);
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    int16_t a0 = alphap[0];
                    int16_t a1 = alphap[1];
                    int16_t a2 = alphap[2];
                    int16_t a3 = alphap[3];
                    const int cx = xofs[dx];
                    const uint8_t* s3p = src3 + cx;

                    rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                    rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                    rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                    alphap += 4;
                    rows3p += 3;
                }
            } else if (sy0 == prev_sy + 1 && valid_rows1) {
                // hresize two row
                int* rows0_tmp = rows0;
                int* rows1_tmp = rows1;
                rows0 = rows2;
                rows1 = rows3;
                rows2 = rows0_tmp;
                rows3 = rows1_tmp;

                const uint8_t* src2 = _src_ptr + sy_off + (_src_stride << 1);
                const uint8_t* src3 = src2 + _src_stride;

                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int cx = xofs[dx];

                    const uint8_t* s2p = src2 + cx;
                    const uint8_t* s3p = src3 + cx;

                    int16_t a0 = alphap[0];
                    int16_t a1 = alphap[1];
                    int16_t a2 = alphap[2];
                    int16_t a3 = alphap[3];

                    rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                    rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                    rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                    rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                    rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                    rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                    alphap += 4;
                    rows2p += 3;
                    rows3p += 3;
                }
            } else if (sy0 == prev_sy + 2 && valid_rows1) {
                // hresize three row
                int* rows0_tmp = rows0;
                rows0 = rows3;
                rows3 = rows2;
                rows2 = rows1;
                rows1 = rows0_tmp;

                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                const uint8_t* src2 = src1 + _src_stride;
                const uint8_t* src3 = src2 + _src_stride;

                int* rows1p = rows1;
                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int cx = xofs[dx];

                    int16_t a0 = alphap[0];
                    int16_t a1 = alphap[1];
                    int16_t a2 = alphap[2];
                    int16_t a3 = alphap[3];

                    const uint8_t* s1p = src1 + cx;
                    const uint8_t* s2p = src2 + cx;
                    const uint8_t* s3p = src3 + cx;

                    rows1p[0] = s1p[0] * a0 + s1p[3] * a1 + s1p[6] * a2 + s1p[9 ] * a3;
                    rows1p[1] = s1p[1] * a0 + s1p[4] * a1 + s1p[7] * a2 + s1p[10] * a3;
                    rows1p[2] = s1p[2] * a0 + s1p[5] * a1 + s1p[8] * a2 + s1p[11] * a3;

                    rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                    rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                    rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                    rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                    rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                    rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                    alphap += 4;

                    rows1p += 3;
                    rows2p += 3;
                    rows3p += 3;
                }
            } else if (sy0 > prev_sy + 2) {
                // hresize four rows
                const uint8_t* src0 = _src_ptr + sy_off;
                const uint8_t* src1 = src0 + _src_stride;
                const uint8_t* src2 = src1 + _src_stride;;
                const uint8_t* src3 = src2 + _src_stride;;

                int* rows0p = rows0;
                int* rows1p = rows1;
                int* rows2p = rows2;
                int* rows3p = rows3;

                for (int dx = 0; dx < _dst_w; dx++) {
                    const int cx = xofs[dx];

                    int16_t a0 = alphap[0];
                    int16_t a1 = alphap[1];
                    int16_t a2 = alphap[2];
                    int16_t a3 = alphap[3];

                    const uint8_t* s0p = src0 + cx;
                    const uint8_t* s1p = src1 + cx;
                    const uint8_t* s2p = src2 + cx;
                    const uint8_t* s3p = src3 + cx;

                    rows0p[0] = s0p[0] * a0 + s0p[3] * a1 + s0p[6] * a2 + s0p[9 ] * a3;
                    rows0p[1] = s0p[1] * a0 + s0p[4] * a1 + s0p[7] * a2 + s0p[10] * a3;
                    rows0p[2] = s0p[2] * a0 + s0p[5] * a1 + s0p[8] * a2 + s0p[11] * a3;

                    rows1p[0] = s1p[0] * a0 + s1p[3] * a1 + s1p[6] * a2 + s1p[9 ] * a3;
                    rows1p[1] = s1p[1] * a0 + s1p[4] * a1 + s1p[7] * a2 + s1p[10] * a3;
                    rows1p[2] = s1p[2] * a0 + s1p[5] * a1 + s1p[8] * a2 + s1p[11] * a3;

                    rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                    rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                    rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                    rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                    rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                    rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                    alphap += 4;

                    rows0p += 3;
                    rows1p += 3;
                    rows2p += 3;
                    rows3p += 3;
                }
            }
            valid_rows1 = true;
            prev_sy = sy0 + 1;
            int dy4 = i << 2;
            int b0 = int(beta[dy4]);
            int b1 = int(beta[dy4 + 1]);
            int b2 = int(beta[dy4 + 2]);
            int b3 = int(beta[dy4 + 3]);

            vertical_resize_cubic_f32_avx(rows0, rows1, rows2, rows3,
                    dst_width_align8, _dst_stride, b0, b1, b2, b3, ptr_dst);

            ptr_dst += _dst_stride;
        }

        free(rows);
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

void resize_cubic_c3_avx(Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int buf_size = (dst_h + dst_w) * sizeof(int) + (dst_h + dst_w) * 4 * sizeof(short);
    int* buf = (int*)malloc(buf_size);
    get_resize_cubic_buf(src_w, src_h, dst_w, dst_h, 3, &buf);

    ResizeCubicC3AVXParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

int resize_cubic_avx(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_cubic_c1_avx(src, dst);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_cubic_c3_avx(src, dst);
        break;
    // case FCVImageType::PKG_RGBA_U8:
    // case FCVImageType::PKG_BGRA_U8:
        //resize_bilinear_c4_neon(src, dst);
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}
G_FCV_NAMESPACE1_END()
