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

#include "modules/img_transform/rotation/include/rotation_arm.h"
#include "modules/core/base/include/type_info.h"

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static int transpose_u8c1_8x8_neon(
        const unsigned char *src_data,
        unsigned char *dst_data,
        int width,
        int src_stride,
        int dst_stride) {
    int nn = width;
    const unsigned char *src_tmp;

    if (nn) {
#if __aarch64__
        asm volatile(
            "0:                                        \n"
            "mov          %0, %1                       \n"
            "ld1         {v0.8b}, [%0], %4             \n"
            "ld1         {v1.8b}, [%0], %4             \n"
            "ld1         {v2.8b}, [%0], %4             \n"
            "ld1         {v3.8b}, [%0], %4             \n"
            "ld1         {v4.8b}, [%0], %4             \n"
            "ld1         {v5.8b}, [%0], %4             \n"
            "ld1         {v6.8b}, [%0], %4             \n"
            "ld1         {v7.8b}, [%0]                 \n"

            "trn1        v16.8b, v0.8b, v1.8b          \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn2        v17.8b, v0.8b, v1.8b          \n"
            "add         %0, %0, %4                    \n"
            "trn1        v18.8b, v2.8b, v3.8b          \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn2        v19.8b, v2.8b, v3.8b          \n"
            "add         %0, %0, %4                    \n"
            "trn1        v20.8b, v4.8b, v5.8b          \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn2        v21.8b, v4.8b, v5.8b          \n"
            "add         %0, %0, %4                    \n"
            "trn1        v22.8b, v6.8b, v7.8b          \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn2        v23.8b, v6.8b, v7.8b          \n"

            "trn1        v0.4h, v16.4h, v18.4h         \n"
            "add         %0, %0, %4                    \n"
            "trn2        v2.4h, v16.4h, v18.4h         \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn1        v1.4h, v17.4h, v19.4h         \n"
            "add         %0, %0, %4                    \n"
            "trn2        v3.4h, v17.4h, v19.4h         \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn1        v4.4h, v20.4h, v22.4h         \n"
            "add         %0, %0, %4                    \n"
            "trn2        v6.4h, v20.4h, v22.4h         \n"
            "prfm        pldl1keep, [%0, 448]          \n"
            "trn1        v5.4h, v21.4h, v23.4h         \n"
            "add         %0, %0, %4                    \n"
            "trn2        v7.4h, v21.4h, v23.4h         \n"
            "prfm        pldl1keep, [%0, 448]          \n"

            "trn1        v16.2s, v0.2s, v4.2s          \n"
            "trn2        v20.2s, v0.2s, v4.2s          \n"
            "trn1        v17.2s, v2.2s, v6.2s          \n"
            "trn2        v21.2s, v2.2s, v6.2s          \n"
            "trn1        v18.2s, v1.2s, v5.2s          \n"
            "trn2        v22.2s, v1.2s, v5.2s          \n"
            "trn1        v19.2s, v3.2s, v7.2s          \n"
            "trn2        v23.2s, v3.2s, v7.2s          \n"

            "mov          %0, %2                       \n"
            "st1         {v16.8b}, [%0], %5            \n"
            "st1         {v18.8b}, [%0], %5            \n"
            "st1         {v17.8b}, [%0], %5            \n"
            "st1         {v19.8b}, [%0], %5            \n"
            "st1         {v20.8b}, [%0], %5            \n"
            "st1         {v22.8b}, [%0], %5            \n"
            "st1         {v21.8b}, [%0], %5            \n"
            "st1         {v23.8b}, [%0]                \n"

            "add         %1, %1, #8                    \n"  // src += 8
            "add         %2, %2, %5, lsl #3            \n"  // dst += 8 * dst_stride
            "subs        %w3, %w3, #8                  \n"  // w   -= 8
            "bne         0b                            \n"

        : "+r"(src_tmp),                                 // %0
            "+r"(src_data),                              // %1
            "+r"(dst_data),                              // %2
            "+r"(nn)                                  // %3
        :"r"(static_cast<ptrdiff_t>(src_stride)),     // %4
            "r"(static_cast<ptrdiff_t>(dst_stride))   // %5
        : "memory", "cc", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v16",
            "v17", "v18", "v19", "v20", "v21", "v22", "v23");
#else
        asm volatile(
            "1:                                        \n"
            "mov          %0, %1                       \n"
            "vld1.8       {d0}, [%0], %4               \n"
            "vld1.8       {d1}, [%0], %4               \n"
            "vld1.8       {d2}, [%0], %4               \n"
            "vld1.8       {d3}, [%0], %4               \n"
            "vld1.8       {d4}, [%0], %4               \n"
            "vld1.8       {d5}, [%0], %4               \n"
            "vld1.8       {d6}, [%0], %4               \n"
            "vld1.8       {d7}, [%0]                   \n"

            "vtrn.8       d1, d0                       \n"
            "pld [%0, #448]                 \n"
            "add          %0, %0, %4                   \n"
            "vtrn.8       d3, d2                       \n"
            "pld [%0, #448]                 \n"
            "add          %0, %0, %4                   \n"
            "vtrn.8       d5, d4                       \n"
            "pld [%0, #448]                 \n"
            "add          %0, %0, %4                   \n"
            "vtrn.8       d7, d6                       \n"
            "pld [%0, #448]                 \n"

            "vtrn.16      d0, d2                       \n"
            "add          %0, %0, %4                   \n"
            "pld [%0, #448]                 \n"
            "vtrn.16      d1, d3                       \n"
            "add          %0, %0, %4                   \n"
            "pld [%0, #448]                 \n"
            "vtrn.16      d4, d6                       \n"
            "add          %0, %0, %4                   \n"
            "pld [%0, #448]                 \n"
            "vtrn.16      d5, d7                       \n"
            "add          %0, %0, %4                   \n"
            "pld [%0, #448]                 \n"

            "vtrn.32      d0, d4                       \n"
            "vtrn.32      d2, d6                       \n"
            "vtrn.32      d1, d5                       \n"
            "vtrn.32      d3, d7                       \n"

            "vrev16.8    q0, q0                        \n"
            "vrev16.8    q1, q1                        \n"
            "vrev16.8    q2, q2                        \n"
            "vrev16.8    q3, q3                        \n"

            "mov          %0, %2                       \n"

            "vst1.8       {d1}, [%0], %5               \n"
            "vst1.8       {d0}, [%0], %5               \n"
            "vst1.8       {d3}, [%0], %5               \n"
            "vst1.8       {d2}, [%0], %5               \n"
            "vst1.8       {d5}, [%0], %5               \n"
            "vst1.8       {d4}, [%0], %5               \n"
            "vst1.8       {d7}, [%0], %5               \n"
            "vst1.8       {d6}, [%0]                   \n"

            "add         %1, #8                        \n"  // src += 8
            "add         %2, %2, %5, lsl #3            \n"  // dst += 8 * dst_stride
            "subs        %3, #8                        \n"  // w   -= 8
            "bne        1b                            \n"

        : "+r"(src_tmp),                                 // %0
            "+r"(src_data),                              // %1
            "+r"(dst_data),                              // %2
            "+r"(nn)                               // %3
        :"r"(static_cast<ptrdiff_t>(src_stride)),    // %4
            "r"(static_cast<ptrdiff_t>(dst_stride))   // %5
        : "memory", "cc", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8");
#endif
    }

    return 0;
}

#define CHANNEL_TRANSPOSE(n)                                                                    \
    vtr01_u8 = vtrn_u8(v_r0_u8.val[n], v_r1_u8.val[n]);                                                     \
    vtr23_u8 = vtrn_u8(v_r2_u8.val[n], v_r3_u8.val[n]);                                                     \
    vtr45_u8 = vtrn_u8(v_r4_u8.val[n], v_r5_u8.val[n]);                                                     \
    vtr67_u8 = vtrn_u8(v_r6_u8.val[n], v_r7_u8.val[n]);                                                     \
    vtr02_u16 = vtrn_u16(vreinterpret_u16_u8(vtr01_u8.val[0]), vreinterpret_u16_u8(vtr23_u8.val[0]));       \
    vtr13_u16 = vtrn_u16(vreinterpret_u16_u8(vtr01_u8.val[1]), vreinterpret_u16_u8(vtr23_u8.val[1]));       \
    vtr46_u16 = vtrn_u16(vreinterpret_u16_u8(vtr45_u8.val[0]), vreinterpret_u16_u8(vtr67_u8.val[0]));       \
    vtr57_u16 = vtrn_u16(vreinterpret_u16_u8(vtr45_u8.val[1]), vreinterpret_u16_u8(vtr67_u8.val[1]));       \
    vtr04_u32 = vtrn_u32(vreinterpret_u32_u16(vtr02_u16.val[0]), vreinterpret_u32_u16(vtr46_u16.val[0]));   \
    vtr26_u32 = vtrn_u32(vreinterpret_u32_u16(vtr02_u16.val[1]), vreinterpret_u32_u16(vtr46_u16.val[1]));   \
    vtr15_u32 = vtrn_u32(vreinterpret_u32_u16(vtr13_u16.val[0]), vreinterpret_u32_u16(vtr57_u16.val[0]));   \
    vtr37_u32 = vtrn_u32(vreinterpret_u32_u16(vtr13_u16.val[1]), vreinterpret_u32_u16(vtr57_u16.val[1]));   \
    v_d0_u8.val[n] = vreinterpret_u8_u32(vtr04_u32.val[0]);                                                 \
    v_d1_u8.val[n] = vreinterpret_u8_u32(vtr15_u32.val[0]);                                                 \
    v_d2_u8.val[n] = vreinterpret_u8_u32(vtr26_u32.val[0]);                                                 \
    v_d3_u8.val[n] = vreinterpret_u8_u32(vtr37_u32.val[0]);                                                 \
    v_d4_u8.val[n] = vreinterpret_u8_u32(vtr04_u32.val[1]);                                                 \
    v_d5_u8.val[n] = vreinterpret_u8_u32(vtr15_u32.val[1]);                                                 \
    v_d6_u8.val[n] = vreinterpret_u8_u32(vtr26_u32.val[1]);                                                 \
    v_d7_u8.val[n] = vreinterpret_u8_u32(vtr37_u32.val[1]);                                                 \

#define CAL_TRANS_C0() CHANNEL_TRANSPOSE(0)
#define CAL_TRANS_C1() CHANNEL_TRANSPOSE(1)
#define CAL_TRANS_C2() CHANNEL_TRANSPOSE(2)
#define CAL_TRANS_C3() CHANNEL_TRANSPOSE(3)

static int transpose_u8c3_8x8_neon(
        const unsigned char *src_data,
        unsigned char *dst_data,
        int src_stride,
        int dst_stride) {
    const unsigned char *src = src_data;
    unsigned char *dst = dst_data;

    uint8x8x2_t vtr01_u8, vtr23_u8, vtr45_u8, vtr67_u8;
    uint16x4x2_t vtr02_u16, vtr13_u16, vtr46_u16, vtr57_u16;
    uint32x2x2_t vtr04_u32, vtr26_u32, vtr15_u32, vtr37_u32;
    uint8x8x3_t v_d0_u8, v_d1_u8, v_d2_u8, v_d3_u8, v_d4_u8, v_d5_u8, v_d6_u8, v_d7_u8;

    uint8x8x3_t v_r0_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r1_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r2_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r3_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r4_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r5_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r6_u8 = vld3_u8(src); src += src_stride;
    uint8x8x3_t v_r7_u8 = vld3_u8(src);

    CAL_TRANS_C0();
    CAL_TRANS_C1();
    CAL_TRANS_C2();

    vst3_u8(dst, v_d0_u8); dst += dst_stride;
    vst3_u8(dst, v_d1_u8); dst += dst_stride;
    vst3_u8(dst, v_d2_u8); dst += dst_stride;
    vst3_u8(dst, v_d3_u8); dst += dst_stride;
    vst3_u8(dst, v_d4_u8); dst += dst_stride;
    vst3_u8(dst, v_d5_u8); dst += dst_stride;
    vst3_u8(dst, v_d6_u8); dst += dst_stride;
    vst3_u8(dst, v_d7_u8);

    return 0;
}

static int transpose_u8c4_8x8_neon(
        const unsigned char *src_data,
        unsigned char *dst_data,
        int src_stride,
        int dst_stride) {
    const unsigned char *src = src_data;
    unsigned char *dst = dst_data;

    uint8x8x2_t vtr01_u8, vtr23_u8, vtr45_u8, vtr67_u8;
    uint16x4x2_t vtr02_u16, vtr13_u16, vtr46_u16, vtr57_u16;
    uint32x2x2_t vtr04_u32, vtr26_u32, vtr15_u32, vtr37_u32;
    uint8x8x4_t v_d0_u8, v_d1_u8, v_d2_u8, v_d3_u8, v_d4_u8, v_d5_u8, v_d6_u8, v_d7_u8;

    uint8x8x4_t v_r0_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r1_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r2_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r3_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r4_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r5_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r6_u8 = vld4_u8(src); src += src_stride;
    uint8x8x4_t v_r7_u8 = vld4_u8(src);

    CAL_TRANS_C0();
    CAL_TRANS_C1();
    CAL_TRANS_C2();
    CAL_TRANS_C3();

    vst4_u8(dst, v_d0_u8); dst += dst_stride;
    vst4_u8(dst, v_d1_u8); dst += dst_stride;
    vst4_u8(dst, v_d2_u8); dst += dst_stride;
    vst4_u8(dst, v_d3_u8); dst += dst_stride;
    vst4_u8(dst, v_d4_u8); dst += dst_stride;
    vst4_u8(dst, v_d5_u8); dst += dst_stride;
    vst4_u8(dst, v_d6_u8); dst += dst_stride;
    vst4_u8(dst, v_d7_u8);

    return 0;
}

int transpose_u8c1_neon(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sstep,
        unsigned char* dst,
        int dstep) {
    const int s_stride = sstep;
    const int d_stride = dstep;

    const int w_align8 = src_w & (~7);
    const int h_align8 = src_h & (~7);

    unsigned char *dst_col = nullptr;
    const unsigned char *src_row = nullptr;
    unsigned char *dst_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align8; i += 8) {
        src_row = src + i * s_stride;
        dst_row = dst + i;
        transpose_u8c1_8x8_neon(src_row, dst_row, w_align8, s_stride, d_stride);

        for (j = w_align8; j < src_w; j++) {
            const unsigned char *lsrc_row = src + i * s_stride + j;
            dst_col = dst + j * d_stride + i;
            dst_col[0] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[1] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[2] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[3] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[4] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[5] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[6] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[7] = *(lsrc_row);
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + i;
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            dst_col[0] = src_row[j];
            dst_col += d_stride;
        }
    }

    return 0;
}

