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

#include "modules/img_transform/color_convert/include/color_convert_arm.h"
#ifdef HAVE_SVE2
#include <arm_sve.h>
#include "modules/img_transform/color_convert/include/color_convert_sve.h"
#endif
#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

// R’ = 1.164(Y’– 16) + 1.596(Cr – 128)
// G’ = 1.164(Y’– 16) – 0.813(Cr – 128) – 0.392(Cb – 128)
// B’ = 1.164(Y’– 16) + 2.017(Cb – 128)

// R' = 74.5y + v * 102 - 14216;
// G' = 74.5y + 8696 - 52 * v - 25 * u;
// B' = 74.5y + 129 * u - 17672;

#if __aarch64__
#define YUVTOBGR(vy, vR, vG, vB)                                  \
    "ld2   {v0.8b, v1.8b},   [%" #vy "], #16            \n"  \
    "movi  v16.8b, #16                                 \n"   \
    "umax  v0.8b, v0.8b, v16.8b                        \n"   \
    "umax  v1.8b, v1.8b, v16.8b                        \n"   \
    "umull v0.8h, v0.8b, v24.8b                        \n"   \
    "umull v1.8h, v1.8b, v24.8b                        \n"   \
    "ushr  v0.8h, v0.8h, #1                            \n"   \
    "ushr  v1.8h, v1.8h, #1                            \n"   \
    "shsub  v4.8h, v0.8h, " #vR ".8h                   \n"   \
    "shsub  v5.8h, v1.8h, " #vR ".8h                   \n"   \
    "sqshrun  v4.8b, v4.8h, #5                         \n"   \
    "sqshrun  v5.8b, v5.8h, #5                         \n"   \
    "shsub  v11.8h, v0.8h, " #vG ".8h                  \n"   \
    "shsub  v12.8h, v1.8h, " #vG ".8h                  \n"   \
    "sqshrun  v11.8b, v11.8h, #5                       \n"   \
    "sqshrun  v12.8b, v12.8h, #5                       \n"   \
    "shsub  v15.8h, v0.8h, " #vB ".8h                  \n"   \
    "shsub  v16.8h, v1.8h, " #vB ".8h                  \n"   \
    "sqshrun  v15.8b, v15.8h, #5                       \n"   \
    "sqshrun  v16.8b, v16.8h, #5                       \n"   \
    "zip1     v22.8b, v4.8b, v5.8b                     \n"  /*bl*/ \
    "zip2     v7.8b, v4.8b, v5.8b                      \n"  /*bh*/ \
    "zip1     v21.8b, v11.8b, v12.8b                   \n"  /*bl*/ \
    "zip2     v14.8b, v11.8b, v12.8b                   \n"  /*bh*/ \
    "ins      v22.d[1], v7.d[0]                        \n" /* Extract rh */  \
    "zip1     v20.8b, v15.8b, v16.8b                   \n"  /*bl*/ \
    "zip2     v18.8b, v15.8b, v16.8b                   \n"  /*bh*/ \
    "ins      v21.d[1], v14.d[0]                       \n" /* Extract gh */  \
    "ins      v20.d[1], v18.d[0]                       \n" /* Extract bh */  \

#define YUVTOBGR_COEFFCAL                                  \
    "ld2r  {v30.8b, v31.8b}, [%[UVCoeffBR]]            \n"    /*UVCoeffBR, 129 102*/ \
    "ld2r  {v25.8b, v26.8b}, [%[UVCoeffG]]             \n"    /*UVCoeffG, 25 52*/    \
    "ld3r  {v27.8h, v28.8h, v29.8h}, [%[UVBiasBGR]]    \n"    /*UVBiasBGR, 17672 8696 14216*/ \
    "ld1r  {v24.8b},   [%[UVcoeffY]]                   \n" \

#define NV12UVCAL                                         \
    "ld2   {v2.8b, v3.8b}, [%2], #16                  \n"    /*vld2_u8(uv)*/     \
    "subs %w5, %w5, #1                                \n"                        \
    "prfm  pldl1keep, [%2, #256]                      \n"                        \
    "prfm  pldl1keep, [%0, #256]                      \n"                        \
    "prfm  pldl1keep, [%1, #256]                      \n"                        \
    "mov   v10.16b,  v29.16b                          \n"                        \
    "umull v9.8h,   v2.8b,  v25.8b                    \n"    /*u * 25  g*/       \
    "umlsl v10.8h,  v3.8b, v31.8b                     \n"  /*R 14216 - 102*v */  \
    "mov   v8.16b,   v27.16b                          \n"                        \
    "umlal v9.8h,  v3.8b, v26.8b                      \n"  /*gu * 25 + v * 52 */ \
    "umlsl v8.8h,  v2.8b, v30.8b                      \n"  /*b 17672 - 129*u */  \
    "sub   v9.8h,  v9.8h, v28.8h                      \n"                        \

#define NV21UVCAL                                         \
    "ld2   {v2.8b, v3.8b}, [%2], #16                  \n"  \
    "subs %w5, %w5, #1                                \n"  \
    "prfm  pldl1keep, [%2, #256]                      \n"  \
    "prfm  pldl1keep, [%0, #256]                      \n"  \
    "prfm  pldl1keep, [%1, #256]                      \n"  \
    "mov   v10.16b,  v29.16b                          \n"  \
    "umull v9.8h, v3.8b,  v25.8b                      \n"  \
    "mov   v8.16b,  v27.16b                           \n"  \
    "umlsl v10.8h,  v2.8b, v31.8b                     \n"  \
    "umlal v9.8h, v2.8b, v26.8b                       \n"  \
    "umlsl v8.8h,  v3.8b, v30.8b                      \n"  \
    "sub   v9.8h, v9.8h, v28.8h                       \n"

#define YUV420UVCAL                                        \
    "ld1   {v2.8b}, [%2], #8                          \n"  \
    "subs %w6, %w6, #1                                \n"  \
    "prfm  pldl1keep, [%2, #256]                      \n"  \
    "prfm  pldl1keep, [%0, #256]                      \n"  \
    "prfm  pldl1keep, [%1, #256]                      \n"  \
    "prfm  pldl1keep, [%3, #256]                      \n"  \
    "ld1   {v3.8b}, [%3], #8                          \n"  \
    "mov   v10.16b,  v29.16b                          \n"  \
    "umull v9.8h, v2.8b,  v25.8b                      \n"  \
    "mov   v8.16b, v27.16b                            \n"  \
    "umlsl v10.8h,  v3.8b, v31.8b                     \n"  \
    "umlal v9.8h, v3.8b, v26.8b                       \n"  \
    "umlsl v8.8h,  v2.8b, v30.8b                      \n"  \
    "sub   v9.8h, v9.8h, v28.8h                       \n"

#define ASM_PARAM                                               \
    "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5",     \
    "v7", "v8", "v9", "v10", "v11", "v12", "v14", "v15",    \
    "v16", "v18", "v19", "v20", "v21", "v22", "v23", "v24", \
    "v25", "v26", "v27", "v28", "v29", "v30", "v31"
#else

#define YUVTOBGR(vy, vR, vG, vB)                                 \
    "vld2.u8   {d0, d2},   [%" #vy "]!                 \n"   \
    "vmov.u8  d26, #16                                 \n"   \
    "vmax.u8  d0, d26                                  \n"   \
    "vmax.u8  d2, d26                                  \n"   \
    "vmull.u8 q0, d0, d27                              \n"   \
    "vshr.u16  q0, #1                                  \n"   \
    "vmull.u8 q1, d2, d27                              \n"   \
    "vshr.u16  q1, #1                                  \n"   \
    "vhsub.s16 q2, q0, " #vR "                         \n"   \
    "vhsub.s16 q3, q1, " #vR "                         \n"   \
    "vqrshrun.s16  d4, q2, #5                          \n"   \
    "vqrshrun.s16  d5, q3, #5                          \n"   \
    "vhadd.s16 q4, q0, " #vG "                         \n"   \
    "vhadd.s16 q5, q1, " #vG "                         \n"   \
    "vqrshrun.s16  d6, q4, #5                          \n"   \
    "vqrshrun.s16  d7, q5, #5                          \n"   \
    "vhsub.s16 q0, q0, " #vB "                         \n"   \
    "vhsub.s16 q1, q1, " #vB "                         \n"   \
    "vqrshrun.s16  d8, q0, #5                          \n"   \
    "vqrshrun.s16  d9, q1, #5                          \n"   \
    "vzip.8   d4, d5                                   \n"   \
    "vzip.8   d6, d7                                   \n"   \
    "vzip.8   d8, d9                                   \n"   \

#define YUVTOBGR_COEFFCAL                                  \
    "vld3.u16  {d12, d14, d16}, [%[UVBiasBGR]]         \n" /* q6:17672 q7 8696 */ \
    "vmov.i8	d30, #129                              \n" \
    "vmov.i8	d31, #102                              \n" \
    "vmov.i8	d28, #25                               \n" \
    "vld3.u16  {d13, d15, d17}, [%[UVBiasBGR]]         \n" /* q8:14216 */         \
    "vmov.i8	d29, #52                               \n" \
    "vmov.i8	d27, #149                              \n" \

#define NV12UVCAL                                         \
    "vld2.u8   {d4, d6}, [%2]!                        \n"  /*vld2_u8(uv)*/      \
    "subs %5, %5, #1                                  \n"                       \
    "pld [%2, #128]                                   \n"                       \
    "pld [%0, #128]                                   \n"                       \
    "pld [%1, #128]                                   \n"                       \
    "vmov.u16  q9, q8                                 \n"                       \
    "vmlsl.u8  q9,  d6, d31                           \n"  /*R 14216 - 102*v */ \
    "vmov.u16  q10, q7                                \n"                       \
    "vmlsl.u8  q10, d4, d28                           \n"  /*G 8696 - u * 25 */ \
    "vmov.u16  q11, q6                                \n"                       \
    "vmlsl.u8  q10, d6, d29                           \n"  /*8696 - u * 25 - v * 52 */ \
    "vmlsl.u8  q11, d4, d30                           \n"  /*B 17672 - 129*u */  \

#define NV21UVCAL                                             \
    "vld2.u8   {d4, d6}, [%2]!                        \n" \
    "subs %5, %5, #1                                  \n" \
    "pld [%2, #128]                                   \n" \
    "pld [%0, #128]                                   \n" \
    "pld [%1, #128]                                   \n" \
    "vmov.u16  q9, q8                                 \n" \
    "vmlsl.u8  q9,  d4, d31                           \n" \
    "vmov.u16  q10, q7                                \n" \
    "vmlsl.u8  q10, d6, d28                           \n" \
    "vmov.u16  q11, q6                                \n" \
    "vmlsl.u8  q10, d4, d29                           \n" \
    "vmlsl.u8  q11, d6, d30                           \n" \

#define YUV420UVCAL                                       \
    "vld1.8   d4, [%2]!                               \n" \
    "subs  %6, %6, #1                                 \n" \
    "vld1.8   d6, [%3]!                               \n" \
    "pld [%2, #128]                                   \n" \
    "pld [%3, #128]                                   \n" \
    "pld [%0, #128]                                   \n" \
    "pld [%1, #128]                                   \n" \
    "vmov.u16  q9, q8                                 \n" \
    "vmov.u16  q10, q7                                \n" \
    "vmlsl.u8  q10, d4, d28                           \n" \
    "vmlsl.u8  q9,  d6, d31                           \n" \
    "vmov.u16  q11, q6                                \n" \
    "vmlsl.u8  q10, d6, d29                           \n" \
    "vmlsl.u8  q11, d4, d30                           \n" \

#define ASM_PARAM                                           \
    "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", \
    "q6", "q7", "q8", "q9", "q10", "q11", "q12",        \
    "q13", "q14", "q15"
#endif

#define PARAM1                                             \
    [_y0]"+r"(y0_ptr),                                 \
    [_y1]"+r"(y1_ptr),                                 \
    [_vu]"+r"(uv),                                     \
    [_d0]"+r"(dst0_ptr),                               \
    [_d1]"+r"(dst1_ptr),                               \
    [_nn]"+r"(nn)

#define PARAM2                                             \
    [UVCoeffBR]"r"(&YuvI601Constants.UVCoeffBR),       \
    [UVCoeffG]"r"(&YuvI601Constants.UVCoeffG),         \
    [UVBiasBGR]"r"(&YuvI601Constants.UVBiasBGR),       \
    [UVcoeffY]"r"(&YuvI601Constants.UVcoeffY)

static void convert_nv12_to_bgr_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_stride * src_h;
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;

        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV12UVCAL
          YUVTOBGR(0, v10, v9, v8)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
          YUVTOBGR(1, v10, v9, v8)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48    \n"
          "bne 0b                                           \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                             \n"
          NV12UVCAL
          YUVTOBGR(0, q11, q10, q9)
          "vst3.8  {d4, d6, d8}, [%[_d0]]!                \n"
          "vst3.8  {d5, d7, d9}, [%[_d0]]!                \n"
          YUVTOBGR(1, q11, q10, q9)
          "vst3.8  {d4, d6, d8}, [%[_d1]]!                \n"
          "vst3.8  {d5, d7, d9}, [%[_d1]]!                \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + width_align8 * 3,
                    uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, true, 0, 2, 3);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv21_to_bgr_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;

        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV21UVCAL
          YUVTOBGR(0, v10, v9, v8)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
          YUVTOBGR(1, v10, v9, v8)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48    \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV21UVCAL
          YUVTOBGR(0, q11, q10, q9)
          "vst3.8  {d4, d6, d8}, [%[_d0]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d0]]!    \n"
          YUVTOBGR(1, q11, q10, q9)
          "vst3.8  {d4, d6, d8}, [%[_d1]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d1]]!    \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);

