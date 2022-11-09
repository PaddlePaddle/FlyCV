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

#include "modules/fusion_api/bgra_to_resize_to_bgr/include/bgra_to_resize_to_bgr_arm.h"

#include <math.h>
#include <stdlib.h>
#include <arm_neon.h>

#include "modules/core/base/include/macro_utils.h"
#include "modules/core/basic_math/interface/basic_math.h"
#include "modules/img_transform/resize/include/resize_arm.h"
#include "modules/img_transform/resize/include/resize_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

unsigned char FCV_ALIGNED(8) bgra_to_bgr_tab[8] = {0, 1, 2, 4, 5, 6, 0, 0};

static void vertical_bgra_resize_bilinear_to_bgr(
        const unsigned short *row0,
        const unsigned short *row1,
        int dst_width_align2,
        int dst_width,
        unsigned short b0,
        unsigned short b1,
        unsigned char *dst) {
    uint8x8_t bgra2bgr_tab = vld1_u8(bgra_to_bgr_tab);

    uint16x4_t v_b0 = vdup_n_u16(b0);
    uint16x4_t v_b1 = vdup_n_u16(b1);
    int dx = 0;
    for (; dx < dst_width_align2; dx += 2) {
        uint16x8_t v_s00_u16 = vld1q_u16(row0);
        uint16x8_t v_s01_u16 = vld1q_u16(row1);

        uint32x4_t rows_u32_lo = vmull_u16(vget_low_u16(v_s00_u16), v_b0);
        uint32x4_t rows_u32_hi = vmull_u16(vget_high_u16(v_s00_u16), v_b0);

        rows_u32_lo = vmlal_u16(rows_u32_lo, vget_low_u16(v_s01_u16), v_b1);
        rows_u32_hi = vmlal_u16(rows_u32_hi, vget_high_u16(v_s01_u16), v_b1);

        uint16x4_t rows_u16_lo = vrshrn_n_u32(rows_u32_lo, 16);
        uint16x4_t rows_u16_hi = vrshrn_n_u32(rows_u32_hi, 16);

        uint8x8_t res_u8 = vrshrn_n_u16(vcombine_u16(rows_u16_lo, rows_u16_hi), 2);

        res_u8 = vtbl1_u8(res_u8, bgra2bgr_tab);
        vst1_u8(dst, res_u8);

        dst += 6;
        row0 += 8;
        row1 += 8;
    }

    for (; dx < dst_width; dx++) {
        dst[0] = (row0[0] * b0 + row1[0] * b1 + (1 << 17)) >> 18;
        dst[1] = (row0[1] * b0 + row1[1] * b1 + (1 << 17)) >> 18;
        dst[2] = (row0[2] * b0 + row1[2] * b1 + (1 << 17)) >> 18;

        dst += 3;
        row0 += 4;
        row1 += 4;
    }
}