int transpose_u8c3_neon(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sstep,
        unsigned char* dst,
        int dstep) {
    const int s_stride = sstep;
    const int d_stride = dstep;
    const int w_align8 = src_w & (~7);
    const int h_align8 = src_h & (~7);

    unsigned char *dst_col = nullptr;
    const unsigned char *src_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align8; i += 8) {
        src_row = src + i * s_stride;
        for (j = 0; j < w_align8; j += 8) {
            dst_col = dst + j * d_stride + i * 3;
            transpose_u8c3_8x8_neon(src_row, dst_col, s_stride, d_stride);
            src_row += 24;
        }

        for (; j < src_w; j++) {
            const unsigned char *lsrc_row = src_row;
            dst_col = dst + j * d_stride + i * 3;
            dst_col[0] = lsrc_row[0];
            dst_col[1] = lsrc_row[1];
            dst_col[2] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[3] = lsrc_row[0];
            dst_col[4] = lsrc_row[1];
            dst_col[5] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[6] = lsrc_row[0];
            dst_col[7] = lsrc_row[1];
            dst_col[8] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[9] = lsrc_row[0];
            dst_col[10] = lsrc_row[1];
            dst_col[11] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[12] = lsrc_row[0];
            dst_col[13] = lsrc_row[1];
            dst_col[14] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[15] = lsrc_row[0];
            dst_col[16] = lsrc_row[1];
            dst_col[17] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[18] = lsrc_row[0];
            dst_col[19] = lsrc_row[1];
            dst_col[20] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[21] = lsrc_row[0];
            dst_col[22] = lsrc_row[1];
            dst_col[23] = lsrc_row[2];
            dst_col += d_stride;
            src_row += 3;
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + i * 3;
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            dst_col[0] = src_row[3 * j];
            dst_col[1] = src_row[3 * j + 1];
            dst_col[2] = src_row[3 * j + 2];
            dst_col += d_stride;
        }
    }

    return 0;
}