#endif

        if (dst_w - width_align8) {
        convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + width_align8 * 3,
                uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, false, 0, 2, 3);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv12_to_rgb_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;

        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV12UVCAL
          YUVTOBGR(0, v8, v9, v10)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
          YUVTOBGR(1, v8, v9, v10)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48    \n"
          "bne 0b                                           \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV12UVCAL
          YUVTOBGR(0, q9, q10, q11)
          "vst3.8  {d4, d6, d8}, [%[_d0]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d0]]!    \n"
          YUVTOBGR(1, q9, q10, q11)
          "vst3.8  {d4, d6, d8}, [%[_d1]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d1]]!    \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif
        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + width_align8 * 3,
                    uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, true, 2, 0, 3);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv21_to_rgb_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;

        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV21UVCAL
          YUVTOBGR(0, v8, v9, v10)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
          YUVTOBGR(1, v8, v9, v10)
          "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48    \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                \n"
          NV21UVCAL
          YUVTOBGR(0, q9, q10, q11)
          "vst3.8  {d4, d6, d8}, [%[_d0]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d0]]!    \n"
          YUVTOBGR(1, q9, q10, q11)
          "vst3.8  {d4, d6, d8}, [%[_d1]]!    \n"
          "vst3.8  {d5, d7, d9}, [%[_d1]]!    \n"
          "bne 0b                                         \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8,
                    dst_ptr + width_align8 * 3, uv_ptr + width_align8,
                    src_w, (n << 4), src_stride, dst_stride, false, 2, 0, 3);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_yuv420_to_bgr_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;

    const unsigned char *u = uv_ptr;
    const unsigned char *v = uv_ptr + src_stride * (src_h >> 2);
    for (; i < src_h; i += 2) {
        nn = n;
        unsigned char *dst0_ptr = dst_ptr;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* y1_ptr = src_ptr + src_stride;
        const unsigned char* u0 = u;
        const unsigned char* v0 = v;
#if __aarch64__
        asm volatile(
            YUVTOBGR_COEFFCAL
            "0:                                                \n"
            YUV420UVCAL
            YUVTOBGR(0, v10, v9, v8)
            "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
            YUVTOBGR(1, v10, v9, v8)
            "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48   \n"
            "bne 0b                                          \n"
            : [_y0]"+r"(y0_ptr),
              [_y1]"+r"(y1_ptr),
              [_u]"+r"(u0),
              [_v]"+r"(v0),
              [_d0]"+r"(dst0_ptr),
              [_d1]"+r"(dst1_ptr),
              [_nn]"+r"(nn)
            : PARAM2
            : ASM_PARAM);
#else
        asm volatile(
            YUVTOBGR_COEFFCAL
            "0:                                                \n"
            YUV420UVCAL
            YUVTOBGR(0, q11, q10, q9)
            "vst3.8  {d4, d6, d8}, [%[_d0]]!    \n"
            "vst3.8  {d5, d7, d9}, [%[_d0]]!    \n"
            YUVTOBGR(1, q11, q10, q9)
            "vst3.8  {d4, d6, d8}, [%[_d1]]!    \n"
            "vst3.8  {d5, d7, d9}, [%[_d1]]!    \n"
            "bne 0b                                         \n"
            : [_y0]"+r"(y0_ptr),
              [_y1]"+r"(y1_ptr),
              [_u]"+r"(u0),
              [_v]"+r"(v0),
              [_d0]"+r"(dst0_ptr),
              [_d1]"+r"(dst1_ptr),
              [_nn]"+r"(nn)
            : PARAM2
            : ASM_PARAM);
#endif
        if (dst_w - width_align8) {
            int uv_off = width_align8 >> 1;
            convet_yuv420_to_bgr_one_row(src_ptr + width_align8, dst_ptr + width_align8 * 3, 
                    u + uv_off, v + uv_off, src_w, src_stride, dst_stride, width_align8, 3);
        }

        src_ptr += doub_src_stride;
        u += (src_stride >> 1);
        v += (src_stride >> 1);
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv12_to_bgra_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* y1_ptr = src_ptr + src_stride;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "movi        v23.16b, #255                             \n" /* A */
          "0:                                                    \n"
          NV12UVCAL
          YUVTOBGR(0, v10, v9, v8)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d0]], #64  \n"
          YUVTOBGR(1, v10, v9, v8)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d1]], #64  \n"
          "bne 0b                                                 \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                    \n"
          NV12UVCAL
          YUVTOBGR(0, q11, q10, q9)
          "vmov.u8	q5, #255                                     \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d0]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d0]]!                  \n"
          YUVTOBGR(1, q11, q10, q9)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d1]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d1]]!                  \n"
          "bne 0b                                                \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
        convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + (width_align8 << 2),
                uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, true, 0, 2, 4);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv21_to_bgra_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;
        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "movi        v23.16b, #255                             \n" /* A */
          "0:                                                    \n"
          NV21UVCAL
          YUVTOBGR(0, v10, v9, v8)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d0]], #64  \n"
          YUVTOBGR(1, v10, v9, v8)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d1]], #64  \n"
          "bne 0b                                                 \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                    \n"
          NV21UVCAL
          YUVTOBGR(0, q11, q10, q9)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d0]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d0]]!                  \n"
          YUVTOBGR(1, q11, q10, q9)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d1]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d1]]!                  \n"
          "bne 0b                                                \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + (width_align8 << 2),
                uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, false, 0, 2, 4);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv12_to_rgba_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;
        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__
        asm volatile(
          YUVTOBGR_COEFFCAL
          "movi        v23.16b, #255                             \n" /* A */
          "0:                                                    \n"
          NV12UVCAL
          YUVTOBGR(0, v8, v9, v10)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d0]], #64  \n"
          YUVTOBGR(1, v8, v9, v10)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d1]], #64  \n"
          "bne 0b                                                 \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                    \n"
          NV12UVCAL
          YUVTOBGR(0, q9, q10, q11)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d0]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d0]]!                  \n"
          YUVTOBGR(1, q9, q10, q11)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d1]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d1]]!                  \n"
          "bne 0b                                                \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr +
                    (width_align8 << 2), uv_ptr + width_align8, src_w,
                    (n << 4), src_stride, dst_stride, true, 2, 0, 4);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

static void convert_nv21_to_rgba_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const int dst_w = dst.width();
    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    const unsigned char *uv_ptr = src_ptr + src_h * src_stride;

    int n = width_align8 >> 4;
    int i = 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char* y0_ptr = src_ptr;
        const unsigned char* uv = uv_ptr;
        unsigned char *dst0_ptr = dst_ptr;
        const unsigned char* y1_ptr = src_ptr + src_stride;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

#if __aarch64__

        asm volatile(
          YUVTOBGR_COEFFCAL
          "movi        v23.16b, #255                             \n" /* A */
          "0:                                                    \n"
          NV21UVCAL
          YUVTOBGR(0, v8, v9, v10)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d0]], #64  \n"
          YUVTOBGR(1, v8, v9, v10)
          "st4  {v20.16b,v21.16b,v22.16b,v23.16b}, [%[_d1]], #64  \n"
          "bne 0b                                                 \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#else
        asm volatile(
          YUVTOBGR_COEFFCAL
          "0:                                                    \n"
          NV21UVCAL
          YUVTOBGR(0, q9, q10, q11)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d0]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d0]]!                  \n"
          YUVTOBGR(1, q9, q10, q11)
          "vmov.u8	q5, #255                                    \n"
          "vst4.8  {d4, d6, d8, d10}, [%[_d1]]!                  \n"
          "vst4.8  {d5, d7, d9, d11}, [%[_d1]]!                  \n"
          "bne 0b                                                \n"
          : PARAM1
          : PARAM2
          : ASM_PARAM);
#endif

        if (dst_w - width_align8) {
            convet_yuv_to_one_row(src_ptr + width_align8, dst_ptr + (width_align8 << 2),
                    uv_ptr + width_align8, src_w, (n << 4), src_stride, dst_stride, false, 2, 0, 4);
        }

        src_ptr += doub_src_stride;
        uv_ptr += src_stride;
        dst_ptr += doub_dst_stride;
    }
}

void convert_yuv420_to_bgr_neon(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst) {
    const int src_w = src_y.width();
    const int src_h = src_y.height();
    const int src_stride = src_y.stride();
    const int dst_w = dst.width();
    const int dst_stride = dst.stride();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));

    int width_align8 = dst_w & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;

    const unsigned char *y_ptr = (const unsigned char *)src_y.data();
    const unsigned char *u = (const unsigned char *)src_u.data();
    const unsigned char *v = (const unsigned char *)src_v.data();

    int n = width_align8 >> 4;

    int i= 0;
    int nn = 0;
    for (; i < src_h; i += 2) {
        nn = n;
        const unsigned char *y0_ptr = y_ptr;
        const unsigned char *y1_ptr = y_ptr + src_stride;

        unsigned char *dst0_ptr = dst_ptr;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;

        const unsigned char *u0 = u;
        const unsigned char *v0 = v;

#if __aarch64__
        asm volatile(
            YUVTOBGR_COEFFCAL
            "0:                                                \n"
            YUV420UVCAL
            YUVTOBGR(0, v10, v9, v8)
            "st3  {v20.16b,v21.16b,v22.16b}, [%[_d0]], #48    \n"
            YUVTOBGR(1, v10, v9, v8)
            "st3  {v20.16b,v21.16b,v22.16b}, [%[_d1]], #48   \n"
            "bne 0b                                          \n"
            : [_y0]"+r"(y0_ptr),
              [_y1]"+r"(y1_ptr),
              [_u]"+r"(u0),
              [_v]"+r"(v0),
              [_d0]"+r"(dst0_ptr),
              [_d1]"+r"(dst1_ptr),
              [_nn]"+r"(nn)
            : PARAM2
            : ASM_PARAM);
#else
        asm volatile(
            YUVTOBGR_COEFFCAL
            "0:                                                \n"
            YUV420UVCAL
            YUVTOBGR(0, q11, q10, q9)
            "vst3.8  {d4, d6, d8}, [%[_d0]]!    \n"
            "vst3.8  {d5, d7, d9}, [%[_d0]]!    \n"
            YUVTOBGR(1, q11, q10, q9)
            "vst3.8  {d4, d6, d8}, [%[_d1]]!    \n"
            "vst3.8  {d5, d7, d9}, [%[_d1]]!    \n"
            "bne 0b                                         \n"
            : [_y0]"+r"(y0_ptr),
              [_y1]"+r"(y1_ptr),
              [_u]"+r"(u0),
              [_v]"+r"(v0),
              [_d0]"+r"(dst0_ptr),
              [_d1]"+r"(dst1_ptr),
              [_nn]"+r"(nn)
            : PARAM2
            : ASM_PARAM);
#endif
        if (dst_w - width_align8) {
            int uv_off = width_align8 >> 1;
            convet_yuv420_to_bgr_one_row(y_ptr + width_align8, dst_ptr + width_align8 * 3, 
                    u + uv_off, v + uv_off, dst_w, src_stride, dst_stride, width_align8, 3);
        }

        y_ptr += doub_src_stride;
        u += (src_stride >> 1);
        v += (src_stride >> 1);
        dst_ptr += doub_dst_stride;
    }
 }

#if __aarch64__
/**************************** bgr->yuv420 armv8 implementation**************************/

#define BGR_TO_YUV420_COEFFS_PARAMS         \
    uint8x16_t vyr = vdupq_n_u8(YR);        \
    uint8x16_t vyg = vdupq_n_u8(YG);        \
    uint8x16_t vyb = vdupq_n_u8(YB);        \
    uint16x8_t vvr = vdupq_n_u16(VR);       \
    int16x8_t  vvg = vdupq_n_s16(VG);       \
    int16x8_t  vvb = vdupq_n_s16(VB);       \
    int16x8_t  vur = vdupq_n_s16(UR);       \
    int16x8_t  vug = vdupq_n_s16(UG);       \
    uint16x8_t vub = vdupq_n_u16(UB);       \
    uint16x8_t vc16  = vdupq_n_s16(YC);     \
    uint16x8_t vc128 = vdupq_n_s16(UVC);

#define BGRTOY(vB, vG, vR)                            \
    "umull       v11.8h, " #vB ".8b,  %[vyb].8b   \n" \
    "umull2      v12.8h, " #vB ".16b, %[vyb].16b  \n" \
    "umlal       v11.8h, " #vG ".8b,  %[vyg].8b   \n" \
    "umlal2      v12.8h, " #vG ".16b, %[vyg].16b  \n" \
    "umlal       v11.8h, " #vR ".8b,  %[vyr].8b   \n" \
    "umlal2      v12.8h, " #vR ".16b, %[vyr].16b  \n" \
    "uqadd       v11.8h, v11.8h, %[vc16].8h       \n" \
    "uqadd       v12.8h, v12.8h, %[vc16].8h       \n" \
    "uqrshrn     v11.8b, v11.8h, #8               \n" \
    "uqrshrn2    v11.16b, v12.8h, #8              \n"

#define BGRTOUV(vB, vG, vR)                 \
    "mul        v13.8h, " #vR ", %[vvr].8h        \n" /* V                    */ \
    "mul        v14.8h, " #vR ", %[vur].8h        \n" /* U                    */ \
    "mla        v13.8h, " #vG ", %[vvg].8h        \n" /* V                    */ \
    "mla        v14.8h, " #vG ", %[vug].8h        \n" /* U                    */ \
    "mla        v13.8h, " #vB ", %[vvb].8h        \n" /* V                    */ \
    "mla        v14.8h, " #vB ", %[vub].8h        \n" /* U                    */ \
    "add        v13.8h, v13.8h, %[vc128].8h       \n" /* +128 -> unsigned     */ \
    "add        v14.8h, v14.8h, %[vc128].8h       \n" /* +128 -> unsigned     */ \
    "uqrshrn     v13.8b, v13.8h, #8               \n" /* 16 bit to 8 bit V    */ \
    "uqrshrn     v14.8b, v14.8h, #8               \n" /* 16 bit to 8 bit U    */