void bgra_to_resize_bilinear_to_bgr_generic_neon(
        const unsigned char* src,
        unsigned char* dst,
        const int dst_w,
        const int dst_h,
        const int src_w,
        const int src_h,
        const int src_stride,
        const int dst_stride) {
    const unsigned char *src_ptr = src;
    unsigned char *dst_ptr = dst;
    const int dst_width_align2 = dst_w & (~1);
    const int dst_bgra_stride = (dst_stride / 3) << 2;

    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = dst_bgra_stride << 2; // dst_bgra_stride * 2 * sizeof(unsigned short);
    int* buf = (int*)malloc(buf_size);
    unsigned short *rows = (unsigned short*)malloc(rows_size);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + dst_bgra_stride;

    unsigned char *ptr_dst = nullptr;
    int sy0 = 0;
    unsigned short b0 = 0;
    unsigned short b1 = 0;

    int prev_sy1 = -1;
    for (int dy = 0; dy < dst_h; dy++) {
        ptr_dst = dst_ptr + dst_stride * dy;
        sy0 = *(yofs + dy);
        const int sy_off = sy0 * src_stride;

        if (sy0 == prev_sy1) {
            unsigned short *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + src_stride);
            unsigned short *alphap = alpha;

            for (int dx = 0; dx < dst_w; dx++) {
                int idx = dx << 2;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* s1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s1 = vld1_u8(s1p);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows1_res = vrshrn_n_u32(v_rows1, 4);
                vst1_u16(rows1 + idx, v_rows1_res);
            }
        } else if (sy0 > prev_sy1) {
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + src_stride);

            unsigned short *rows0p = rows0;
            unsigned short *rows1p = rows1;

            unsigned short *alphap = alpha;
            for (int dx = 0; dx < dst_w; dx++) {
                int idx = dx << 2;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* s0p = src0 + cx;
                const unsigned char* s1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s0 = vld1_u8(s0p);
                uint8x8_t v_s1 = vld1_u8(s1p);
                uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s0_lo = vget_low_u16(v_s0_u16);
                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);

                uint16x4_t v_s0_hi = vget_high_u16(v_s0_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows0 = vmull_u16(v_s0_lo, v_a0);
                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);

                v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows0_res = vshrn_n_u32(v_rows0, 4);
                uint16x4_t v_rows1_res = vshrn_n_u32(v_rows1, 4);
                vst1_u16(rows0p + idx, v_rows0_res);
                vst1_u16(rows1p + idx, v_rows1_res);
            }
        }

        prev_sy1 = sy0 + 1;
        b0 = *beta++;
        b1 = *beta++;

        vertical_bgra_resize_bilinear_to_bgr(rows0, rows1,
                dst_width_align2, dst_w, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void bgra_to_resize_bilinear_dn2x_to_bgr_neon(
        const unsigned char* src,
        unsigned char* dst,
        int dst_w,
        int dst_h,
        int src_stride,
        int dst_stride) {
    const unsigned char *ptr_src = src;
    unsigned char *ptr_dst = dst;
    int dou_src_step = src_stride << 1;

    int count = dst_w & (~7);
    int remain = dst_w - count;
    int src_paralle_size = count << 3;
    int dst_paralle_size = count * 3;

    for (int dy = 0; dy < dst_h; dy++) {
        const unsigned char *s0 = (unsigned char*)ptr_src;
        const unsigned char *s1 = s0 + src_stride;
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        int nn = count;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                       \n"
                "prfm  pldl1keep, [%0, #128]                              \n"
                "ld4         {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64  \n"  // load 16 pixels of ARGB
                "subs        %w3, %w3, #8                                 \n"  // 16 processed per loop
                "uaddlp      v0.8h, v0.16b                                \n"  // B  Long pairwise add
                "uaddlp      v1.8h, v1.16b                                \n"  // G
                "uaddlp      v2.8h, v2.16b                                \n"  // R
                "ld4         {v4.16b, v5.16b, v6.16b, v7.16b}, [%1], #64  \n"
                "uadalp      v0.8h, v4.16b                                \n"  // B  Long pairwise add
                "uadalp      v1.8h, v5.16b                                \n"  // G
                "uadalp      v2.8h, v6.16b                                \n"  // R
                "prfm        pldl1keep, [%1, #128]                         \n"
                "rshrn       v0.8b, v0.8h, #2                             \n"  // rounding narrowing shift right by 2
                "rshrn       v1.8b, v1.8h, #2                             \n"
                "rshrn       v2.8b, v2.8h, #2                             \n"
                "st3 {v0.8b, v1.8b, v2.8b}, [%2], #24                     \n"
                "b.gt        0b                                           \n"
                : "+r"(s0),              // %0
                "+r"(s1),                // %1
                "+r"(dst0),              // %2
                "+r"(nn)                 // %3
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7");
#else
            asm volatile(
                "0:                                                    \n"
                "pld         [%0, #128]                                \n"
                "vld4.8      {d0, d2, d4, d6}, [%0]!                   \n"  // load 8 ARGB pixels.
                "vld4.8      {d1, d3, d5, d7}, [%0]!                   \n"
                "subs        %3, %3, #8                                \n"
                "vpaddl.u8   q0, q0                                    \n"  // B
                "vpaddl.u8   q1, q1                                    \n"  // G
                "vpaddl.u8   q2, q2                                    \n"  // R
                "vld4.8      {d8, d10, d12, d14}, [%1]!                \n"  // load 8 more ARGB
                "vld4.8      {d9, d11, d13, d15}, [%1]!                \n"
                "pld         [%1, #128]                                \n"
                "vpadal.u8   q0, q4                                    \n"
                "vpadal.u8   q1, q5                                    \n"
                "vpadal.u8   q2, q6                                    \n"
                "vrshrn.u16  d0, q0, #2                                \n"
                "vrshrn.u16  d1, q1, #2                                \n"
                "vrshrn.u16  d2, q2, #2                                \n"
                "vst3.8      {d0, d1, d2},   [%2]!                     \n"  // store 24 pixels of BGR.
                "bgt        0b                                         \n"
                : "+r"(s0),              // %0
                "+r"(s1),                // %1
                "+r"(dst0),              // %2
                "+r"(nn)                 // %3
                :
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7");
#endif
    }

    if (remain) {
        const unsigned char* s0_remain = ptr_src + src_paralle_size;
        const unsigned char* s1_remain = s0_remain + src_stride;
        unsigned char* dst_remain = ptr_dst + dst_paralle_size;
        for (int j = 0; j < remain; j++) {
            dst_remain[0] = (unsigned char)((s0_remain[0] + s0_remain[4]
                    + s1_remain[0] + s1_remain[4] + 2) >> 2);
            dst_remain[1] = (unsigned char)((s0_remain[1] + s0_remain[5]
                    + s1_remain[1] + s1_remain[5] + 2) >> 2);
            dst_remain[2] = (unsigned char)((s0_remain[2] + s0_remain[6]
                    + s1_remain[2] + s1_remain[6] + 2) >> 2);

            s0_remain += 4;
            s1_remain += 4;
            dst_remain += 3;
        }
    }

        ptr_src += dou_src_step;
        ptr_dst += dst_stride;
    }
}

void bgra_to_resize_bilinear_dn4x_to_bgr_neon(
        const unsigned char* src,
        unsigned char* dst,
        const int dst_w,
        const int dst_h,
        const int src_stride,
        const int dst_stride) {
    const int dou_src_step = src_stride << 1;
    const int fou_src_step = dou_src_step << 1;

    const unsigned char *ptr_src = src;
    unsigned char *ptr_dst = dst;

    uint8x8_t src0_u8, src1_u8;
    for (int dy = 0; dy < dst_h; dy++) {
        const unsigned char* s01 = (unsigned char*)(ptr_src + src_stride);
        const unsigned char* s02 = (unsigned char*)(ptr_src + dou_src_step);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (int dx = 0; dx < dst_w; dx++) {
            src0_u8 = vld1_u8(s01 + 4);
            src1_u8 = vld1_u8(s02 + 4);

            uint16x8_t hsum0_u16  = vaddl_u8(src0_u8, src1_u8);
            uint16x8_t hsum1_u16 = vextq_u16(hsum0_u16, hsum0_u16, 4);//b2 g2 r2
            uint16x8_t vsum_u16  = vaddq_u16(hsum0_u16, hsum1_u16);

            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_u8(dst0, res_u8);

            s01 += 16;
            s02 += 16;
            dst0 += 3;
        }

        ptr_src += fou_src_step;
        ptr_dst += dst_stride;
    }
}

int bgra_to_resize_bilinear_to_bgr_neon(Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int dst_w = dst.width();
    int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;

    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    int src_stride = src.stride();
    int dst_stride = dst.stride();

    if (is_almost_equal(scale_x, scale_y)) {
        if (is_almost_equal(scale_x, 2.)) {
            bgra_to_resize_bilinear_dn2x_to_bgr_neon(src_ptr,
                    dst_ptr, dst_w, dst_h, src_stride, dst_stride);
        } else if (is_almost_equal(scale_x, 4.)) {
            bgra_to_resize_bilinear_dn4x_to_bgr_neon(src_ptr,
                    dst_ptr, dst_w, dst_h, src_stride, dst_stride);
        } else {
            bgra_to_resize_bilinear_to_bgr_generic_neon(src_ptr,
                    dst_ptr,dst_w, dst_h, src_w, src_h, src_stride, dst_stride);
        }
    } else {
        bgra_to_resize_bilinear_to_bgr_generic_neon(src_ptr,
                dst_ptr, dst_w, dst_h, src_w, src_h, src_stride, dst_stride);
    }

    return 0;
}

void bgra_to_resize_nearest_dn2x_to_bgr_neon(Mat& src, Mat& dst) {
    int dst_w = dst.width();
    int dst_h = dst.height();

    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int dou_src_step = src_stride << 1;

    const unsigned char *ptr_src = (const unsigned char*)src.data() + src_stride;
    unsigned char *ptr_dst = (unsigned char*)dst.data();

    int count = dst_w & (~7);
    int remain = dst_w - count;
    int src_paralle_size = count << 3;
    int dst_paralle_size = count * 3;

    for (int dy = 0; dy < dst_h; dy++) {
        const unsigned char *s0 = (unsigned char*)ptr_src;
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        int nn = count;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                       \n"
                "prfm  pldl1keep, [%0, #128]                              \n"
                "ld4         {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64  \n"  // load 16 pixels of ARGB
                "subs        %w2, %w2, #8                                 \n"  // 8 processed per loop
                "uzp2        v4.16b, v0.16b, v0.16b                       \n"  // 1 3 5 7 9 11 13 15
                "uzp2        v5.16b, v1.16b, v1.16b                       \n"
                "uzp2        v6.16b, v2.16b, v2.16b                       \n"
                "st3 {v4.8b, v5.8b, v6.8b}, [%1], #24                     \n"
                "b.gt        0b                                           \n"
                : "+r"(s0),              // %0
                "+r"(dst0),              // %1
                "+r"(nn)                 // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6");
#else
            asm volatile(
                "0:                                                    \n"
                "pld         [%0, #128]                                \n"
                "vld4.8      {d0, d2, d4, d6}, [%0]!                   \n"  // load 8 ARGB pixels.
                "vld4.8      {d1, d3, d5, d7}, [%0]!                   \n"
                "subs         %2, %2, #8                                \n"
                "vuzp.u8      d0, d1                                   \n"
                "vuzp.u8      d2, d3                                   \n"
                "vuzp.u8      d4, d5                                   \n"
                "vst3.8      {d1, d3, d5},   [%1]!                     \n"  // store 24 pixels of BGR.
                "bgt        0b                                         \n"
                : "+r"(s0),              // %0
                "+r"(dst0),              // %1
                "+r"(nn)                 // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
        }

        if (remain) {
            const unsigned char *s0_remain = ptr_src + src_paralle_size;

            unsigned char *dst_remain = ptr_dst + dst_paralle_size;
            for (int j = 0; j < remain; j++) {
                dst_remain[0] = (unsigned char)(s0_remain[4]);
                dst_remain[1] = (unsigned char)(s0_remain[5]);
                dst_remain[2] = (unsigned char)(s0_remain[6]);

                s0_remain += 8;
                dst_remain += 3;
            }
        }

        ptr_src += dou_src_step;
        ptr_dst += dst_stride;
    }
}

static unsigned char FCV_ALIGNED(8) bgra_to_bgr_dn4_tab[8] = {8, 9, 10, 24, 25, 26, 0, 0};

void bgra_to_resize_nearest_dn4x_to_bgr_neon(Mat& src, Mat& dst) {
    int dst_w = dst.width();
    int dst_h = dst.height();

    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int doub_src_step = src_stride << 1;
    const int four_src_step = doub_src_step << 1;

    const unsigned char *ptr_src = (const unsigned char *)src.data() + doub_src_step;
    unsigned char *ptr_dst = (unsigned char *)dst.data();

    int count = dst_w & (~1);
    int remain = dst_w - count;
    int src_paralle_size = count << 4;
    int dst_paralle_size = count * 3;

    for (int dy = 0; dy < dst_h; dy++) {
        const unsigned char *s0 = (unsigned char*)ptr_src;
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        int nn = count;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "ld1         {v4.8b}, [%3]                             \n"
                "0:                                                    \n"
                "prfm  pldl1keep, [%0, #128]                           \n"
                "ld1         {v0.16b, v1.16b}, [%0], #32               \n"  // load 16 pixels of ARGB
                "subs        %w2, %w2, #2                              \n"  // 8 processed per loop
                "tbl         v3.8b, {v0.16b, v1.16b}, v4.8b            \n"
                "st1 {v3.8b}, [%1]                                     \n"
                "add         %1, %1, #6                                \n"
                "b.gt        0b                                        \n"
                : "+r"(s0),              // %0
                "+r"(dst0),              // %1
                "+r"(nn)                 // %2
                : "r"(&bgra_to_bgr_dn4_tab)
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4");
#else
            asm volatile(
                "vld1.8      {d4}, [%3]                                \n"
                "0:                                                    \n"
                "pld         [%0, #128]                                \n"
                "vld1.8      {q0, q1}, [%0]!                           \n"  // load 8 ARGB pixels.
                "subs         %2, %2, #2                               \n"
                "vtbl.8      d5, {d0, d1, d2, d3}, d4                  \n"
                "vst1.8      {d5},   [%1]                              \n"  // store 24 pixels of BGR.
                "add         %1, #6                                    \n"
                "bgt        0b                                         \n"
                : "+r"(s0),              // %0
                "+r"(dst0),              // %1
                "+r"(nn)                 // %2
                : "r"(&bgra_to_bgr_dn4_tab)
            : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
    }

        if (remain) {
            const unsigned char *s0_remain = ptr_src + src_paralle_size;

            unsigned char *dst_remain = ptr_dst + dst_paralle_size;
            for (int j = 0; j < remain; j++) {
                dst_remain[0] = (unsigned char)(s0_remain[8]);
                dst_remain[1] = (unsigned char)(s0_remain[9]);
                dst_remain[2] = (unsigned char)(s0_remain[10]);

                s0_remain += 16;
                dst_remain += 3;
            }
        }

        ptr_src += four_src_step;
        ptr_dst += dst_stride;
    }
}

int bgra_to_resize_nearest_to_bgr_generic_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    int buf_size = (dst_w + dst_h) << 2; //(dst_w + dst_h) * sizeof(int)
    int* buf = (int*)malloc(buf_size);

    int* xofs = buf;
    int* yofs = buf + dst_w;

    nearest_cal_offset(xofs, dst_w, src_w);
    nearest_cal_offset(yofs, dst_h, src_h);

    for (int dy = 0; dy < dst_h; dy++) {
        const unsigned char* src0 = src_ptr + (yofs[dy] >> 16) * src_stride;
        unsigned char * ptr_dst0 = dst_ptr + dst_stride * dy;

        for (int dx = 0; dx < dst_w; dx++) {
            int idx0 = (xofs[dx] >> 16) << 2; 

            ptr_dst0[dx * 3] = src0[idx0];
            ptr_dst0[dx * 3 + 1] = src0[idx0 + 1];
            ptr_dst0[dx * 3 + 2] = src0[idx0 + 2];
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    return 0;
}

int bgra_to_resize_nearest_to_bgr_neon(Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int dst_w = dst.width();
    int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;

    if (is_almost_equal(scale_x, scale_y)) {
        if (is_almost_equal(scale_x, 2.)) {
            bgra_to_resize_nearest_dn2x_to_bgr_neon(src, dst);
        } else if (is_almost_equal(scale_x, 4.)) {
            bgra_to_resize_nearest_dn4x_to_bgr_neon(src, dst);
        } else {
            bgra_to_resize_nearest_to_bgr_generic_neon(src, dst);
        }
    } else {
        bgra_to_resize_nearest_to_bgr_generic_neon(src, dst);
    }

     return 0;
}

G_FCV_NAMESPACE1_END()