int transpose_u8c4_neon(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sstep,
        unsigned char* dst,
        int dstep) {
    const int s_stride = sstep;
    const int d_stride = dstep;
    const int w_align8 = src_w & (~7);
    const int h_align8 = src_h & (~7);

    unsigned char *dst_col = nullptr;
    const unsigned char *src_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align8; i += 8) {
        int i_4 = i << 2;
        src_row = src + i * s_stride;
        for (j = 0; j < w_align8; j += 8) {
            dst_col = dst + j * d_stride + i_4;
            transpose_u8c4_8x8_neon(src_row, dst_col, s_stride, d_stride);
            src_row += 32;
        }

        for (; j < src_w; j++) {
            const unsigned char *lsrc_row = src + i * s_stride + j * 4;
            dst_col = dst + j * d_stride + i_4;
            dst_col[0] = lsrc_row[0];
            dst_col[1] = lsrc_row[1];
            dst_col[2] = lsrc_row[2];
            dst_col[3] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[4] = lsrc_row[0];
            dst_col[5] = lsrc_row[1];
            dst_col[6] = lsrc_row[2];
            dst_col[7] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[8] = lsrc_row[0];
            dst_col[9] = lsrc_row[1];
            dst_col[10] = lsrc_row[2];
            dst_col[11] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[12] = lsrc_row[0];
            dst_col[13] = lsrc_row[1];
            dst_col[14] = lsrc_row[2];
            dst_col[15] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[16] = lsrc_row[0];
            dst_col[17] = lsrc_row[1];
            dst_col[18] = lsrc_row[2];
            dst_col[19] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[20] = lsrc_row[0];
            dst_col[21] = lsrc_row[1];
            dst_col[22] = lsrc_row[2];
            dst_col[23] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[24] = lsrc_row[0];
            dst_col[25] = lsrc_row[1];
            dst_col[26] = lsrc_row[2];
            dst_col[27] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[28] = lsrc_row[0];
            dst_col[29] = lsrc_row[1];
            dst_col[30] = lsrc_row[2];
            dst_col[31] = lsrc_row[3];
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + (i << 2);
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            int j_4 = j << 2;
            dst_col[0] = src_row[j_4    ];
            dst_col[1] = src_row[j_4 + 1];
            dst_col[2] = src_row[j_4 + 2];
            dst_col[3] = src_row[j_4 + 3];
            dst_col += d_stride;
        }
    }

    return 0;
}