#define BGRA_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */ \
    "ld4         {v0.16b,v1.16b,v2.16b,v3.16b}, [%0], #64 \n"  /* load 16 pixels.       */ \
    "subs        %w5, %w5, #16                  \n"            /* 8 processed per loop. */ \
    "uaddlp      v8.8h, v0.16b                  \n"            /* B b00 + b01.          */ \
    "prfm        pldl1keep, [%0, 448]           \n"                                        \
    "uaddlp      v9.8h, v1.16b                  \n"            /* G g00 + g01 */           \
    "uaddlp      v10.8h, v2.16b                 \n"            /* R r00 + r01 */           \
    "ld4         {v4.16b,v5.16b,v6.16b,v7.16b}, [%1], #64 \n"  /* load next 16 */          \
    "prfm        pldl1keep, [%1, 448]           \n"                                        \
                                                                                           \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, open the codes below */ \
    /*"uadalp      v8.8h, v4.16b                  \n"                                        \
    "uadalp      v9.8h, v5.16b                  \n"                                        \
    "uadalp      v10.8h, v6.16b                 \n"                                        \*/ \
                                                                                           \
    "urshr       v8.8h, v8.8h, #1              \n"  /* b g r */                            \
    "urshr       v9.8h, v9.8h, #1              \n"                                         \
    "urshr       v10.8h, v10.8h, #1            \n"

#define BGR_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */ \
    "ld3         {v0.16b,v1.16b,v2.16b}, [%0], #48 \n"  /* load 16 pixels.       */ \
    "subs        %w5, %w5, #16                  \n"            /* 8 processed per loop. */ \
    "uaddlp      v8.8h, v0.16b                  \n"            /* B b00 + b01.          */ \
    "prfm        pldl1keep, [%0, 448]           \n"                                        \
    "uaddlp      v9.8h, v1.16b                  \n"            /* G g00 + g01 */           \
    "uaddlp      v10.8h, v2.16b                 \n"            /* R r00 + r01 */           \
    "ld3         {v4.16b,v5.16b,v6.16b}, [%1], #48 \n"  /* load next 16 */          \
    "prfm        pldl1keep, [%1, 448]           \n"                                        \
                                                                                           \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, open the codes below */ \
    /*"uadalp      v8.8h, v4.16b                 \n"         \
    "uadalp      v9.8h, v5.16b                  \n"         \
    "uadalp      v10.8h, v6.16b                 \n"         */                             \
                                                                                           \
    "urshr       v8.8h, v8.8h, #1              \n"  /* b g r */                            \
    "urshr       v9.8h, v9.8h, #1              \n"                                         \
    "urshr       v10.8h, v10.8h, #1            \n"

#define RGBA_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */ \
    "ld4         {v0.16b,v1.16b,v2.16b,v3.16b}, [%0], #64 \n"  /* load 16 pixels.       */ \
    "subs        %w5, %w5, #16                  \n"            /* 8 processed per loop. */ \
    "uaddlp      v8.8h, v2.16b                  \n"            /* B b00 + b01.          */ \
    "prfm        pldl1keep, [%0, 448]           \n"                                        \
    "uaddlp      v9.8h, v1.16b                  \n"            /* G g00 + g01 */           \
    "uaddlp      v10.8h, v0.16b                 \n"            /* R r00 + r01 */           \
    "ld4         {v4.16b,v5.16b,v6.16b,v7.16b}, [%1], #64 \n"  /* load next 16 */          \
    "prfm        pldl1keep, [%1, 448]           \n"                                        \
                                                                                           \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, open the codes below */ \
    /*"uadalp      v8.8h, v6.16b                  \n"        \
    "uadalp      v9.8h, v5.16b                  \n"         \
    "uadalp      v10.8h, v4.16b                 \n"         */                             \
                                                                                           \
    "urshr       v8.8h, v8.8h, #1              \n"  /* b g r */                            \
    "urshr       v9.8h, v9.8h, #1              \n"                                         \
    "urshr       v10.8h, v10.8h, #1            \n"

#define RGB_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */ \
    "ld3         {v0.16b,v1.16b,v2.16b}, [%0], #48 \n"  /* load 16 pixels.       */ \
    "subs        %w5, %w5, #16                  \n"            /* 8 processed per loop. */ \
    "uaddlp      v8.8h, v2.16b                  \n"            /* B b00 + b01.          */ \
    "prfm        pldl1keep, [%0, 448]           \n"                                        \
    "uaddlp      v9.8h, v1.16b                  \n"            /* G g00 + g01 */           \
    "uaddlp      v10.8h, v0.16b                 \n"            /* R r00 + r01 */           \
    "ld3         {v4.16b,v5.16b,v6.16b}, [%1], #48 \n"  /* load next 16 */          \
    "prfm        pldl1keep, [%1, 448]           \n"                                        \
                                                                                           \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, open the codes below */ \
   /* "uadalp      v8.8h, v6.16b                  \n"        \
    "uadalp      v9.8h, v5.16b                  \n"        \
    "uadalp      v10.8h, v4.16b                 \n"        */                              \
                                                                                           \
    "urshr       v8.8h, v8.8h, #1              \n"  /* b g r */                            \
    "urshr       v9.8h, v9.8h, #1              \n"                                         \
    "urshr       v10.8h, v10.8h, #1            \n"

#define BGR_TO_YUV420_PARAM2                  \
      [vyr]"w"(vyr),                               \
      [vyg]"w"(vyg),                               \
      [vyb]"w"(vyb),                               \
      [vvr]"w"(vvr),                               \
      [vvg]"w"(vvg),                               \
      [vvb]"w"(vvb),                               \
      [vur]"w"(vur),                               \
      [vug]"w"(vug),                               \
      [vub]"w"(vub),                               \
      [vc16]"w"(vc16),                              \
      [vc128]"w"(vc128)

#define BGR_TO_YUV420_ASM_PARAM              \
    "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",  "v9", "v10", "v11", "v12", \
                "v13", "v14", "v15", "v16"

#else

/**************************** bgr->yuv420 armv7 implementation**************************/
#define BGR_TO_YUV420_COEFFS_PARAMS        \

#define BGRTOY(vB0, vB1, vG0, vG1, vR0, vR1)           \
    "vmov.u8     d12, #66                \n"  /* r */  \
    "vmov.u8     d13, #129               \n"  /* g */  \
    "vmov.u8     d14, #25                \n"  /* b */  \
    "vmov.u8     d15, #16                \n"  /* 16 */ \
    "vmull.u8    q13, " #vB0 ", d14      \n" \
    "vmull.u8    q14, " #vB1 ", d14      \n" \
    "vmlal.u8    q13, " #vG0 ", d13      \n" \
    "vmlal.u8    q14, " #vG1 ", d13      \n" \
    "vmlal.u8    q13, " #vR0 ", d12      \n" \
    "vmlal.u8    q14, " #vR1 ", d12      \n" \
    "vqrshrn.u16 d26, q13,  #8           \n" \
    "vqrshrn.u16 d27, q14,  #8           \n" \
    "vqadd.u8    d26, d26,  d15          \n" \
    "vqadd.u8    d27, d27,  d15          \n"

#define BGRTOUV(vB, vG, vR)                 \
    "vmov.s16    q11, #112                \n"      \
    "vmov.s16    q12, #94                 \n"      \
    "vmov.s16    q13, #74                 \n"      \
    "vmov.s16    q14, #18                 \n"      \
    "vmov.s16    q15, #38                 \n"      \
    "vmul.s16    q10, " #vR ", q11        \n" /* V                    */ \
    "vmul.s16    q11, " #vB ", q11        \n" /* U                    */ \
    "vmls.s16    q10, " #vG ", q12        \n" /* V                    */ \
    "vmls.s16    q11, " #vG ", q13        \n" /* U                    */ \
    "vmls.s16    q10, " #vB ", q14        \n" /* V                    */ \
    "vmls.s16    q11, " #vR ", q15        \n" /* U                    */ \
    "vmov.u16    q15, #0x8000             \n"      \
    "vadd.u16   q10, q10, q15             \n" /* +128 -> unsigned     */ \
    "vadd.u16   q11, q11, q15             \n" /* +128 -> unsigned     */

#define BGRA_TO_YUV420_UV_CAL                   \
/* calculate the uv plane value in the interpolation method of linear */         \
    "vld4.8       {d0, d2, d4, d6}, [%0]!      \n"  /* load 8 pixels.       */   \
    "vld4.8       {d1, d3, d5, d7}, [%0]!      \n"  /* load 8 pixels.       */   \
    "subs         %5, %5, #16                  \n"  /* 16 processed per loop. */ \
    "vpaddl.u8    q7, q0                       \n"  /* B b00 + b01.          */  \
    "pld [%0, #128]                            \n"                               \
    "vpaddl.u8    q8, q1                       \n"  /* G g00 + g01 */            \
    "vpaddl.u8    q9, q2                       \n"  /* R r00 + r01 */            \
    "vld4.8       {d6, d8, d10, d12}, [%1]!    \n"  /* load 8 pixels.    */      \
    "vld4.8       {d7, d9, d11, d13}, [%1]!    \n"  /* load 8 pixels.    */      \
    "pld [%1, #128]                            \n"                               \
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, 
        open the codes below */ \
    /*"vpadal.u8      q7, q3                     \n"                               \
    "vpadal.u8      q8, q4                     \n"                               \
    "vpadal.u8      q9, q5                     \n"                               */\
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear,
        set the shift value to 2, else if linear interpolation method set to 1 */ \
    "vrshr.u16      q7, q7, #1                 \n"  /* b g r */                  \
    "vrshr.u16      q8, q8, #1                 \n"                               \
    "vrshr.u16      q9, q9, #1                 \n"

#define BGR_TO_YUV420_UV_CAL                 \
/* calculate the uv plane value in the interpolation method of linear */         \
    "vld3.8       {d0, d2, d4}, [%0]!          \n"  /* load 8 pixels.       */   \
    "vld3.8       {d1, d3, d5}, [%0]!          \n"  /* load 8 pixels.       */   \
    "subs         %5, %5, #16                  \n"  /* 16 processed per loop. */ \
    "vpaddl.u8    q7, q0                       \n"  /* B b00 + b01.          */  \
    "pld [%0, #128]                            \n"                               \
    "vpaddl.u8    q8, q1                       \n"  /* G g00 + g01 */            \
    "vpaddl.u8    q9, q2                       \n"  /* R r00 + r01 */            \
    "vld3.8       {d6, d8, d10}, [%1]!         \n"  /* load 8 pixels.    */      \
    "vld3.8       {d7, d9, d11}, [%1]!         \n"  /* load 8 pixels.    */      \
    "pld [%1, #128]                            \n"                               \
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, 
        open the codes below */ \
    /*"vpadal.u8      q7, q3                     \n"                               \
    "vpadal.u8      q8, q4                     \n"                               \
    "vpadal.u8      q9, q5                     \n"                               */\
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear,
        set the shift value to 2, else if linear interpolation method set to 1 */ \
    "vrshr.u16      q7, q7, #1                 \n"  /* b g r */                  \
    "vrshr.u16      q8, q8, #1                 \n"                               \
    "vrshr.u16      q9, q9, #1                 \n"

#define RGBA_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */         \
    "vld4.8       {d0, d2, d4, d6}, [%0]!      \n"  /* load 8 pixels.       */   \
    "vld4.8       {d1, d3, d5, d7}, [%0]!      \n"  /* load 8 pixels.       */   \
    "subs         %5, %5, #16                  \n"  /* 16 processed per loop. */ \
    "vpaddl.u8    q7, q2                       \n"  /* B b00 + b01.          */  \
    "pld [%0, #128]                            \n"                               \
    "vpaddl.u8    q8, q1                       \n"  /* G g00 + g01 */            \
    "vpaddl.u8    q9, q0                       \n"  /* R r00 + r01 */            \
    "vld4.8       {d6, d8, d10, d12}, [%1]!    \n"  /* load 8 pixels.    */      \
    "vld4.8       {d7, d9, d11, d13}, [%1]!    \n"  /* load 8 pixels.    */      \
    "pld [%1, #128]                            \n"                               \
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, 
        open the codes below */ \
    /*"vpadal.u8      q7, q5                     \n"                               \
    "vpadal.u8      q8, q4                     \n"                               \
    "vpadal.u8      q9, q3                     \n"                               */\
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear,
        set the shift value to 2, else if linear interpolation method set to 1 */ \
    "vrshr.u16      q7, q7, #1                 \n"  /* b g r */                  \
    "vrshr.u16      q8, q8, #1                 \n"                               \
    "vrshr.u16      q9, q9, #1                 \n"

#define RGB_TO_YUV420_UV_CAL                \
/* calculate the uv plane value in the interpolation method of linear */         \
    "vld3.8       {d0, d2, d4}, [%0]!          \n"  /* load 8 pixels.       */   \
    "vld3.8       {d1, d3, d5}, [%0]!          \n"  /* load 8 pixels.       */   \
    "subs         %5, %5, #16                  \n"  /* 16 processed per loop. */ \
    "vpaddl.u8    q7, q2                       \n"  /* B b00 + b01.          */  \
    "pld [%0, #128]                            \n"                               \
    "vpaddl.u8    q8, q1                       \n"  /* G g00 + g01 */            \
    "vpaddl.u8    q9, q0                       \n"  /* R r00 + r01 */            \
    "vld3.8       {d6, d8, d10}, [%1]!         \n"  /* load 8 pixels.    */      \
    "vld3.8       {d7, d9, d11}, [%1]!         \n"  /* load 8 pixels.    */      \
    "pld [%1, #128]                            \n"                               \
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear, 
        open the codes below */ \
    /*"vpadal.u8      q7, q5                     \n"                               \
    "vpadal.u8      q8, q4                     \n"                               \
    "vpadal.u8      q9, q3                     \n"                              */ \
                                                                                 \
    /* if need to calculate the uv plane value in the interpolation method of bilinear,
        set the shift value to 2, else if linear interpolation method set to 1 */ \
    "vrshr.u16      q7, q7, #1                 \n"  /* b g r */                  \
    "vrshr.u16      q8, q8, #1                 \n"                               \
    "vrshr.u16      q9, q9, #1                 \n"

#define BGR_TO_YUV420_ASM_PARAM              \
    "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", \
                "q13", "q14", "q15"

#endif

#define BGR_TO_YUV420_PARAM1                  \
      "+r"(y0_ptr),                        \
      "+r"(y1_ptr),                        \
      "+r"(dst0_ptr),                      \
      "+r"(dst1_ptr),                      \
      "+r"(uv),                            \
      "+r"(nn)

