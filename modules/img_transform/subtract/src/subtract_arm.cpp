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

#include "modules/img_transform/subtract/include/subtract_arm.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void subtract_neon(
        const float* src,
        const int width,
        const int height,
        const int stride,
        const int channel,
        Scalar scalar,
        float* dst) {
    int cnt_loop = (channel == 1) ? (width >> 4) : (width >> 2);
    int remain = (channel == 1) ? (width - (cnt_loop << 4)) : (width - (cnt_loop << 2));

    for (int i = 0; i < height; i++) {
        float *src0 = (float*)src;
        float *dst0 = (float*)dst;
        int nn = cnt_loop;

        if (channel == 1) {
            float scalar0 = (float)scalar[0];
#if __aarch64__
            asm volatile(
                "dup         v24.4s, %w[scalar]    \n"
                "0:                                \n"
                "prfm  pldl1keep, [%0, #64]        \n"
                "ld1 {v0.4s}, [%[in]], #16         \n"
                "subs     %w2, %w2, #1             \n"
                "ld1 {v1.4s}, [%[in]], #16         \n"
                "fsub  v0.4s, v0.4s, v24.4s        \n"
                "ld1 {v2.4s}, [%[in]], #16         \n"
                "fsub  v1.4s, v1.4s, v24.4s        \n"
                "ld1 {v3.4s}, [%[in]], #16         \n"
                "fsub  v2.4s, v2.4s, v24.4s        \n"
                "st1 {v0.4s}, [%[out]], #16        \n"
                "fsub  v3.4s, v3.4s, v24.4s        \n"
                "st1 {v1.4s}, [%[out]], #16        \n"
                "st1 {v2.4s}, [%[out]], #16        \n"
                "st1 {v3.4s}, [%[out]], #16        \n"
                "b.gt        0b                    \n"
                : [in]"+r"(src0),                  // %0
                  [out]"+r"(dst0),                 // %1
                  "+r"(nn)                         // %2
                : [scalar]"r"(scalar0)             // %3
                : "cc", "memory", "v0", "v1", "v2", "v3", "v24");
#else
            asm volatile(
                "vdup.32    q4, %3              \n"
                "0:                             \n"
                "vld1.32 {q0}, [%0]!            \n"
                "subs     %2, %2, #1           \n"
                "vld1.32 {q1}, [%0]!            \n"
                "vsub.f32  q0, q0, q4           \n"
                "vld1.32 {q2}, [%0]!            \n"
                "vsub.f32  q1, q1, q4           \n"
                "vld1.32 {q3}, [%0]!            \n"
                "vsub.f32  q2, q2, q4           \n"
                "vst1.32 {q0}, [%1]!            \n"
                "vsub.f32  q3, q3, q4           \n"
                "vst1.32 {q1}, [%1]!            \n"
                "vst1.32 {q2}, [%1]!            \n"
                "vst1.32 {q3}, [%1]!            \n"
                "bgt        0b                  \n"
                : "+r"(src0),               // %0
                "+r"(dst0),                 // %1
                "+r"(nn)                    // %2
                : "r"(scalar0)              // %3
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4");
#endif
            for (int j = 0; j < remain; ++j) {
                *(dst0++) = *(src0++) - scalar0;
            }
        } else if (channel == 3) {
            float scalar0 = (float)scalar[0];
            float scalar1 = (float)scalar[1];
            float scalar2 = (float)scalar[2];
#if __aarch64__
            asm volatile(
                "dup         v4.4s, %w3                             \n"
                "dup         v5.4s, %w4                             \n"
                "dup         v6.4s, %w5                             \n"
                "0:                                                 \n"
                "ld3 {v0.4s, v1.4s, v2.4s}, [%0], #48               \n"
                "subs     %w2, %w2, #1                              \n"
                "fsub  v0.4s, v0.4s, v4.4s                          \n"
                "fsub  v1.4s, v1.4s, v5.4s                          \n"
                "fsub  v2.4s, v2.4s, v6.4s                          \n"
                "st3 {v0.4s, v1.4s, v2.4s}, [%1], #48               \n"
                "b.gt        0b                                     \n"
                : "+r"(src0),                 // %0
                  "+r"(dst0),                 // %1
                  "+r"(nn)                    // %2
                : "r"(scalar0),               // %3
                  "r"(scalar1),               // %4
                  "r"(scalar2)                // %5
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6");
#else
            asm volatile(
                "vdup.32         q4, %3                     \n"
                "vdup.32         q5, %4                     \n"
                "vdup.32         q6, %5                     \n"
                "0:                                         \n"
                "vld3.32 {d0, d2, d4}, [%0]!                \n"
                "vld3.32 {d1, d3, d5}, [%0]!                \n"
                "subs     %2, %2, #1                       \n"
                "vsub.f32  q0, q0, q4                       \n"
                "vsub.f32  q1, q1, q5                       \n"
                "vsub.f32  q2, q2, q6                       \n"
                "vst3.32 {d0, d2, d4}, [%1]!                \n"
                "vst3.32 {d1, d3, d5}, [%1]!                \n"
                "bgt        0b                              \n"
                : "+r"(src0),                 // %0
                "+r"(dst0),                   // %1
                "+r"(nn)                      // %2
                : "r"(scalar0),               // %3
                  "r"(scalar1),               // %4
                  "r"(scalar2)                // %5
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6");
#endif

            for (int j = 0; j < remain; ++j) {
                *(dst0++) = *(src0++) - scalar0;
                *(dst0++) = *(src0++) - scalar1;
                *(dst0++) = *(src0++) - scalar2;
            }
        } else if (channel == 4) {
            float scalar0 = (float)scalar[0];
            float scalar1 = (float)scalar[1];
            float scalar2 = (float)scalar[2];
            float scalar3 = (float)scalar[3];
#if __aarch64__
            asm volatile(
                "dup         v4.4s, %w3                             \n"
                "dup         v5.4s, %w4                             \n"
                "dup         v6.4s, %w5                             \n"
                "dup         v7.4s, %w6                             \n"
                "0:                                                 \n"
                "ld4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%0], #64        \n"
                "subs     %w2, %w2, #1                              \n"
                "fsub  v0.4s, v0.4s, v4.4s                          \n"
                "fsub  v1.4s, v1.4s, v5.4s                          \n"
                "fsub  v2.4s, v2.4s, v6.4s                          \n"
                "fsub  v3.4s, v3.4s, v7.4s                          \n"
                "st4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%1], #64        \n"
                "b.gt        0b                                     \n"
                : "+r"(src0),                // %0
                  "+r"(dst0),                // %1
                  "+r"(nn)                   // %2
                : "r"(scalar0),              // %3
                  "r"(scalar1),              // %4
                  "r"(scalar2),              // %5
                  "r"(scalar3)               // %6
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7");
#else
            asm volatile(
                "vdup.32         q4, %3                     \n"
                "vdup.32         q5, %4                     \n"
                "vdup.32         q6, %5                     \n"
                "vdup.32         q7, %6                     \n"
                "0:                                         \n"
                "vld4.32 {d0, d2, d4, d6}, [%0]!            \n"
                "vld4.32 {d1, d3, d5, d7}, [%0]!            \n"
                "subs     %2, %2, #1                       \n"
                "vsub.f32  q0, q0, q4                       \n"
                "vsub.f32  q1, q1, q5                       \n"
                "vsub.f32  q2, q2, q6                       \n"
                "vsub.f32  q3, q3, q7                       \n"
                "vst4.32 {d0, d2, d4, d6}, [%1]!            \n"
                "vst4.32 {d1, d3, d5, d7}, [%1]!            \n"
                "bgt        0b                              \n"
                : "+r"(src0),              // %0
                "+r"(dst0),                // %1
                "+r"(nn)                   // %2
                : "r"(scalar0),            // %3
                  "r"(scalar1),            // %4
                  "r"(scalar2),            // %5
                  "r"(scalar3)             // %6
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7");
#endif

            for (int j = 0; j < remain; ++j) {
                *(dst0++) = *(src0++) - scalar0;
                *(dst0++) = *(src0++) - scalar1;
                *(dst0++) = *(src0++) - scalar2;
                *(dst0++) = *(src0++) - scalar3;
            }
        }

        src += stride;
        dst += stride;
    }
}

G_FCV_NAMESPACE1_END()