static void transpose8u_neon(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        unsigned char* dst,
        int dstep) {
    if (1 == sc) {
        transpose_u8c1_neon(src, src_h, src_w, sstep, dst, dstep);
    } else if (3 == sc) {
        transpose_u8c3_neon(src, src_h, src_w, sstep, dst, dstep);
    } else if (4 == sc) {
        transpose_u8c4_neon(src, src_h, src_w, sstep, dst, dstep);
    } else {
        LOG_ERR("transpose channel not support yet!");
    }
}

static int transpose_f32c1_4x4_neon(
        const float *src_data,
        float *dst_data,
        int src_stride,
        int dst_stride) {
    const float *src = src_data;
    float *dst = dst_data;

    float32x4_t v_r0_f32 = vld1q_f32(src);
    src += src_stride;
    float32x4_t v_r1_f32 = vld1q_f32(src);
    src += src_stride;
    float32x4_t v_r2_f32 = vld1q_f32(src);
    src += src_stride;
    float32x4_t v_r3_f32 = vld1q_f32(src);

    float32x4x2_t vtr01_f32 = vtrnq_f32(v_r0_f32, v_r1_f32);// [0] 0 [1] 1
    float32x4x2_t vtr23_f32 = vtrnq_f32(v_r2_f32, v_r3_f32);

    float32x4_t v_d0_f32 = vcombine_f32(vget_low_f32(
            vtr01_f32.val[0]), vget_low_f32(vtr23_f32.val[0]));
    float32x4_t v_d1_f32 = vcombine_f32(vget_low_f32(
            vtr01_f32.val[1]), vget_low_f32(vtr23_f32.val[1]));
    float32x4_t v_d2_f32 = vcombine_f32(vget_high_f32(
            vtr01_f32.val[0]), vget_high_f32(vtr23_f32.val[0]));
    float32x4_t v_d3_f32 = vcombine_f32(vget_high_f32(
            vtr01_f32.val[1]), vget_high_f32(vtr23_f32.val[1]));

    vst1q_f32(dst, v_d0_f32); dst += dst_stride;
    vst1q_f32(dst, v_d1_f32); dst += dst_stride;
    vst1q_f32(dst, v_d2_f32); dst += dst_stride;
    vst1q_f32(dst, v_d3_f32);

    return 0;
}