void bgra2nv12_neon(
        const unsigned char* src_bgra,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgra;
        const unsigned char *y1_ptr = src_bgra + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                "0:                                         \n"
                BGRA_TO_YUV420_UV_CAL
                BGRTOY(v0, v1, v2)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v4, v5, v6)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v14.8b, v13.8b         \n"  // UV
                "zip2       v16.8b, v14.8b, v13.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);
#else
    asm volatile (
                "0:                                         \n"
                BGRA_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d21, q10, #8                    \n" /* V    */
                "vqshrn.u16 d20, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d0, d1, d2, d3, d4, d5)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d6, d7, d8, d9, d10, d11)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);

#endif
        }

        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, true, 0, 2, 4);
        }

        src_bgra += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void bgra2nv21_neon(
        const unsigned char* src_bgra,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgra;
        const unsigned char *y1_ptr = src_bgra + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                BGRA_TO_YUV420_UV_CAL
                BGRTOY(v0, v1, v2)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v4, v5, v6)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v13.8b, v14.8b         \n"  // VU
                "zip2       v16.8b, v13.8b, v14.8b         \n"  //
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                BGRA_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d20, q10, #8                    \n" /* V    */
                "vqshrn.u16 d21, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d0, d1, d2, d3, d4, d5)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d6, d7, d8, d9, d10, d11)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, false, 0, 2, 4);
        }

        src_bgra += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv  += dst_stride;
    }
}

void rgba2nv12_neon(
        const unsigned char* src_bgra,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgra;
        const unsigned char *y1_ptr = src_bgra + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                RGBA_TO_YUV420_UV_CAL
                BGRTOY(v2, v1, v0)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v6, v5, v4)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v14.8b, v13.8b         \n"  // UV
                "zip2       v16.8b, v14.8b, v13.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                RGBA_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d21, q10, #8                    \n" /* V    */
                "vqshrn.u16 d20, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d4, d5, d2, d3, d0, d1)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d10, d11, d8, d9, d6, d7)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, true, 2, 0, 4);
        }

        src_bgra += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void rgba2nv21_neon(
        const unsigned char* src_bgra,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgra;
        const unsigned char *y1_ptr = src_bgra + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                RGBA_TO_YUV420_UV_CAL
                BGRTOY(v2, v1, v0)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v6, v5, v4)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v13.8b, v14.8b         \n"  // VU
                "zip2       v16.8b, v13.8b, v14.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                RGBA_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d20, q10, #8                    \n" /* V    */
                "vqshrn.u16 d21, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d4, d5, d2, d3, d0, d1)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d10, d11, d8, d9, d6, d7)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, false, 2, 0, 4);
        }

        src_bgra += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void bgr2nv12_neon(
        const unsigned char* src_bgr,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgr;
        const unsigned char *y1_ptr = src_bgr + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                BGR_TO_YUV420_UV_CAL
                BGRTOY(v0, v1, v2)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v4, v5, v6)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v14.8b, v13.8b         \n"  // UV
                "zip2       v16.8b, v14.8b, v13.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                BGR_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d21, q10, #8                    \n" /* V    */
                "vqshrn.u16 d20, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d0, d1, d2, d3, d4, d5)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d6, d7, d8, d9, d10, d11)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, true, 0, 2, 3);
        }

        src_bgr += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void bgr2nv21_neon(
        const unsigned char* src_bgr,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    //const int parallel_size = width_align16 * 3;
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_bgr;
        const unsigned char *y1_ptr = src_bgr + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                BGR_TO_YUV420_UV_CAL
                BGRTOY(v0, v1, v2)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v4, v5, v6)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v13.8b, v14.8b         \n"  // VU
                "zip2       v16.8b, v13.8b, v14.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                BGR_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d20, q10, #8                    \n" /* V    */
                "vqshrn.u16 d21, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d0, d1, d2, d3, d4, d5)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d6, d7, d8, d9, d10, d11)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, false, 0, 2, 3);
        }

        src_bgr += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void rgb2nv12_neon(
        const unsigned char* src_rgb,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_rgb;
        const unsigned char *y1_ptr = src_rgb + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                RGB_TO_YUV420_UV_CAL
                BGRTOY(v2, v1, v0)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v6, v5, v4)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v14.8b, v13.8b         \n" // UV
                "zip2       v16.8b, v14.8b, v13.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                RGB_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d21, q10, #8                    \n" /* V    */
                "vqshrn.u16 d20, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d4, d5, d2, d3, d0, d1)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d10, d11, d8, d9, d6, d7)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, true, 2, 0, 3);
        }

        src_rgb += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void rgb2nv21_neon(
        const unsigned char* src_rgb,
        unsigned char* dst_y,
        unsigned char* dst_uv,
        const int width,
        const int height,
        const int src_stride,
        const int dst_stride) {
    const int width_align16 = width & (~15);
    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;
    BGR_TO_YUV420_COEFFS_PARAMS

    int i = 0;
    for (; i < height; i += 2) {
        int nn = width_align16;
        const unsigned char *y0_ptr = src_rgb;
        const unsigned char *y1_ptr = src_rgb + src_stride;

        unsigned char *dst0_ptr = dst_y;
        unsigned char *dst1_ptr = dst_y + dst_stride;
        unsigned char *uv = dst_uv;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                // calculate the uv plane value in the interpolation method of linear
                "0:                                         \n"
                RGB_TO_YUV420_UV_CAL
                BGRTOY(v2, v1, v0)
                BGRTOUV(v8.8h, v9.8h, v10.8h)
                "st1         {v11.16b}, [%2], #16          \n"  // store 16 pixels Y0.
                BGRTOY(v6, v5, v4)
                "st1         {v11.16b}, [%3], #16          \n"  // store 16 pixels Y1.
                "zip1       v15.8b, v13.8b, v14.8b         \n"  // VU
                "zip2       v16.8b, v13.8b, v14.8b         \n"
                "ins	    v15.d[1], v16.d[0]             \n"
                "st1        {v15.16b}, [%4], #16           \n"  // store 16 pixels UV.
                "b.gt       0b                             \n"

            : BGR_TO_YUV420_PARAM1
            : BGR_TO_YUV420_PARAM2
            : BGR_TO_YUV420_ASM_PARAM);

#else
            asm volatile (
                "0:                                         \n"
                RGB_TO_YUV420_UV_CAL
                BGRTOUV(q7, q8, q9)
                "vqshrn.u16 d20, q10, #8                    \n" /* V    */
                "vqshrn.u16 d21, q11, #8                    \n" /* U    */
                "vst2.8      {d20, d21}, [%4]!              \n"
                BGRTOY(d4, d5, d2, d3, d0, d1)
                "vst1.8      {q13}, [%2]!                   \n"  // store 16 pixels Y0
                BGRTOY(d10, d11, d8, d9, d6, d7)
                "vst1.8      {q13}, [%3]!                   \n"  // store 16 pixels Y1
                "bgt       0b                               \n"

            : BGR_TO_YUV420_PARAM1
            :
            : BGR_TO_YUV420_ASM_PARAM);
#endif
        }
        if (width - width_align16) {
            convert_to_yuv_one_row(y0_ptr, dst0_ptr, uv, width,
                    src_stride, width_align16, false, 2, 0, 3);
        }

        src_rgb += doub_src_stride;
        dst_y += doub_dst_stride;
        dst_uv += dst_stride;
    }
}

void convert_bgr_to_yuv420_neon(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();

    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    CHECK_CVT_SIZE((src_w == dst_w) && (src_h == dst_h));

    const int src_stride = src.stride();
    const int dst_stride = dst.stride();

    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    unsigned char *uv_ptr = dst_ptr + dst_h * dst_w;

    switch (cvt_type) {
        case ColorConvertType::CVT_PA_BGRA2NV12:
            bgra2nv12_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_BGRA2NV21:
            bgra2nv21_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_RGBA2NV12:
            rgba2nv12_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_RGBA2NV21:
            rgba2nv21_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_BGR2NV12:
            bgr2nv12_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_BGR2NV21:
            bgr2nv21_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_RGB2NV12:
            rgb2nv12_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        case ColorConvertType::CVT_PA_RGB2NV21:
            rgb2nv21_neon(src_ptr, dst_ptr, uv_ptr, src_w, src_h, src_stride, dst_stride);
            break;
        default:
            LOG_ERR("cvt type not support yet!");
            break;
    }
}

#if __aarch64__
#define CVTTOGRAY(b_parm, g_parm, r_parm)              \
    "prfm  pldl1keep, [%0, #192]                      \n" \
    "ld3         {v3.16b,v4.16b,v5.16b}, [%0], #48    \n"  /* load 16 pixels.       */  \
    "subs         %w2, %w2, #32                       \n"  /* 16 processed per loop. */ \
    "ld3         {v10.16b,v11.16b,v12.16b}, [%0], #48 \n"  /* load 16 pixels.       */  \
    "umull       v6.8h, " #b_parm ".8b , v3.8b        \n" \
    "umull2      v7.8h, " #b_parm ".16b, v3.16b       \n" \
    "umull       v8.8h, " #b_parm ".8b , v10.8b       \n" \
    "umull2      v9.8h, " #b_parm ".16b, v10.16b      \n" \
    "umlal       v6.8h, " #g_parm ".8b,  v4.8b        \n" \
    "umlal2      v7.8h, " #g_parm ".16b, v4.16b       \n" \
    "umlal       v8.8h, " #g_parm ".8b,  v11.8b       \n" \
    "umlal2      v9.8h, " #g_parm ".16b, v11.16b      \n" \
    "umlal       v6.8h, " #r_parm ".8b,  v5.8b        \n" \
    "umlal2      v7.8h, " #r_parm ".16b, v5.16b       \n" \
    "umlal       v8.8h, " #r_parm ".8b,  v12.8b       \n" \
    "umlal2      v9.8h, " #r_parm ".16b, v12.16b      \n" \
    "uqrshrn     v6.8b, v6.8h, #8                     \n" \
    "uqrshrn2    v6.16b, v7.8h, #8                    \n" \
    "uqrshrn     v8.8b, v8.8h, #8                     \n" \
    "uqrshrn2    v8.16b, v9.8h, #8                    \n" \
    "st1         {v6.16b}, [%1], #16                  \n" /* store 16 pixels.       */ \
    "st1         {v8.16b}, [%1], #16                  \n" /* store 16 pixels.       */

#else

#define CVTTOGRAY(b_parm, g_parm, r_parm)              \
    "vld3.8       {d6, d8, d10}, [%0]!                \n"  /* load 8 pixels.       */  \
    "vld3.8       {d7, d9, d11}, [%0]!                \n"  /* load 8 pixels.       */  \
    "vld3.8       {d20, d22, d24}, [%0]!              \n"  /* load 8 pixels.       */  \
    "vld3.8       {d21, d23, d25}, [%0]!              \n"  /* load 8 pixels.       */  \
    "subs         %2, %2, #32                         \n"  /* 16 processed per loop. */ \
    "vmull.u8     q6, " #b_parm ", d6                 \n" \
    "vmull.u8     q7, " #b_parm ", d7                 \n" \
    "vmlal.u8     q6, " #g_parm ", d8                 \n" \
    "vmlal.u8     q7, " #g_parm ", d9                 \n" \
    "pld          [%0, #128]                          \n" \
    "vmlal.u8     q6, " #r_parm ", d10                \n" \
    "vmlal.u8     q7, " #r_parm ", d11                \n" \
    "vmull.u8     q8, " #b_parm ", d20                \n" \
    "vmull.u8     q9, " #b_parm ", d21                \n" \
    "vmlal.u8     q8, " #g_parm ", d22                \n" \
    "vmlal.u8     q9, " #g_parm ", d23                \n" \
    "vmlal.u8     q8, " #r_parm ", d24                \n" \
    "vmlal.u8     q9, " #r_parm ", d25                \n" \
    "vqrshrn.u16  d12, q6, #8                         \n" \
    "vqrshrn.u16  d13, q7, #8                         \n" \
    "vqrshrn.u16  d16, q8, #8                         \n" \
    "vqrshrn.u16  d17, q9, #8                         \n" \
    "vst1.8      {q6}, [%1]!                          \n" \
    "vst1.8      {q8}, [%1]!                          \n"

#endif

void bgr2gray_neon(
        const unsigned char* src,
        unsigned char* dst,
        int width,
        int height,
        int src_stride,
        int dst_stride) {
    // Coalesce rows.
    if ((src_stride == width * 3) && (dst_stride == width)) {
        width *= height;
        height = 1;
        src_stride = dst_stride = 0;
    }

    int width_align8 = width & (~31);
    int remain = width - width_align8;
    int i = 0;
    for (i = 0; i < height; i++) {
        const unsigned char *src_ptr0 = src;
        unsigned char *dst_ptr0 = dst;
        int nn = width_align8;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                "movi       v0.16b, #29                    \n" \
                "movi       v1.16b, #150                   \n" \
                "movi       v2.16b, #77                    \n" \
                "0:                                        \n"
                CVTTOGRAY(v0, v1, v2)
                "b.gt       0b                             \n"
                :"+r"(src_ptr0),
                 "+r"(dst_ptr0),
                 "+r"(nn)
                :
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6",    \
                "v7", "v8", "v9", "v10", "v11", "v12");
#else
            asm volatile (
                "vmov.u8      d0, #29                       \n"
                "vmov.u8      d1, #150                      \n"
                "vmov.u8      d2, #77                       \n"
                "0:                                        \n"
                CVTTOGRAY(d0, d1, d2)
                "bgt        0b                             \n"
                :"+r"(src_ptr0),
                 "+r"(dst_ptr0),
                 "+r"(nn)
                :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6",    \
                "q7", "q8", "q9", "q10", "q11", "q12");
#endif
    }

        if (remain) {
            for (int j = 0; j < remain; j++) {
                unsigned char b = src_ptr0[0];
                unsigned char g = src_ptr0[1];
                unsigned char r = src_ptr0[2];

                int value = r * R_RATION + g * G_RATION + b * B_RATION;
                *dst_ptr0 = (unsigned char)(value >> Q);
                src_ptr0 += 3;
                dst_ptr0 += 1;
            }
        }

        src += src_stride;
        dst += dst_stride;
    }
}

