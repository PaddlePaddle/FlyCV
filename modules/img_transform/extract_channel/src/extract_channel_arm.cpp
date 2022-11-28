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

#include "modules/img_transform/extract_channel/include/extract_channel_arm.h"

#include <arm_neon.h>

#include <cstdint>
#include <iostream>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int extract_channel_neon_u8(
        const unsigned char* src,
        int width,
        int height,
        int stride,
        int channel,
        int index,
        unsigned char* dst) {
    stride /= sizeof(unsigned char);
    width = FCV_MAX(width, stride / channel);

    int cnt = width * height;
    int nn = cnt >> 4;
    int remain = cnt - (nn << 4);
    // std::cout << "remain: " << remain << std::endl;

    if (channel == 3) {
        if (nn != 0) {
            switch (index) {
            case 0:
#if __aarch64__
                asm volatile(
                    "0:                                                \n" // label 0
                    "prfm  pldl1keep, [%0, #128]                       \n"
                    "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n" // load 16 pixels of BGR.
                    "subs        %w2, %w2, #1                          \n" // 16 processed per loop
                    "st1 {v0.16b}, [%1], #16                           \n"
                    "b.gt        0b                                    \n"
                    : "+r"(src),                // %0
                    "+r"(dst),                  // %1
                    "+r"(nn)                    // %2
                    :
                    : "cc", "memory", "v0", "v1", "v2");
#else
                asm volatile(
                "0:                                                \n"
                "vld3.32      {d0, d2, d4}, [%0]!                  \n"  // load 16 u8 pixels of BGR.
                "vld3.32      {d1, d3, d5}, [%0]!                  \n"  // next 2 RGB
                "subs         %2, %2, #1                           \n"
                "vst1.8       {q0}, [%1]!                          \n" // store 16 pixels of index
                "bgt          0b                                   \n"
                : "+r"(src),                // %0
                "+r"(dst),                  // %1
                "+r"(nn)                    // %2
                :
                : "cc", "memory", "q0", "q1", "q2");
#endif
                break;
            case 1:
#if __aarch64__
                asm volatile(
                    "0:                                                \n" // label 0
                    "prfm  pldl1keep, [%0, #128]                       \n"
                    "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n" // load 16 pixels of BGR.
                    "subs        %w2, %w2, #1                          \n" // 16 processed per loop
                    "st1 {v1.16b}, [%1], #16                           \n"
                    "b.gt        0b                                    \n"
                    : "+r"(src),                // %0
                    "+r"(dst),                  // %1
                    "+r"(nn)                    // %2
                    :
                    : "cc", "memory", "v0", "v1", "v2");
#else
                asm volatile(
                "0:                                                \n"
                "vld3.32      {d0, d2, d4}, [%0]!                  \n"  // load 16 u8 pixels of BGR.
                "vld3.32      {d1, d3, d5}, [%0]!                  \n"  // next 2 RGB
                "subs         %2, %2, #1                           \n"
                "vst1.8       {q1}, [%1]!                          \n" // store 16 pixels of index
                "bgt          0b                                   \n"
                : "+r"(src),                // %0
                "+r"(dst),                  // %1
                "+r"(nn)                    // %2
                :
                : "cc", "memory", "q0", "q1", "q2");
#endif
                break;
            case 2:
#if __aarch64__
                asm volatile(
                    "0:                                                \n" // label 0
                    "prfm  pldl1keep, [%0, #128]                       \n"
                    "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n" // load 16 pixels of BGR.
                    "subs        %w2, %w2, #1                          \n" // 16 processed per loop
                    "st1 {v2.16b}, [%1], #16                           \n"
                    "b.gt        0b                                    \n"
                    : "+r"(src),                // %0
                    "+r"(dst),                  // %1
                    "+r"(nn)                    // %2
                    :
                    : "cc", "memory", "v0", "v1", "v2");
#else
                asm volatile(
                "0:                                                \n"
                "vld3.32      {d0, d2, d4}, [%0]!                  \n"  // load 16 u8 pixels of BGR.
                "vld3.32      {d1, d3, d5}, [%0]!                  \n"  // next 2 RGB
                "subs         %2, %2, #1                           \n"
                "vst1.8       {q2}, [%1]!                          \n" // store 16 pixels of index
                "bgt          0b                                   \n"
                : "+r"(src),                // %0
                "+r"(dst),                  // %1
                "+r"(nn)                    // %2
                :
                : "cc", "memory", "q0", "q1", "q2");
#endif
                break;
            default:
                break;
            }
        }

        for (int i = 0; i < remain; ++i) {
            *(dst + i) = *(src + index + i * channel);
        }
    } else {
        LOG_ERR("the channel count is not supported!\n");
        return -1;
    }
    return 0;
}

int extract_channel_neon(Mat& src, Mat& dst, int index) {
    const void* src_ptr = (const void*)src.data();
    void* dst_ptr = (void*)dst.data();

    int src_w = src.width();
    int src_h = src.height();
    int src_c = src.channels();
    int src_s = src.stride();

    switch (src.type()) {
    case FCVImageType::PKG_BGR_U8:
        return extract_channel_neon_u8(
                (const unsigned char*)src_ptr,
                src_w,
                src_h,
                src_s,
                src_c,
                index,
                (unsigned char*)dst_ptr);
        break;
    default:
        LOG_ERR("type not support yet!");
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