#define CHANNEL_F32_TRANSPOSE(n)                                                                    \
    vtr01_f32 = vtrnq_f32(v_r0_f32.val[n], v_r1_f32.val[n]);                                                     \
    vtr23_f32 = vtrnq_f32(v_r2_f32.val[n], v_r3_f32.val[n]);                                                     \
    v_d0_f32.val[n] = vcombine_f32(vget_low_f32(vtr01_f32.val[0]), vget_low_f32(vtr23_f32.val[0]));              \
    v_d1_f32.val[n] = vcombine_f32(vget_low_f32(vtr01_f32.val[1]), vget_low_f32(vtr23_f32.val[1]));              \
    v_d2_f32.val[n] = vcombine_f32(vget_high_f32(vtr01_f32.val[0]), vget_high_f32(vtr23_f32.val[0]));            \
    v_d3_f32.val[n] = vcombine_f32(vget_high_f32(vtr01_f32.val[1]), vget_high_f32(vtr23_f32.val[1]));

#define CAL_TRANS_F32_C0() CHANNEL_F32_TRANSPOSE(0)
#define CAL_TRANS_F32_C1() CHANNEL_F32_TRANSPOSE(1)
#define CAL_TRANS_F32_C2() CHANNEL_F32_TRANSPOSE(2)
#define CAL_TRANS_F32_C3() CHANNEL_F32_TRANSPOSE(3)

