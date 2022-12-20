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

#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_arm.h"
#include "modules/core/parallel/interface/parallel.h"
#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class BgrToRgbaWithMaskArmParallelTask : public ParallelTask {
public:
    BgrToRgbaWithMaskArmParallelTask(
            const unsigned char *src_ptr,
            unsigned char *dst_ptr,
            const unsigned char *mask_ptr,
            int src_stride,
            int dst_stride,
            int mask_stride,
            int width_align32,
            int loop,
            int remain) : 
            _src_ptr(src_ptr),
            _dst_ptr(dst_ptr),
            _mask_ptr(mask_ptr),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _mask_stride(mask_stride),
            _width_align32(width_align32),
            _loop(loop),
            _remain(remain) {}

    void operator()(const Range& range) const override {
        for (int i = range.start(); i < range.end(); i++) {
            /*vectorized implementation, process 32 pixels in each channel,
            which means load 96(32 * 3) pixels in three channels of src, and 32 mask pixels,
            finally store total 128(32 * 4) dst pixels of four channels;*/
            int nn = _loop;
            if (nn > 0) {
                const unsigned char *src0_ptr = _src_ptr + i * _src_stride;
                const unsigned char *mer0_ptr = _mask_ptr + i * _mask_stride;
                unsigned char *dst0_ptr = _dst_ptr + i * _dst_stride;
    #if __aarch64__
                asm volatile(
                    "0:                                                \n"
                    "prfm  pldl1keep, [%0, #128]                       \n"
                    "prfm  pldl1keep, [%1, #64]                        \n"
                    "ld3   {v0.16b, v1.16b, v2.16b}, [%0], #48         \n"  // load 16 * 3 pixels of BGR.
                    "mov   v3.16b,  v2.16b                             \n"
                    "ld1   {v6.16b}, [%1], #16                         \n"  // load 16 pixels of mask.
                    "mov   v4.16b,  v1.16b                             \n"
                    "mov   v5.16b,  v0.16b                             \n"
                    "ld3   {v0.16b, v1.16b, v2.16b}, [%0], #48         \n"  // load 16 * 3 pixels of BGR.
                    "subs        %w3, %w3, #1                          \n"  // 32 processed per loop
                    "ld1  {v10.16b}, [%1], #16                         \n"  // load 16 pixels of mask.
                    "mov   v7.16b,  v2.16b                             \n"
                    "mov   v8.16b,  v1.16b                             \n"
                    "mov   v9.16b,  v0.16b                             \n"
                    "st4 {v3.16b, v4.16b, v5.16b, v6.16b},  [%2], #64  \n"
                    "st4 {v7.16b, v8.16b, v9.16b, v10.16b}, [%2], #64  \n"
                    "b.gt        0b                                    \n"
                    : "+r"(src0_ptr),               // %0
                    "+r"(mer0_ptr),                // %1
                    "+r"(dst0_ptr),                 // %2
                    "+r"(nn)                        // %3
                    :
                : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10");
    #else
                asm volatile(
                    "0:                                              \n"
                    "pld         [%0, #128]                          \n"
                    "pld         [%1, #64]                           \n"
                    "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
                    "vld3.8      {d1, d3, d5}, [%0]!                 \n"  // next 8 RGB
                    "vmov.u8     q3,  q2                             \n"
                    "vld1.8      {q6}, [%1]!                         \n" // load 16 pixels of mask.
                    "vmov.u8     q4,  q1                             \n"
                    "vmov.u8     q5,  q0                             \n"
                    "vld3.8      {d0, d2, d4}, [%0]!                 \n"  // load 8 pixels of BGR.
                    "vld3.8      {d1, d3, d5}, [%0]!                 \n"  // next 8 RGB
                    "subs        %3, %3, #1                          \n"
                    "vmov.u8     q7,  q2                             \n"
                    "vld1.8      {q10}, [%1]!                        \n" // load 16 pixels of mask.
                    "vmov.u8     q8,  q1                             \n"
                    "vmov.u8     q9,  q0                             \n"
                    "vst4.8      {d6, d8, d10, d12},   [%2]!         \n" // store 24 pixels of BGR.
                    "vst4.8      {d7, d9, d11, d13},   [%2]!         \n"
                    "vst4.8      {d14, d16, d18, d20}, [%2]!         \n" // store 24 pixels of BGR.
                    "vst4.8      {d15, d17, d19, d21}, [%2]!         \n"
                    "bgt        0b                                   \n"
                    : "+r"(src0_ptr),               // %0
                    "+r"(mer0_ptr),                 // %1
                    "+r"(dst0_ptr),                 // %2
                    "+r"(nn)                        // %3
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10");
    #endif
            }

            if (_remain) {
                const unsigned char *srcl_ptr = _src_ptr + _width_align32 * 3;
                const unsigned char *merl_ptr = _mask_ptr + _width_align32;
                unsigned char *dstl_ptr = _dst_ptr + (_width_align32 << 2);

                for (int j = 0; j < _remain; ++j) {

                    dstl_ptr[0] = srcl_ptr[2];
                    dstl_ptr[1] = srcl_ptr[1];
                    dstl_ptr[2] = srcl_ptr[0];
                    dstl_ptr[3] = merl_ptr[0];

                    srcl_ptr += 3;
                    dstl_ptr += 4;
                    merl_ptr += 1;
                }
            }
        }
    }
private:
    const unsigned char *_src_ptr;
    unsigned char *_dst_ptr;
    const unsigned char *_mask_ptr;
    int _src_stride;
    int _dst_stride; 
    int _mask_stride;
    int _width_align32;
    int _loop;
    int _remain;
};

int bgr_to_rgba_with_mask_neon(Mat& src, Mat& mask, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const unsigned char *mask_ptr = (const unsigned char *)mask.data();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int mask_stride = mask.stride();

    int width_align32 = src_w & (~31);
    int loop = width_align32 >> 5;
    int remain = src_w & (31); // calculate the remainder

    BgrToRgbaWithMaskArmParallelTask task(
            src_ptr,
            dst_ptr,
            mask_ptr,
            src_stride,
            dst_stride,
            mask_stride,
            width_align32,
            loop,
            remain);

    parallel_run(Range(0, src_h), task);
    return 0;
}

G_FCV_NAMESPACE1_END()