void rgb2gray_neon(
        const unsigned char* src,
        unsigned char* dst,
        int width,
        int height,
        int src_stride,
        int dst_stride) {
    // Coalesce rows.
    if ((src_stride == width * 3) && (dst_stride == width)) {
        width *= height;
        height = 1;
        src_stride = dst_stride = 0;
    }

    int width_align8 = width & (~31);
    int remain = width - width_align8;

    int i = 0;
    for (i = 0; i < height; i++) {
        const unsigned char *src_ptr0 = src;
        unsigned char *dst_ptr0 = dst;
        int nn = width_align8;

        if (nn > 0) {
#if __aarch64__
            asm volatile (
                "movi       v0.16b, #29                    \n"
                "movi       v1.16b, #150                   \n"
                "movi       v2.16b, #77                    \n"
                "0:                                        \n"
                CVTTOGRAY(v2, v1, v0)
                "b.gt       0b                             \n"
                :"+r"(src_ptr0),
                 "+r"(dst_ptr0),
                 "+r"(nn)
                :
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6",    \
                "v7", "v8", "v9", "v10", "v11", "v12");
#else
        asm volatile (
                "vmov.u8      d0, #29                    \n"
                "vmov.u8      d1, #150                   \n"
                "vmov.u8      d2, #77                    \n"
                "0:                                     \n"
                CVTTOGRAY(d2, d1, d0)
                "bgt        0b                         \n"
                :"+r"(src_ptr0),
                 "+r"(dst_ptr0),
                 "+r"(nn)
                :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6",    \
                "q7", "q8", "q9", "q10", "q11", "q12");

#endif
     }

        if (remain) {
            for (int j = 0; j < remain; j++) {
                int value = src_ptr0[0] * R_RATION + src_ptr0[1] * G_RATION + src_ptr0[2] * B_RATION;
                *dst_ptr0 = (unsigned char)(value >> Q);
                src_ptr0 += 3;
                dst_ptr0 += 1;
            }
        }

        src += src_stride;
        dst += dst_stride;
    }
}

void convert_bgr_to_gray_neon(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

#ifdef HAVE_SVE2    
    if (cvt_type == ColorConvertType::CVT_PA_BGR2GRAY){
        bgr2gray_sve(src_ptr, dst_ptr, src_w, src_h, src_stride, dst_stride)
    } else if (cvt_type == ColorConvertType::CVT_PA_RGB2GRAY) {
        rgb2gray_sve(src_ptr, dst_ptr, src_w, src_h, src_stride, dst_stride);
    }
#else
    if (cvt_type == ColorConvertType::CVT_PA_BGR2GRAY){
        bgr2gray_neon(src_ptr, dst_ptr, src_w, src_h, src_stride, dst_stride);
    } else if (cvt_type == ColorConvertType::CVT_PA_RGB2GRAY) {
        rgb2gray_neon(src_ptr, dst_ptr, src_w, src_h, src_stride, dst_stride);
    }
#endif
}

class BGR_RGBConverterParallelTask : public ParallelTask {
public:
    BGR_RGBConverterParallelTask(
            int src_stride,
            const unsigned char* src_ptr,
            int dst_stride,
            unsigned char* dst_ptr,
            int count,
            int remain)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _count(count),
            _remain(remain) {}

    void operator()(const Range& range) const override {
        for (int i = range.start(); i < range.end(); ++i) {
            const unsigned char* src_bgr = _src_ptr + i * _src_stride;
            unsigned char* dst_rgb = _dst_ptr + i * _dst_stride;

             int nn = _count;

             if (nn > 0) {
  #if __aarch64__
                  asm volatile(
                      "0:                                                \n"
                      "prfm  pldl1keep, [%0, #128]                       \n"
                      "ld3 {v0.8b, v1.8b, v2.8b}, [%0], #24              \n"  // load 16 pixels of BGR.
                      "subs        %w2, %w2, #8                          \n"  // 16 processed per loop
                      "mov   v3.8b,  v1.8b                               \n"
                      "mov   v4.8b,  v0.8b                               \n"
                      "st3 {v2.8b, v3.8b, v4.8b}, [%1], #24              \n"
                      "b.gt        0b                                    \n"
                      : "+r"(src_bgr),              // %0
                      "+r"(dst_rgb),                // %1
                      "+r"(nn)                      // %2
                      :
                  : "cc", "memory", "v0", "v1", "v2", "v3", "v4");
  #else
                  asm volatile(
                      "0:                                              \n"
                      "pld         [%0, #128]                          \n"
                      "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
                      "subs        %2, %2, #8                         \n"
                      "vswp.u8     d0, d4                              \n"  // swap R, B
                      "vst3.8      {d0, d2, d4},   [%1]!               \n" // store 24 pixels of BGR.
                      "bgt        0b                                   \n"
                      : "+r"(src_bgr),                // %0
                      "+r"(dst_rgb),                  // %1
                      "+r"(nn)                        // %2
                      :
                  : "cc", "memory", "q0", "q1", "q2", "q3");
  #endif
             }

              if (_remain) {
                 const unsigned char *src_ptr0 = src_bgr;
                 unsigned char *dst_ptr0 = dst_rgb;

                 for (int j = 0; j < _remain; j++) {
                     unsigned char b00 = src_ptr0[0];
                     unsigned char g00 = src_ptr0[1];
                     unsigned char r00 = src_ptr0[2];
    
                     dst_ptr0[0] = r00;
                     dst_ptr0[1] = g00;
                     dst_ptr0[2] = b00;
    
                     src_ptr0 += 3;
                     dst_ptr0 += 3;
                 }
             }
         }
    }

private:
    int _src_stride;
    const unsigned char* _src_ptr;
    int _dst_stride;
    unsigned char* _dst_ptr;
    int _count;
    int _remain;
};

void convert_bgr_to_rgb_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    int count = src_w & (~7);
    int remain = src_w - count;

    BGR_RGBConverterParallelTask task(src_stride,
            src_ptr, dst_stride, dst_ptr, count, remain);

    parallel_run(Range(0, src_h), task);
}

void convert_bgr_to_rgba_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    // Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 3)
            && (dst_stride == dst.width() << 2)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int count = src_w & (~31);
    int remain = src_w - count;
    int paralle_size = count * 3;

    int y = 0;
    for (y = 0; y < src_h; y++) {
        int nn = count;

        const unsigned char *src_bgr = src_ptr;
        unsigned char *dst_rgba = dst_ptr;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "movi       v5.16b,  #255                          \n"  //  for a 255
                "movi       v11.16b, #255                          \n"  // for a 255
                "0:                                                \n"
                "prfm  pldl1keep, [%0, 384]                        \n"
                "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n"  // load 16 pixels of BGR.
                "mov   v3.16b,  v1.16b                             \n"
                "mov   v4.16b,  v0.16b                             \n"
                "ld3 {v6.16b, v7.16b, v8.16b}, [%0], #48           \n"  // load 16 pixels of BGR.
                "subs        %w2, %w2, #32                         \n"  // 32 processed per loop
                "mov   v9.16b,  v7.16b                             \n"
                "st4 {v2.16b, v3.16b, v4.16b, v5.16b}, [%1], #64   \n"
                "mov   v10.16b,  v6.16b                            \n"
                "st4 {v8.16b, v9.16b, v10.16b, v11.16b}, [%1], #64 \n"
                "b.gt        0b                                    \n"
                : "+r"(src_bgr),               // %0
                "+r"(dst_rgba),                // %1
                "+r"(nn)                       // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
#else
            asm volatile(
                "vmov.u8    q3, #255                             \n" 
                "vmov.u8    q7, #255                            \n" 
                "0:                                              \n"
                "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
                "vld3.8      {d1, d3, d5}, [%0]!                 \n"  // next 8 RGB
                "pld         [%0, #128]                          \n"
                "vswp.u8     d0, d4                              \n"  // swap R, B
                "vswp.u8     d1, d5                              \n"
                "vld3.8      {d8, d10, d12}, [%0]!               \n"  // load 8 pixels of BGR.
                "subs        %2, %2, #32                         \n"
                "vld3.8      {d9, d11, d13}, [%0]!               \n"  // next 8 RGB
                "vswp.u8     d8, d12                             \n"  // swap R, B
                "vswp.u8     d9, d13                             \n"
                "vst4.8      {d0, d2, d4, d6},   [%1]!           \n" // store 24 pixels of BGR.
                "vst4.8      {d1, d3, d5, d7},   [%1]!           \n"
                "vst4.8      {d8, d10, d12, d14}, [%1]!          \n" // store 24 pixels of BGR.
                "vst4.8      {d9, d11, d13, d15}, [%1]!          \n"
                "bgt        0b                                   \n"
                : "+r"(src_bgr),                 // %0
                "+r"(dst_rgba),                  // %1
                "+r"(nn)                         // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7");
#endif
    }

        if (remain) {
            const unsigned char *src_ptr0 = src_ptr + paralle_size;
            unsigned char *dst_ptr0 = dst_ptr + (count << 2);
            for (int j = 0; j < remain; j++) {
                unsigned char b00 = src_ptr0[0];
                unsigned char g00 = src_ptr0[1];
                unsigned char r00 = src_ptr0[2];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;
                dst_ptr0[3] = 255;

                src_ptr0 += 3;
                dst_ptr0 += 4;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_bgr_to_bgra_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    // Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 3)
            && (dst_stride == dst.width() << 2)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int count = src_w & (~7);
    int remain = src_w - count;
    int paralle_size = count * 3;

    int y = 0;
    for (y = 0; y < src_h; y++) {
        int nn = count;

        const unsigned char *src_bgr = src_ptr;
        unsigned char *dst_bgra = dst_ptr;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "movi       v3.8b,  #255                           \n"  //  for a 255
                "0:                                                \n"
                // "prfm  pldl1keep, [%0, 448]                        \n"
                "ld3 {v0.8b, v1.8b, v2.8b}, [%0], #24              \n"  // load 16 pixels of BGR.
                "subs        %w2, %w2, #8                          \n"  // 32 processed per loop
                "st4 {v0.8b, v1.8b, v2.8b, v3.8b}, [%1], #32       \n"
                "b.gt        0b                                    \n"
                : "+r"(src_bgr),               // %0
                "+r"(dst_bgra),                // %1
                "+r"(nn)                       // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3");
#else
            asm volatile(
                "vmov.u8    q3, #255                             \n" 
                "0:                                              \n"
                "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
                "pld         [%0, #128]                          \n"
                "subs        %2, %2, #8                         \n"
                "vst4.8      {d0, d2, d4, d6},   [%1]!           \n" // store 24 pixels of BGR.
                "bgt        0b                                   \n"
                : "+r"(src_bgr),                 // %0
                "+r"(dst_bgra),                  // %1
                "+r"(nn)                         // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
    }

        if (remain) {
            const unsigned char *src_ptr0 = src_ptr + paralle_size;
            unsigned char *dst_ptr0 = dst_ptr + (count << 2);
            for (int j = 0; j < remain; j++) {
                unsigned char b00 = src_ptr0[0];
                unsigned char g00 = src_ptr0[1];
                unsigned char r00 = src_ptr0[2];

                dst_ptr0[0] = b00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = r00;
                dst_ptr0[3] = 255;

                src_ptr0 += 3;
                dst_ptr0 += 4;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_bgra_to_bgr_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    // Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 4)
            && (dst_stride == dst.width() * 3)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int count = src_w & (~15);
    int remain = src_w - count;
    int paralle_size = count * 3;

    int y = 0;
    for (y = 0; y < src_h; ++y) {
        int nn = count;

        const unsigned char *src_bgra = src_ptr;
        unsigned char *dst_bgr = dst_ptr;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                \n"
                // "prfm  pldl1keep, [%0, 448]                        \n"
                "ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64   \n"  // load 16 pixels of BGR.
                "subs        %w2, %w2, #16                         \n"  // 32 processed per loop
                "st3 {v0.16b, v1.16b, v2.16b}, [%1], #48           \n"
                "b.gt        0b                                    \n"
                : "+r"(src_bgra),             // %0
                "+r"(dst_bgr),                // %1
                "+r"(nn)                      // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3");
#else
            asm volatile(
                "0:                                              \n"
                "vld4.8      {d0, d2, d4, d6}, [%0]!             \n"  // load 8 pixels of BGR.
                "vld4.8      {d1, d3, d5, d7}, [%0]!             \n"  // next 8 RGB
                "subs        %2, %2, #16                         \n"
                "pld         [%0, #448]                           \n"
                "vst3.8      {d0, d2, d4}, [%1]!                 \n" // store 24 pixels of BGR.
                "vst3.8      {d1, d3, d5}, [%1]!                 \n"
                "bgt         0b                                  \n"
                : "+r"(src_bgra),             // %0
                "+r"(dst_bgr),                // %1
                "+r"(nn)                      // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
    }

    if (remain) {
        const unsigned char *src_ptr0 = src_ptr + (count << 2);
        unsigned char *dst_ptr0 = dst_ptr + paralle_size;
        for (int j = 0; j < remain; j++) {
            unsigned char b00 = src_ptr0[0];
            unsigned char g00 = src_ptr0[1];
            unsigned char r00 = src_ptr0[2];

            dst_ptr0[0] = b00;
            dst_ptr0[1] = g00;
            dst_ptr0[2] = r00;

            src_ptr0 += 4;
            dst_ptr0 += 3;
        }
    }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_bgra_to_rgb_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    // Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w << 2)
            && (dst_stride == dst.width() * 3)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int count = src_w & (~15);
    int remain = src_w - count;
    int paralle_size = count << 2;

    int y = 0;
    for (y = 0; y < src_h; y++) {
        int nn = count;
        const unsigned char *src_bgra = src_ptr;
        unsigned char *dst_rgba = dst_ptr;

    if (nn > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                \n"
                "ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64   \n"  // load 16 pixels of BGR.
                "subs        %w2, %w2, #16                         \n"  // 32 processed per loop
                "prfm  pldl1keep, [%0, 448]                       \n"
                "mov   v3.16b,  v1.16b                             \n"
                "mov   v4.16b,  v0.16b                             \n"
                "st3 {v2.16b, v3.16b, v4.16b}, [%1], #48           \n"
                "b.gt        0b                                    \n"
                : "+r"(src_bgra),              // %0
                "+r"(dst_rgba),                // %1
                "+r"(nn)                      // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4");
#else
            asm volatile(
                "0:                                              \n"
                "pld         [%0, #448]                           \n"
                "vld4.8      {d0, d2, d4, d6}, [%0]!             \n"  // load 8 pixels of RGBA.
                "vld4.8      {d1, d3, d5, d7}, [%0]!             \n"
                "vswp.u8     d4,  d0                             \n"
                "vswp.u8     d1,  d5                             \n"
                "subs        %2, %2, #16                         \n"
                "vst3.8 {d0, d2, d4}, [%1]!                      \n" // store 24 pixels of BGR.
                "vst3.8 {d1, d3, d5}, [%1]!                      \n"
                "bgt        0b                                   \n"
                : "+r"(src_bgra),              // %0
                "+r"(dst_rgba),                // %1
                "+r"(nn)                      // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
    }

        if (remain) {
            const unsigned char *src_ptr0 = src_ptr + paralle_size;
            unsigned char *dst_ptr0 = dst_ptr + (count * 3);
            for (int j = 0; j < remain; j++) {
                unsigned char b00 = src_ptr0[0];
                unsigned char g00 = src_ptr0[1];
                unsigned char r00 = src_ptr0[2];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;

                src_ptr0 += 4;
                dst_ptr0 += 3;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

// Shuffle table for converting BGRA to RGBA.
unsigned char bgra_to_rgba_tab[16] = {2u, 1u, 0u,
    3u, 6u, 5u, 4u, 7u, 10u, 9u, 8u, 11u, 14u, 13u, 12u, 15u};

void convert_bgra_to_rgba_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    // Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w << 2)
            && (dst_stride == dst.width() << 2)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    const unsigned char* shuffle = (const unsigned char*)(&bgra_to_rgba_tab);

    int count = src_w & (~15);
    int remain = src_w - count;
    int paralle_size = count << 2;

    int y = 0;
    for (y = 0; y < src_h; y++) {
        int nn = count;

        const unsigned char *src_bgra = src_ptr;
        unsigned char *dst_rgba = dst_ptr;

        if (nn > 0) {
#if __aarch64__
            asm volatile(
                "ld1         {v7.16b}, [%3]                \n"  // shuffler
                "0:                                        \n"
                "ld1         {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64   \n"  // load 32 pixels.
                "subs        %w2, %w2, #16                 \n"  // 16 processed per loop
                "prfm        pldl1keep, [%0, 448]         \n"
                "tbl         v0.16b, {v0.16b}, v7.16b      \n"  // look up 16 pixels
                "tbl         v1.16b, {v1.16b}, v7.16b      \n"  // look up 16 pixels
                "tbl         v2.16b, {v2.16b}, v7.16b      \n"  // look up 16 pixels
                "tbl         v3.16b, {v3.16b}, v7.16b      \n"  // look up 16 pixels
                "st1         {v0.16b, v1.16b, v2.16b, v3.16b}, [%1], #64   \n"  // store 32.
                "b.gt        0b                            \n"
                : "+r"(src_bgra),                   // %0
                  "+r"(dst_rgba),                   // %1
                  "+r"(nn)                         // %2
                : "r"(shuffle)                     // %3
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v7");
#else
            asm volatile(
                "vld1.8      {q7}, [%3]!                 \n"
                "0:                                      \n"
                "pld         [%0, #256]                  \n"
                "vld1.8      {q0, q1}, [%0]!             \n"
                "vld1.8      {q2, q3}, [%0]!             \n"
                "subs        %2, %2, #16                 \n"
                "vtbl.8      d0, {d0, d1}, d14           \n"
                "vtbl.8      d1, {d0, d1}, d15           \n"
                "vtbl.8      d2, {d2, d3}, d14           \n"
                "vtbl.8      d3, {d2, d3}, d15           \n"
                "vtbl.8      d4, {d4, d5}, d14           \n"
                "vtbl.8      d5, {d4, d5}, d15           \n"
                "vtbl.8      d6, {d6, d7}, d14           \n"
                "vtbl.8      d7, {d6, d7}, d15           \n"
                "vst1.8      {q0, q1}, [%1]!             \n"  // store 32 pixels of BGRA.
                "vst1.8      {q2, q3}, [%1]!             \n"
                "bgt        0b                                   \n"
                : "+r"(src_bgra),              // %0
                "+r"(dst_rgba),                // %1
                "+r"(nn)                      // %2
                :"r"(shuffle)
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7");
#endif
        }

        if (remain) {
            const unsigned char *src_ptr0 = src_ptr + paralle_size;
            unsigned char *dst_ptr0 = dst_ptr + paralle_size;
            for (int j = 0; j < remain; j++) {
                unsigned char b00 = src_ptr0[0];
                unsigned char g00 = src_ptr0[1];
                unsigned char r00 = src_ptr0[2];
                unsigned char a00 = src_ptr0[3];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;
                dst_ptr0[3] = a00;

                src_ptr0 += 4;
                dst_ptr0 += 4;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_gray_to_bgr_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w)
            && (dst_stride == dst.width() * 3)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int count = src_w & (~15);

    unsigned char left_src = 0;
    uint8x16x3_t v_dst0_u8;

    int i = 0, j = 0;
    for (; i < src_h; i++) {
        const unsigned char *src_gray = src_ptr;
        unsigned char *dst_bgr = dst_ptr;

        for (j = 0; j < count; j += 16) {
            v_dst0_u8.val[0] = vld1q_u8(src_gray);

            v_dst0_u8.val[1] = v_dst0_u8.val[0];
            v_dst0_u8.val[2] = v_dst0_u8.val[0];

            vst3q_u8(dst_bgr, v_dst0_u8);
            src_gray += 16;
            dst_bgr += 48;
        }

        for (; j < src_w; j++) {
            left_src = *src_gray;
            *(dst_bgr    ) = left_src;
            *(dst_bgr + 1) = left_src;
            *(dst_bgr + 2) = left_src;

            src_gray++;
            dst_bgr += 3;
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_gray_to_bgra_neon(const Mat& src, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w)
            && (dst_stride == dst.width() << 2)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    const int width_align8 = src_w & (~7);

    uint8x8x4_t v_dst0_u8;
    v_dst0_u8.val[3] = vdup_n_u8(255);
    unsigned char left_src = 0;

    int i = 0;
    int j = 0;
    for (; i < src_h; i++) {
        const unsigned char *src0_ptr = src_ptr;
        unsigned char *dst0_ptr = dst_ptr;

        for (j = 0; j < width_align8; j += 8) {
            v_dst0_u8.val[0] = vld1_u8(src0_ptr);
            v_dst0_u8.val[1] = v_dst0_u8.val[0];
            v_dst0_u8.val[2] = v_dst0_u8.val[0];

            vst4_u8(dst0_ptr, v_dst0_u8);
            src0_ptr += 8;
            dst0_ptr += 32;
        }

        for (; j < src_w; j++) {
            left_src = *src0_ptr;
            *(dst0_ptr    ) = left_src;
            *(dst0_ptr + 1) = left_src;
            *(dst0_ptr + 2) = left_src;
            *(dst0_ptr + 3) = 255;

            src0_ptr++;
            dst0_ptr += 4;
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

#if __aarch64__
#define INTC3                                        \
    "ld3 {v1.16b, v2.16b, v3.16b}, [%0], #48         \n" /* q0       q1       q2       */ \
    "subs        %w2, %w2, #1                        \n" /* bbbbBBBB ggggGGGG rrrrRRRR */ \
    "prfm  pldl1keep, [%0, #448]                     \n" \

#define INTC4                                        \
    "prfm  pldl1keep, [%0, 448]                     \n" \
    "ld4 {v1.16b, v2.16b, v3.16b, v4.16b}, [%0], #64 \n" /* q0       q1       q2       */ \
    "subs        %w2, %w2, #1                        \n" /* bbbbBBBB ggggGGGG rrrrRRRR */ \

#define BGR2BGR565(bb, gg, rr)                       \
    "sri  " #rr ".16b, " #gg ".16b, #5               \n" /* rrrrRggg                   */ \
    "shl  " #gg ".16b, " #gg ".16b, #3               \n" /* gGGGG000                   */ \
    "sri  " #gg ".16b, " #bb ".16b, #3               \n" /* gGGbbbB                    */ \
    "st2 {v2.16b, v3.16b}, [%1], #32                 \n" /* gGGbbbB rrrrRggg           */ \

#define RGB2BGR565                                   \
    "sri  v1.16b, v2.16b, #5                         \n" /* rrrrRggg                   */ \
    "shl  v0.16b, v2.16b, #3                         \n" /* gGGGG000                   */ \
    "sri  v0.16b, v3.16b, #3                         \n" /* gGGbbbB                    */ \
    "st2 {v0.16b, v1.16b}, [%1], #32                 \n" /* gGGbbbB rrrrRggg           */ \

#else
#define INTC3                                        \
    "pld [%0, #448]                                  \n" \
    "vld3.8 {d2, d4, d6}, [%0]!                      \n" /* q0       q1       q2       */ \
    "vld3.8 {d3, d5, d7}, [%0]!                      \n" /* bbbbBBBB ggggGGGG rrrrRRRR */ \
    "subs        %2, %2, #1                          \n"                                  \

#define INTC4                                        \
    "pld [%0, #448]                                  \n" \
    "vld4.8 {d2, d4, d6, d8}, [%0]!                  \n" /* q0       q1       q2       */ \
    "vld4.8 {d3, d5, d7, d9}, [%0]!                  \n" /* bbbbBBBB ggggGGGG rrrrRRRR */ \
    "subs        %2, %2, #1                          \n"                                  \

#define BGR2BGR565(bb, gg, rr)                       \
    "vsri.8  " #rr ", " #gg ", #5                    \n" /* rrrrRggg                   */ \
    "vshl.u8 " #gg ", #3                             \n" /* gGGGG000                   */ \
    "vsri.8  " #gg ", " #bb ", #3                    \n" /* gGGbbbB                    */ \
    "vst2.8 {d4, d6}, [%1]!                          \n" /* gGGbbbB rrrrRggg           */ \
    "vst2.8 {d5, d7}, [%1]!                          \n" \

#define RGB2BGR565                                   \
    "vsri.8  q1, q2, #5                              \n" /* rrrrRggg                   */ \
    "vshl.u8 q0, q2, #3                              \n" /* gGGGG000                   */ \
    "vsri.8  q0, q3, #3                              \n" /* gGGbbbB                    */ \
    "vst2.8 {d0, d2}, [%1]!                          \n" /* gGGbbbB rrrrRggg           */ \
    "vst2.8 {d1, d3}, [%1]!                          \n" \

#endif

#define CONVERT_TO_BGR565_PARAM                                           \
    int src_w = src.width();                                              \
    int src_h = src.height();                                             \
    int src_stride = src.stride();                                        \
    int dst_stride = dst.stride();                                        \
    const unsigned char *src_ptr = (const unsigned char *)src.data();     \
    unsigned char *dst_ptr = (unsigned char *)dst.data();

void convert_gray_to_bgr565_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    int i = 0;
    int j = 0;
    for (i = 0; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

       if (cnt > 0) {
#if __aarch64__
        asm volatile(
            "0:                                    \n"
            "ld1 {v1.16b}, [%0], #16               \n"
            "mov v2.16b, v1.16b                    \n"
            "subs      %w2, %w2, #1                \n"
            "sri  v1.16b, v1.16b, #5               \n"
            "shl  v0.16b, v2.16b, #3               \n"
            "sri  v0.16b, v2.16b, #3               \n"
            "st2 {v0.16b, v1.16b}, [%1], #32       \n"
            "bgt        0b                         \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "v0", "v1", "v2");

#else
        asm volatile(
            "0:                                    \n"
            "vld1.8 {d2-d3}, [%0]!                 \n"
            "vmov.16 q2, q1                        \n"
            "subs        %2, %2, #1                \n"
            "vsri.8  q1, q1, #5                    \n"
            "vshl.u8 q0, q2, #3                    \n"
            "vsri.8  q0, q2, #3                    \n"
            "vst2.8 {d0, d2}, [%1]!                \n"
            "vst2.8 {d1, d3}, [%1]!                \n"
            "bgt        0b                         \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "q0", "q1", "q2");
#endif
       }

       if (remain) {
            const unsigned char *in_remain = src_ptr + src_w_align16;
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 1);
            for (j = 0; j < remain; j++) {
                convertTo565(in_remain[0], in_remain[0], in_remain[0], out_remain);
                in_remain += 1;
                out_remain += 2;
            }
       }

       src_ptr += src_stride;
       dst_ptr += dst_stride;
    }
}

void convert_bgr_to_bgr565_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 3)
            && (dst_stride == dst.width() << 1)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    int i = 0, j = 0;
    for (; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

       if (cnt > 0) {
#if __aarch64__
        asm volatile(
            "0:                                                \n"
            INTC3
            BGR2BGR565(v1, v2, v3)
            "bgt        0b                                     \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "v1", "v2", "v3");

#else
        asm volatile(
            "0:                                                \n"
            INTC3
            BGR2BGR565(q1, q2, q3)
            "bgt        0b                                     \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "q1", "q2", "q3");
#endif
       }

       if (remain) {
           const unsigned char *in_remain = src_ptr + src_w_align16 * 3;
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 1);
            for (j = 0; j < remain; j++) {
                convertTo565(in_remain[0], in_remain[1], in_remain[2], out_remain);
                in_remain += 3;
                out_remain += 2;
            }
       }

       src_ptr += src_stride;
       dst_ptr += dst_stride;
    }
}

void convert_rgb_to_bgr565_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 3)
            && (dst_stride == dst.width() << 1)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    int i = 0, j = 0;
    for (i = 0; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

       if (cnt > 0) {
#if __aarch64__
        asm volatile(
            "0:                                                \n"
            INTC3
            RGB2BGR565
            "bgt        0b                                     \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "v1", "v2", "v3");

#else
        asm volatile(
            "0:                                                \n"
            INTC3
            RGB2BGR565
            "bgt        0b                                     \n"
            : "+r"(in_ptr),              // %0
            "+r"(out_ptr),               // %1
            "+r"(cnt)                    // %2
            :
        : "cc", "memory", "q1", "q2", "q3");
#endif
       }

       if (remain) {
           const unsigned char *in_remain = src_ptr + src_w_align16 * 3;
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 1);
            for (j = 0; j < remain; j++) {
                convertTo565(in_remain[2], in_remain[1], in_remain[0], out_remain);
                in_remain += 3;
                out_remain += 2;
            }
       }

       src_ptr += src_stride;
       dst_ptr += dst_stride;
    }
}

void convert_bgra_to_bgr565_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 4)
            && (dst_stride == dst.width() << 1)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    int i = 0, j = 0;
    for (i = 0; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

        if (cnt > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                \n"
                INTC4
                BGR2BGR565(v1, v2, v3)
                "bgt        0b                                     \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4");

#else
            asm volatile(
                "0:                                                \n"
                INTC4
                BGR2BGR565(q1, q2, q3)
                "bgt        0b                                     \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4");
#endif
        }

        if (remain) {
            const unsigned char *in_remain = src_ptr + (src_w_align16 << 2);
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 1);
            for (j = 0; j < remain; j++) {
                convertTo565(in_remain[0], in_remain[1], in_remain[2], out_remain);
                in_remain += 4;
                out_remain += 2;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_rgba_to_bgr565_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    //Coalesce rows.
    if ((src_w == dst.width())
            && (src_stride == src_w * 4)
            && (dst_stride == dst.width() << 1)) {
        src_w *= src_h;
        src_h = 1;
        src_stride = dst_stride = 0;
    }

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    int i = 0, j = 0;
    for (i = 0; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

        if (cnt > 0) {
#if __aarch64__
            asm volatile(
                "0:                                                \n"
                INTC4
                RGB2BGR565
                "bgt        0b                                     \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                :
            : "cc", "memory", "v0", "v1", "v2", "v3", "v4");
#else
            asm volatile(
                "0:                                                \n"
                INTC4
                RGB2BGR565
                "bgt        0b                                     \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                :
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4");
#endif
        }

        if (remain) {
            const unsigned char *in_remain = src_ptr + (src_w_align16 << 2) ;
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 1);
            for (j = 0; j < remain; j++) {
                convertTo565(in_remain[2], in_remain[1], in_remain[0], out_remain);
                in_remain  += 4;
                out_remain += 2;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
    }
}

void convert_rgba_to_mrgba_neon(const Mat& src, Mat& dst) {
    CONVERT_TO_BGR565_PARAM

    int src_w_align16 = src_w & (~15);
    int nn = src_w_align16 >> 4;
    int remain = src_w - src_w_align16;

    const float max_val  = 1.0 / 255.0;
    const unsigned char half_val = 128;

    int i = 0;
    int j = 0;
    for (i = 0; i < src_h; i++) {
        const unsigned char *in_ptr = src_ptr;
        unsigned char *out_ptr = dst_ptr;
        int cnt = nn;

        if (cnt > 0) {
#if __aarch64__
            asm volatile(
                "movi   v4.8h, #128                                   \n"
                "dup    v6.4s, %w[max_val]                            \n"
                "0:                                                   \n"
                "ld4   {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64    \n"
                "subs        %w2, %w2, #1                             \n"
                "umull    v7.8h, v0.8b,  v3.8b                        \n"
                "prfm  pldl1keep, [%0, #448]                          \n"
                "uaddl    v20.4s, v4.4h, v7.4h                        \n"
                "umull2   v8.8h, v0.16b, v3.16b                       \n"
                "uaddl2   v21.4s, v4.8h, v7.8h                        \n"
                "ucvtf    v20.4s, v20.4s                              \n"
                "uaddl    v22.4s, v4.4h, v8.4h                        \n"
                "ucvtf    v21.4s, v21.4s                              \n"
                "uaddl2   v23.4s, v4.8h, v8.8h                        \n"
                "ucvtf    v22.4s, v22.4s                              \n"
                "umull    v9.8h,  v1.8b, v3.8b                        \n"
                "uaddl    v24.4s, v4.4h, v9.4h                        \n"
                "umull2   v10.8h, v1.16b, v3.16b                      \n"
                "ucvtf    v23.4s, v23.4s                              \n"
                "umull    v11.8h, v2.8b, v3.8b                        \n"
                "uaddl2   v25.4s, v4.8h, v9.8h                        \n"
                "umull2   v12.8h, v2.16b, v3.16b                      \n"
                "ucvtf    v24.4s, v24.4s                              \n"
                "uaddl    v26.4s, v4.4h, v10.4h                       \n"
                "ucvtf    v25.4s, v25.4s                              \n"
                "uaddl2   v27.4s, v4.8h, v10.8h                       \n"
                "ucvtf    v26.4s, v26.4s                              \n"
                "uaddl    v28.4s, v4.4h, v11.4h                       \n"
                "ucvtf    v27.4s, v27.4s                              \n"
                "uaddl2   v29.4s, v4.8h, v11.8h                       \n"
                "ucvtf    v28.4s, v28.4s                              \n"
                "uaddl    v30.4s, v4.4h, v12.4h                       \n"
                "ucvtf    v29.4s, v29.4s                              \n"
                "uaddl2   v31.4s, v4.8h, v12.8h                       \n"
                "ucvtf    v30.4s, v30.4s                              \n"
                "ucvtf    v31.4s, v31.4s                              \n"
                "fmul     v20.4s, v20.4s, v6.4s                       \n"
                "fmul     v21.4s, v21.4s, v6.4s                       \n"
                /* f32 -> int32 */                     \
                "fcvtzu   v20.4s, v20.4s                              \n"
                "fmul     v22.4s, v22.4s, v6.4s                       \n"
                "fcvtzu   v21.4s, v21.4s                              \n"
                "fmul     v23.4s, v23.4s, v6.4s                       \n"
                "fcvtzu   v22.4s, v22.4s                              \n"
                "fmul     v24.4s, v24.4s, v6.4s                       \n"
                "fcvtzu   v23.4s, v23.4s                              \n"
                "fmul     v25.4s, v25.4s, v6.4s                       \n"
                "fcvtzu   v24.4s, v24.4s                              \n"
                "fcvtzu   v25.4s, v25.4s                              \n"
                "uqxtn    v20.4h,  v20.4s                             \n"
                "uqxtn2   v20.8h,  v21.4s                             \n"
                "fmul     v26.4s, v26.4s, v6.4s                       \n"
                "uqxtn    v22.4h,  v22.4s                             \n"
                "fmul     v27.4s, v27.4s, v6.4s                       \n"
                "uqxtn2   v22.8h,  v23.4s                             \n"
                "fmul     v28.4s, v28.4s, v6.4s                       \n"
                "uqxtn    v24.4h,  v24.4s                             \n"
                "fmul     v29.4s, v29.4s, v6.4s                       \n"
                "uqxtn2   v24.8h,  v25.4s                             \n"
                "fmul     v30.4s, v30.4s, v6.4s                       \n"
                "fmul     v31.4s, v31.4s, v6.4s                       \n"
                "fcvtzu   v26.4s, v26.4s                              \n"
                "uqxtn    v26.4h,  v26.4s                             \n"
                "fcvtzu   v27.4s, v27.4s                              \n"
                "uqxtn2   v26.8h,  v27.4s                             \n"
                "fcvtzu   v28.4s, v28.4s                              \n"
                "uqxtn    v28.4h,  v28.4s                             \n"
                "fcvtzu   v29.4s, v29.4s                              \n"
                "fcvtzu   v30.4s, v30.4s                              \n"
                "fcvtzu   v31.4s, v31.4s                              \n"
                "uqxtn2   v28.8h,  v29.4s                             \n"
                "uqxtn    v30.4h,  v30.4s                             \n"
                "uqxtn2   v30.8h,  v31.4s                             \n"
                "uqxtn    v0.8b,   v20.8h                             \n"
                "uqxtn2   v0.16b,  v22.8h                             \n"
                "uqxtn    v1.8b,   v24.8h                             \n"
                "uqxtn2   v1.16b,  v26.8h                             \n"
                "uqxtn    v2.8b,   v28.8h                             \n"
                "uqxtn2   v2.16b,  v30.8h                             \n"
                "st4 {v0.16b, v1.16b, v2.16b, v3.16b}, [%1], #64      \n"
                "bgt        0b                         \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                : [max_val]"r"(max_val)
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", 
                    "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",
                    "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",
                     "v29", "v30", "v31");
#else
            asm volatile(
                "vmov.u16    d10, #128                 \n"
                "vdup.s32    q6, %[max_val]            \n"
                "0:                                    \n"
                "pld [%0, #448]                        \n"
                "vld4.8 {d0, d2, d4, d6}, [%0]!        \n"
                "subs        %2, %2, #1                \n"
                "vmull.u8    q7, d0, d6                \n"
                "vmull.u8    q8, d2, d6                \n"
                "vmull.u8    q9, d4, d6                \n"
                "vaddl.u16   q10, d10, d14             \n"
                "vaddl.u16   q11, d10, d15             \n"
                "vaddl.u16   q12, d10, d16             \n"
                "vaddl.u16   q13, d10, d17             \n"
                "vaddl.u16   q14, d10, d18             \n"
                "vaddl.u16   q15, d10, d19             \n"
                "vcvt.f32.u32 q10, q10                 \n"
                "vcvt.f32.u32 q11, q11                 \n"
                "vcvt.f32.u32 q12, q12                 \n"
                "vcvt.f32.u32 q13, q13                 \n"
                "vcvt.f32.u32 q14, q14                 \n"
                "vcvt.f32.u32 q15, q15                 \n"
                "vmul.f32     q10, q10, q6             \n"
                "vmul.f32     q11, q11, q6             \n"
                "vmul.f32     q12, q12, q6             \n"
                "vmul.f32     q13, q13, q6             \n"
                "vmul.f32     q14, q14, q6             \n"
                "vmul.f32     q15, q15, q6             \n"
                /* f32 -> int32 */                     \
                "vcvt.u32.f32 q10, q10                 \n"
                "vcvt.u32.f32 q11, q11                 \n"
                "vcvt.u32.f32 q12, q12                 \n"
                "vcvt.u32.f32 q13, q13                 \n"
                "vcvt.u32.f32 q14, q14                 \n"
                "vcvt.u32.f32 q15, q15                 \n"
                "vqmovn.u32 d20, q10                   \n"
                "vqmovn.u32 d21, q11                   \n"
                "vqmovn.u32 d22, q12                   \n"
                "vqmovn.u32 d23, q13                   \n"
                "vqmovn.u32 d24, q14                   \n"
                "vqmovn.u32 d25, q15                   \n"
                "vqmovn.u16 d0, q10                    \n"
                "vqmovn.u16 d2, q11                    \n"
                "vqmovn.u16 d4, q12                    \n"
                "vld4.8 {d1, d3, d5, d7}, [%0]!        \n"
                "vmull.u8    q7, d1, d7                \n"
                "vmull.u8    q8, d3, d7                \n"
                "vmull.u8    q9, d5, d7                \n"
                "vaddl.u16   q10, d10, d14             \n"
                "vaddl.u16   q11, d10, d15             \n"
                "vaddl.u16   q12, d10, d16             \n"
                "vaddl.u16   q13, d10, d17             \n"
                "vaddl.u16   q14, d10, d18             \n"
                "vaddl.u16   q15, d10, d19             \n"
                "vcvt.f32.u32 q10, q10                 \n"
                "vcvt.f32.u32 q11, q11                 \n"
                "vcvt.f32.u32 q12, q12                 \n"
                "vcvt.f32.u32 q13, q13                 \n"
                "vcvt.f32.u32 q14, q14                 \n"
                "vcvt.f32.u32 q15, q15                 \n"
                "vmul.f32     q10, q10, q6             \n"
                "vmul.f32     q11, q11, q6             \n"
                "vmul.f32     q12, q12, q6             \n"
                "vmul.f32     q13, q13, q6             \n"
                "vmul.f32     q14, q14, q6             \n"
                "vmul.f32     q15, q15, q6             \n"
                /* f32 -> int32 */                     \
                "vcvt.u32.f32 q10, q10                 \n"
                "vcvt.u32.f32 q11, q11                 \n"
                "vcvt.u32.f32 q12, q12                 \n"
                "vcvt.u32.f32 q13, q13                 \n"
                "vcvt.u32.f32 q14, q14                 \n"
                "vcvt.u32.f32 q15, q15                 \n"
                "vqmovn.u32 d20, q10                   \n"
                "vqmovn.u32 d21, q11                   \n"
                "vqmovn.u32 d22, q12                   \n"
                "vqmovn.u32 d23, q13                   \n"
                "vqmovn.u32 d24, q14                   \n"
                "vqmovn.u32 d25, q15                   \n"
                "vqmovn.u16 d1, q10                    \n"
                "vqmovn.u16 d3, q11                    \n"
                "vqmovn.u16 d5, q12                    \n"
                "vst4.8 {d0, d2, d4, d6}, [%1]!        \n"
                "vst4.8 {d1, d3, d5, d7}, [%1]!        \n"
                "bgt        0b                         \n"
                : "+r"(in_ptr),              // %0
                "+r"(out_ptr),               // %1
                "+r"(cnt)                    // %2
                : [max_val]"r"(max_val)
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", 
                    "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#endif
        }

        if (remain) {
            const unsigned char *in_remain = src_ptr + (src_w_align16 << 2);
            unsigned char *out_remain = dst_ptr + (src_w_align16 << 2);
            for (j = 0; j < remain; j++) {
                unsigned char v0 = in_remain[0];
                unsigned char v1 = in_remain[1];
                unsigned char v2 = in_remain[2];
                unsigned char v3 = in_remain[3];

                out_remain[0] = (v0 * v3 + half_val) / 255;
                out_remain[1] = (v1 * v3 + half_val) / 255;
                out_remain[2] = (v2 * v3 + half_val) / 255;
                out_remain[3] = v3;

                in_remain += 4;
                out_remain += 4;
            }
       }

       src_ptr += src_stride;
       dst_ptr += dst_stride;
    }
}

//bgrbgr..... to bbbb..ggg...rrr...
void convert_package_to_planer_neon(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int channel = src.channels();
    int cnt = src_h * src_w;
    int nn = cnt >> 4;
    int remain = cnt - (nn << 4);

    if (channel == 3) {
        // the first address of each channel
        unsigned char *dstb_ptr = dst_ptr;
        unsigned char *dstg_ptr = dstb_ptr + cnt;
        unsigned char *dstr_ptr = dstg_ptr + cnt;

#if __aarch64__
        asm volatile(
            "0:                                                \n"
            "prfm  pldl1keep, [%0, #128]                       \n"

            "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n" // load 16 pixels of BGR.
            "subs        %w4, %w4, #1                          \n"  // 16 processed per loop

            "st1 {v0.16b}, [%1], #16                          \n"
            "st1 {v1.16b}, [%2], #16                          \n"
            "st1 {v2.16b}, [%3], #16                          \n"
            "b.gt        0b                                   \n"
            : "+r"(src_ptr),               // %0
            "+r"(dstb_ptr),                // %1
            "+r"(dstg_ptr),                // %2
            "+r"(dstr_ptr),                // %3
            "+r"(nn)                       // %4
            :
        : "cc", "memory", "v0", "v1", "v2");
#else
            asm volatile(
            "0:                                              \n"
            "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
            "vld3.8      {d1, d3, d5}, [%0]!                 \n"  // next 8 RGB
            "subs         %4, %4, #1                         \n"

            "vst1.8 {q0}, [%1]!                              \n" // store 16 pixels of B
            "vst1.8 {q1}, [%2]!                              \n" // store 16 pixels of G
            "vst1.8 {q2}, [%3]!                              \n" // store 16 pixels of R
            "bgt        0b                                   \n"

             : "+r"(src_ptr),                // %0
            "+r"(dstb_ptr),                 // %1
            "+r"(dstg_ptr),                 // %2
            "+r"(dstr_ptr),                 // %3
            "+r"(nn)                        // %4
            :
        : "cc", "memory", "q0", "q1", "q2");
#endif

        for (int i = 0; i < remain; ++i) {
            *(dstb_ptr++) = *(src_ptr++);
            *(dstg_ptr++) = *(src_ptr++);
            *(dstr_ptr++) = *(src_ptr++);
        }
    } else if (channel == 4) {
         // the first address of each channel
        unsigned char *dstb_ptr = dst_ptr;
        unsigned char *dstg_ptr = dstb_ptr + cnt;
        unsigned char *dstr_ptr = dstg_ptr + cnt;
        unsigned char *dsta_ptr = dstr_ptr + cnt;

#if __aarch64__
        asm volatile(
            "0:                                                 \n"
            "prfm  pldl1keep, [%0, #128]                        \n"

            "ld4 {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64   \n" // load 16 pixels of BGR.
            "subs        %w5, %w5, #1                          \n"  // 16 processed per loop

            "st1 {v0.16b}, [%1], #16                          \n"
            "st1 {v1.16b}, [%2], #16                          \n"
            "st1 {v2.16b}, [%3], #16                          \n"
            "st1 {v3.16b}, [%4], #16                          \n"
            "b.gt        0b                                   \n"
            : "+r"(src_ptr),                // %0
            "+r"(dstb_ptr),                 // %1
            "+r"(dstg_ptr),                 // %2
            "+r"(dstr_ptr),                 // %3
            "+r"(dsta_ptr),                 // %4
            "+r"(nn)                        // %5
            :
        : "cc", "memory", "v0", "v1", "v2", "v3");
#else
            asm volatile(
            "0:                                                \n"
            "vld4.8      {d0, d2, d4, d6}, [%0]!               \n"  // load 8 pixels of BGR.
            "vld4.8      {d1, d3, d5, d7}, [%0]!               \n"  // load next 8 pixels of BGR .

            "subs         %5, %5, #1                          \n"
            "vst1.8 {q0}, [%1]!                               \n" // atore 16 pixels of B
            "vst1.8 {q1}, [%2]!                               \n" // atore 16 pixels of G
            "vst1.8 {q2}, [%3]!                               \n" // atore 16 pixels of R
            "vst1.8 {q3}, [%4]!                               \n" // atore 16 pixels of A
            "bgt        0b                                    \n"

             : "+r"(src_ptr),                // %0
            "+r"(dstb_ptr),                 // %1
            "+r"(dstg_ptr),                 // %2
            "+r"(dstr_ptr),                 // %3
            "+r"(dsta_ptr),                 // %4
            "+r"(nn)                        // %5
            :
        : "cc", "memory", "q0", "q1", "q2", "q3");
#endif

        for (int i = 0; i < remain; ++i) {
            *(dstb_ptr++) = *(src_ptr++);
            *(dstg_ptr++) = *(src_ptr++);
            *(dstr_ptr++) = *(src_ptr++);
            *(dsta_ptr++) = *(src_ptr++);
        }
    } else {
        LOG_ERR("the channel of planer convert to package not supported!\n");
    }
}

//bbb...ggg...rrr... convert bgrbgr...
void convert_planer_to_package_neon(const Mat& src, Mat& dst){
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int channel = src.channels();
    int cnt = src_h * src_w;
    int nn = cnt >> 4;
    int remain = cnt - (nn << 4);

    if (channel == 3) {
        // the first address of each channel
        const unsigned char *srcb_ptr = src_ptr;
        const unsigned char *srcg_ptr = srcb_ptr + cnt;
        const unsigned char *srcr_ptr = srcg_ptr + cnt;

#if __aarch64__
        asm volatile(
            "0:                                                \n"
            "prfm  pldl1keep, [%0, #128]                       \n"
            "prfm  pldl1keep, [%1, #128]                       \n"
            "prfm  pldl1keep, [%2, #128]                       \n"

            "ld1 {v0.16b}, [%0], #16                           \n" // load 16 pixels of B.
            "ld1 {v1.16b}, [%1], #16                           \n" // load 16 pixels of G.
            "ld1 {v2.16b}, [%2], #16                           \n" // load 16 pixels of R.
            "subs        %w4, %w4, #1                          \n"  // 16 processed per loop

            "st3 {v0.16b, v1.16b, v2.16b}, [%3], #48           \n" // store 48 pixels of BGR.
            "b.gt        0b                                    \n"
            : "+r"(srcb_ptr),               // %0
            "+r"(srcg_ptr),                 // %1
            "+r"(srcr_ptr),                 // %2
            "+r"(dst_ptr),                  // %3
            "+r"(nn)                        // %4
            :
        : "cc", "memory", "v0", "v1", "v2");
#else
        asm volatile(
            "0:                                                \n"
            "vld1.8 {q0}, [%0]!                                \n" // load 16 pixels of B.
            "vld1.8 {q1}, [%1]!                                \n" // load 16 pixels of G.
            "vld1.8 {q2}, [%2]!                                \n" // load 16 pixels of R.
            "subs        %4, %4, #1                            \n"  // 16 processed per loop

            "vst3.8 {d0, d2, d4}, [%3]!                        \n" // store 24 pixels of BGR.
            "vst3.8 {d1, d3, d5}, [%3]!                        \n" // store next 24 pixels of BGR.
            "bgt        0b                                     \n"
            : "+r"(srcb_ptr),              // %0
            "+r"(srcg_ptr),                // %1
            "+r"(srcr_ptr),                // %2
            "+r"(dst_ptr),                 // %3
            "+r"(nn)                       // %4
            :
        : "cc", "memory", "q0", "q1", "q2");
#endif

        for (int i = 0; i < remain; ++i) {
            *(dst_ptr++) = *(srcb_ptr++);
            *(dst_ptr++) = *(srcg_ptr++);
            *(dst_ptr++) = *(srcr_ptr++);
        }
    } else if (channel == 4) {
        // the first address of each channel
        const unsigned char *srcb_ptr = src_ptr;
        const unsigned char *srcg_ptr = srcb_ptr + cnt;
        const unsigned char *srcr_ptr = srcg_ptr + cnt;
        const unsigned char *srca_ptr = srcr_ptr + cnt;
#if __aarch64__
        asm volatile(
            "0:                                                \n"
            "prfm  pldl1keep, [%0, #128]                       \n"
            "prfm  pldl1keep, [%1, #128]                       \n"
            "prfm  pldl1keep, [%2, #128]                       \n"
            "prfm  pldl1keep, [%3, #128]                       \n"

            "ld1 {v0.16b}, [%0], #16                           \n" // load 16 pixels of B.
            "ld1 {v1.16b}, [%1], #16                           \n" // load 16 pixels of G.
            "ld1 {v2.16b}, [%2], #16                           \n" // load 16 pixels of R.
            "ld1 {v3.16b}, [%3], #16                           \n" // load 16 pixels of A.
            "subs        %w5, %w5, #1                          \n"  // 16 processed per loop

            "st4 {v0.16b, v1.16b, v2.16b, v3.16b}, [%4], #64   \n" // store 48 pixels of BGR.
            "b.gt        0b                                    \n"
            : "+r"(srcb_ptr),                // %0
            "+r"(srcg_ptr),                 // %1
            "+r"(srcr_ptr),                 // %2
            "+r"(srca_ptr),                 // %3
            "+r"(dst_ptr),                  // %4
            "+r"(nn)                        // %5
            :
        : "cc", "memory", "v0", "v1", "v2");
#else
        asm volatile(
            "0:                                                \n"
            "vld1.8 {q0}, [%0]!                                \n" // load 16 pixels of B.
            "vld1.8 {q1}, [%1]!                                \n" // load 16 pixels of G.
            "vld1.8 {q2}, [%2]!                                \n" // load 16 pixels of R.
            "vld1.8 {q3}, [%3]!                                \n" // load 16 pixels of R.
            "subs   %5, %5, #1                                 \n"  // 16 processed per loop

            "vst4.8 {d0, d2, d4, d6}, [%4]!                    \n" // store 48 pixels of BGR.
            "vst4.8 {d1, d3, d5, d7}, [%4]!                    \n" // store 48 pixels of BGR.
            "bgt        0b                                    \n"
            : "+r"(srcb_ptr),              // %0
            "+r"(srcg_ptr),                // %1
            "+r"(srcr_ptr),                // %2
            "+r"(srca_ptr),                // %3
            "+r"(dst_ptr),                 // %4
            "+r"(nn)                       // %5
            :
        : "cc", "memory", "q0", "q1", "q2", "q3");
#endif

        for (int i = 0; i < remain; ++i) {
            *(dst_ptr++) = *(srcb_ptr++);
            *(dst_ptr++) = *(srcg_ptr++);
            *(dst_ptr++) = *(srcr_ptr++);
            *(dst_ptr++) = *(srca_ptr++);
        }
    } else {
        LOG_ERR("the channel of planer convert to package not supported!");
    }
}

int cvt_color_neon(
        const Mat& src,
        Mat& dst,
        ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_NV212PA_BGR:
        convert_nv21_to_bgr_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_BGR:
        convert_nv12_to_bgr_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_RGB:
        convert_nv21_to_rgb_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_RGB:
        convert_nv12_to_rgb_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_BGRA:
        convert_nv21_to_bgra_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_BGRA:
        convert_nv12_to_bgra_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_RGBA:
        convert_nv21_to_rgba_neon(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_RGBA:
        convert_nv12_to_rgba_neon(src, dst);
        break;
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2NV12:
    case ColorConvertType::CVT_PA_BGR2NV21:
    case ColorConvertType::CVT_PA_RGB2NV12:
    case ColorConvertType::CVT_PA_RGB2NV21:
    case ColorConvertType::CVT_PA_BGRA2NV12:
    case ColorConvertType::CVT_PA_BGRA2NV21:
    case ColorConvertType::CVT_PA_RGBA2NV12:
    case ColorConvertType::CVT_PA_RGBA2NV21:
        convert_bgr_to_yuv420_neon(src, dst, cvt_type);
        break;

    case ColorConvertType::CVT_PA_BGR2GRAY:
    case ColorConvertType::CVT_PA_RGB2GRAY:
        convert_bgr_to_gray_neon(src, dst, cvt_type);
        break;

    //cvt from bgr/rgb to rgb/bgr/rgba/bgra
    case ColorConvertType::CVT_PA_BGR2PA_RGB:
    case ColorConvertType::CVT_PA_RGB2PA_BGR:
        convert_bgr_to_rgb_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_BGRA:
    case ColorConvertType::CVT_PA_RGB2PA_RGBA:
        convert_bgr_to_bgra_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_RGBA:
    case ColorConvertType::CVT_PA_RGB2PA_BGRA:
        convert_bgr_to_rgba_neon(src, dst);
        break;

    //cvt from bgra/rgba to rgb/bgr/rgba/bgra
    case ColorConvertType::CVT_PA_BGRA2PA_BGR:
    case ColorConvertType::CVT_PA_RGBA2PA_RGB:
        convert_bgra_to_bgr_neon(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGRA2PA_RGB:
    case ColorConvertType::CVT_PA_RGBA2PA_BGR:
        convert_bgra_to_rgb_neon(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGBA2PA_BGRA:
    case ColorConvertType::CVT_PA_BGRA2PA_RGBA:
        convert_bgra_to_rgba_neon(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_RGB:
    case ColorConvertType::CVT_GRAY2PA_BGR:
        convert_gray_to_bgr_neon(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_RGBA:
    case ColorConvertType::CVT_GRAY2PA_BGRA:
        convert_gray_to_bgra_neon(src, dst);
        break;

    case ColorConvertType::CVT_PL_BGR2PA_BGR:
        convert_planer_to_package_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PL_BGR:
        convert_package_to_planer_neon(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_BGR565:
        convert_gray_to_bgr565_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_BGR565:
        convert_bgr_to_bgr565_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGB2PA_BGR565:
        convert_rgb_to_bgr565_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGRA2PA_BGR565:
        convert_bgra_to_bgr565_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_BGR565:
        convert_rgba_to_bgr565_neon(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_mRGBA:
        convert_rgba_to_mrgba_neon(src, dst);
        break;

    default:
        LOG_ERR("cvt type not support yet!");
        break;
    };

    return 0;
}

int cvt_color_neon(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_neon(src_y, src_u, src_v, dst);
        break;
    default:
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