static int transpose_f32c3_4x4_neon(
        const float *src_data,
        float *dst_data,
        int src_stride,
        int dst_stride) {
    const float *src = src_data;
    float *dst = dst_data;

    float32x4x2_t vtr01_f32, vtr23_f32;
    float32x4x3_t v_d0_f32, v_d1_f32, v_d2_f32, v_d3_f32;

    float32x4x3_t v_r0_f32 = vld3q_f32(src); src += src_stride;
    float32x4x3_t v_r1_f32 = vld3q_f32(src); src += src_stride;
    float32x4x3_t v_r2_f32 = vld3q_f32(src); src += src_stride;
    float32x4x3_t v_r3_f32 = vld3q_f32(src);

    CAL_TRANS_F32_C0();
    CAL_TRANS_F32_C1();
    CAL_TRANS_F32_C2();

    vst3q_f32(dst, v_d0_f32); dst += dst_stride;
    vst3q_f32(dst, v_d1_f32); dst += dst_stride;
    vst3q_f32(dst, v_d2_f32); dst += dst_stride;
    vst3q_f32(dst, v_d3_f32);

    return 0;
}

static int transpose_f32c4_4x4_neon(
        const float *src_data,
        float *dst_data,
        int src_stride,
        int dst_stride) {
    const float *src = src_data;
    float *dst = dst_data;

    float32x4x2_t vtr01_f32, vtr23_f32;
    float32x4x4_t v_d0_f32, v_d1_f32, v_d2_f32, v_d3_f32;

    float32x4x4_t v_r0_f32 = vld4q_f32(src);
    src += src_stride;
    float32x4x4_t v_r1_f32 = vld4q_f32(src);
    src += src_stride;
    float32x4x4_t v_r2_f32 = vld4q_f32(src);
    src += src_stride;
    float32x4x4_t v_r3_f32 = vld4q_f32(src);

    CAL_TRANS_F32_C0();
    CAL_TRANS_F32_C1();
    CAL_TRANS_F32_C2();
    CAL_TRANS_F32_C3();

    vst4q_f32(dst, v_d0_f32); dst += dst_stride;
    vst4q_f32(dst, v_d1_f32); dst += dst_stride;
    vst4q_f32(dst, v_d2_f32); dst += dst_stride;
    vst4q_f32(dst, v_d3_f32);

    return 0;
}

int transpose_f32c1_neon(
        const float* src,
        int src_h,
        int src_w,
        int sstep,
        float* dst,
        int dstep) {
    const int s_stride = sstep / sizeof(float);
    const int d_stride = dstep / sizeof(float);
    const int w_align4 = src_w & (~3);
    const int h_align4 = src_h & (~3);

    float* dst_col = nullptr;
    const float *src_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align4; i += 4) {
        src_row = src + i * s_stride;
        for (j = 0; j < w_align4; j += 4) {
            dst_col = dst + j * d_stride + i;
            transpose_f32c1_4x4_neon(src_row, dst_col, s_stride, d_stride);
            src_row += 4;
        }

        for (; j < src_w; j++) {
            const float *lsrc_row = src_row;
            dst_col = dst + j * d_stride + i;
            dst_col[0] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[1] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[2] = *(lsrc_row);
            lsrc_row += s_stride;
            dst_col[3] = *(lsrc_row);
            dst_col += d_stride;
            src_row++;
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + i;
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            dst_col[0] = src_row[j];
            dst_col += d_stride;
        }
    }

    return 0;
}

int transpose_f32c3_neon(
        const float* src,
        int src_h,
        int src_w,
        int sstep,
        float* dst,
        int dstep) {
    const int s_stride = sstep / sizeof(float);
    const int d_stride = dstep / sizeof(float);
    const int w_align4 = src_w & (~3);
    const int h_align4 = src_h & (~3);

    float* dst_col = nullptr;
    const float* src_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align4; i += 4) {
        src_row = src + i * s_stride;
        for (j = 0; j < w_align4; j += 4) {
            dst_col = dst + j * d_stride + i * 3;
            transpose_f32c3_4x4_neon(src_row, dst_col, s_stride, d_stride);
            src_row += 12;
        }

        for (; j < src_w; j++) {
            const float *lsrc_row = src_row;
            dst_col = dst + j * d_stride + i * 3;
            dst_col[0] = lsrc_row[0];
            dst_col[1] = lsrc_row[1];
            dst_col[2] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[3] = lsrc_row[0];
            dst_col[4] = lsrc_row[1];
            dst_col[5] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[6] = lsrc_row[0];
            dst_col[7] = lsrc_row[1];
            dst_col[8] = lsrc_row[2];
            lsrc_row += s_stride;
            dst_col[9] = lsrc_row[0];
            dst_col[10] = lsrc_row[1];
            dst_col[11] = lsrc_row[2];
            dst_col += d_stride;
            src_row += 3;
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + i * 3;
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            dst_col[0] = src_row[3 * j    ];
            dst_col[1] = src_row[3 * j + 1];
            dst_col[2] = src_row[3 * j + 2];
            dst_col += d_stride;
        }
    }

    return 0;
}

int transpose_f32c4_neon(
        const float* src,
        int src_h,
        int src_w,
        int sstep,
        float* dst,
        int dstep) {
    const int s_stride = sstep / sizeof(float);
    const int d_stride = dstep / sizeof(float);
    const int w_align4 = src_w & (~3);
    const int h_align4 = src_h & (~3);

    float *dst_col = nullptr;
    const float *src_row = nullptr;
    int i = 0, j = 0;
    for (; i < h_align4; i += 4) {
        int i_4 = i << 2;
        src_row = src + i * s_stride;
        for (j = 0; j < w_align4; j += 4) {
            dst_col = dst + j * d_stride + i_4;
            transpose_f32c4_4x4_neon(src_row, dst_col, s_stride, d_stride);
            src_row += 16;
        }

        for (; j < src_w; j++) {
            const float *lsrc_row = src_row;
            dst_col = dst + j * d_stride + i_4;
            dst_col[0] = lsrc_row[0];
            dst_col[1] = lsrc_row[1];
            dst_col[2] = lsrc_row[2];
            dst_col[3] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[4] = lsrc_row[0];
            dst_col[5] = lsrc_row[1];
            dst_col[6] = lsrc_row[2];
            dst_col[7] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[8] = lsrc_row[0];
            dst_col[9] = lsrc_row[1];
            dst_col[10] = lsrc_row[2];
            dst_col[11] = lsrc_row[3];
            lsrc_row += s_stride;
            dst_col[12] = lsrc_row[0];
            dst_col[13] = lsrc_row[1];
            dst_col[14] = lsrc_row[2];
            dst_col[15] = lsrc_row[3];
            dst_col += d_stride;
            src_row += 4;
        }
    }

    for (; i < src_h; i++) {
        dst_col = dst + (i << 2);
        src_row = src + i * s_stride;
        for (j = 0; j < src_w; j++) {
            int j_4 = j << 2;
            dst_col[0] = src_row[j_4    ];
            dst_col[1] = src_row[j_4 + 1];
            dst_col[2] = src_row[j_4 + 2];
            dst_col[3] = src_row[j_4 + 2];
            dst_col += d_stride;
        }
    }

    return 0;
}

static void transpose32f_neon(
        const float* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        float* dst,
        int dstep) {
    if (1 == sc) {
        transpose_f32c1_neon(src, src_h, src_w, sstep, dst, dstep);
    } else if (3 == sc) {
        transpose_f32c3_neon(src, src_h, src_w, sstep, dst, dstep);
    } else if (4 == sc) {
        transpose_f32c4_neon(src, src_h, src_w, sstep, dst, dstep);
    } else {
        LOG_ERR("transpose channel not support yet!");
    }
}

int transpose_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();

    if ((src_w != dst_h) || (src_h != dst_w)) {
        LOG_ERR("size of input or output is not match!");
        return -1;
    }

    int sc = 0;
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::GRAY_F32:
        sc = 1;
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_F32:
    case FCVImageType::PKG_BGR_F32:
        sc = 3;
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
    case FCVImageType::PKG_RGBA_F32:
    case FCVImageType::PKG_BGRA_F32:
        sc = 4;
        break;
    default:
        LOG_ERR("transpose type not support yet!");
        break;
    };

    const int s_stride = src.stride();
    const int d_stride = dst.stride();

    TypeInfo src_type_info;
    int status = get_type_info(src.type(), src_type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    if (src_type_info.data_type == DataType::UINT8) {
        transpose8u_neon(reinterpret_cast<unsigned char*>(src.data()), src_h, src_w,
                sc, s_stride, reinterpret_cast<unsigned char*>(dst.data()), d_stride);
    } else if (src_type_info.data_type == DataType::F32) {
        transpose32f_neon(reinterpret_cast<float*>(src.data()), src_h, src_w,
                sc, s_stride, reinterpret_cast<float*>(dst.data()), d_stride);
    } else {
        return -1;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
