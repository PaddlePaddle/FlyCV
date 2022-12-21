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

#include "modules/img_transform/flip/include/flip_arm.h"
#include "modules/core/parallel/interface/parallel.h"
#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/*
 1 2 3
 4 5 6
 7 8 9
---------- flip_x
 7 8 9
 4 5 6
 1 2 3
*/
class FlipXTask : public ParallelTask {
public:
    FlipXTask(const unsigned char * src,
            int src_h,
            int src_w,
            int sc,
            int sstep,
            unsigned char * dst,
            int dstep) :
            _src(src),
            _src_h(src_h),
            _src_w(src_w),
            _sc(sc),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){} 

    void operator() (const Range & range) const {
        const unsigned char* ptr_src = _src + range.start() * _sstep ;
        unsigned char* ptr_dst = _dst + (_src_h - 1 - range.start()) * _dstep;
        int size = range.size();

        int width = _src_w * _sc;
        int cnt = width >>6;
        int paralle_num = cnt << 6;
        int remain = width - paralle_num;
        int i = 0;
        int j = 0;
        for (; i < size; i++) {
            const unsigned char* src_row0 = ptr_src;
            unsigned char* dst_row0 = ptr_dst;

            int nn = cnt;
            if (nn) {
#if __aarch64__
                asm volatile(
                    "0:                                               \n"
                    "ld1  {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64 \n"
                    "subs        %w2, %w2, #1                         \n"  // 64 processed per loop
                    "prfm   pldl1keep, [%0, 448]                      \n"
                    "st1 {v0.16b, v1.16b, v2.16b, v3.16b}, [%1], #64  \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "v0", "v1", "v2", "v3");
#else
                asm volatile(
                    "0:                                                  \n"
                    "vld1.8  {d0, d1, d2, d3}, [%0]!                     \n"
                    "vld1.8  {d4, d5, d6, d7}, [%0]!                     \n"
                    "subs    %2, %2, #1                                  \n" // 32 processed per loop
                    "pld [%0, #128]                                      \n"
                    "vst1.8 {d0, d1, d2, d3}, [%1]!                      \n"
                    "vst1.8 {d4, d5, d6, d7}, [%1]!                      \n"
                    "bgt        0b                                       \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8");
#endif
            }

            if (remain) {
                const unsigned char* src_row0 = ptr_src + paralle_num;
                unsigned char* dst_row0 = ptr_dst + paralle_num;

                for (j = 0; j < remain; j++) {
                    *(dst_row0++) = *(src_row0++);
                }
            }

            ptr_dst -= _dstep;
            ptr_src += _sstep;
        }
    } 

private:
    const unsigned char* _src;
    int _src_h;
    int _src_w;
    int _sc;
    int _sstep;
    unsigned char* _dst;
    int _dstep;
};

/*
 1 2 3  | 3 2 1
 4 5 6  | 6 5 4
 7 8 9  | 9 8 7
*****flip_y*****
*/
class FlipYC1Task : public ParallelTask {
public :
    FlipYC1Task(const unsigned char* src,
            int src_w,
            int sstep,
            unsigned char* dst,
            int dstep):
            _src(src),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){} 

    void operator() (const Range & range) const {
        int w_step = 32;
        int cnt = _src_w >> 5;
        int paralle_num = cnt << 5;
        int remain = _src_w - paralle_num;

        const unsigned char* ptr_src = _src + range.start() * _sstep;
        unsigned char* ptr_dst = _dst + (range.start() + 1) * _dstep;
        int size = range.size();

        int i = 0;
        int j = 0;
        for (; i < size; i++) {
            const unsigned char* src_row0 = ptr_src;
            unsigned char* dst_row0 = ptr_dst - w_step;

            int nn = cnt;
            if(nn) {
# if __aarch64__
                asm volatile(
                    "0:                                               \n"
                    "ld1  {v0.8b, v1.8b, v2.8b, v3.8b}, [%0], #32     \n"
                    "subs        %w2, %w2, #1                         \n"  // 32 processed per loop
                    "prfm   pldl1keep, [%0]                           \n"
                    "rev64  v15.8b, v0.8b                             \n"
                    "rev64  v14.8b, v1.8b                             \n"
                    "rev64  v13.8b, v2.8b                             \n"
                    "rev64  v12.8b, v3.8b                             \n"
                    "st1 {v12.8b, v13.8b, v14.8b, v15.8b}, [%1]       \n"
                    "sub  %1, %1, #32                                 \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "v0", "v1", "v2", "v3", "v12", "v13", "v14", "v15");
#else
                asm volatile(
                    "0:                                                  \n"
                    "vld1.8  {d0, d1, d2, d3}, [%0]!                     \n"
                    "subs    %2, %2, #1                                  \n" // 32 processed per loop
                    "pld [%0]                                            \n"
                    "vrev64.8  d7, d0                                    \n"
                    "vrev64.8  d6, d1                                    \n"
                    "vrev64.8  d5, d2                                    \n"
                    "vrev64.8  d4, d3                                    \n"
                    "vst1.8 {d4, d5, d6, d7}, [%1]                       \n"
                    "sub   %1, #32                                       \n"
                    "bgt        0b                                       \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
            }
            if (remain) {
                const unsigned char* src_row0 = ptr_src + paralle_num;
                unsigned char* dst_row0 = ptr_dst - paralle_num - 1;

                for (j = 0; j < remain; j++) {
                    *(dst_row0--) = *(src_row0++);
                }
            }

            ptr_dst += _dstep;
            ptr_src += _sstep;
        }
    }

private:
    const unsigned char* _src;
    int _src_w;
    int _sstep;
    unsigned char* _dst;
    int _dstep;
};

#if __aarch64__
static const unsigned char FlipYC3[16] = {
    15u, 14u, 13u, 12u, 11u, 10u, 9u, 8u,
    7u,  6u,  5u,  4u,  3u,  2u,  1u, 0u};
#endif

class FlipYC3Task : public ParallelTask {
public :
    FlipYC3Task(const unsigned char* src,
            int src_w,
            int sstep,
            unsigned char* dst,
            int dstep):
            _src(src),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){} 

    void operator() (const Range & range) const {
        int w_step = 48;
        int cnt = _src_w >> 4;
        int paralle_num = cnt << 4;
        int remain = _src_w - paralle_num;

        const unsigned char* ptr_src = _src + (range.start() + 1) * _sstep;
        unsigned char* ptr_dst = _dst + range.start() * _dstep;
        int size = range.size();

        int i = 0;
        int j = 0;
        for (; i < size; i++) {
            const unsigned char* src_row0 = ptr_src - w_step;
            unsigned char* dst_row0 = ptr_dst;

            int nn = cnt;
            if (nn) {
#if __aarch64__
                asm volatile(
                    "ld1         {v4.16b}, [%3]                       \n"
                    "0:                                               \n"
                    "ld3  {v0.16b, v1.16b, v2.16b}, [%0]              \n"
                    "subs        %w2, %w2, #1                         \n"  // 32 processed per loop
                    "prfm   pldl1keep, [%0, 192]                      \n"
                    "tbl   v15.16b, {v0.16b}, v4.16b                  \n"
                    "tbl   v16.16b, {v1.16b}, v4.16b                  \n"
                    "tbl   v17.16b, {v2.16b}, v4.16b                  \n"
                    "st3  {v15.16b, v16.16b, v17.16b}, [%1], #48      \n"
                    "sub    %0, %0, #48                               \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    : "r"(&FlipYC3)             // %3
                : "cc", "memory", "v0", "v1", "v2", "v12", "v13", "v14", "v15", "v16", "v17");
#else
                asm volatile(
                    "0:                                                  \n"
                    "vld3.8  {d0, d2, d4}, [%0]!                         \n"
                    "vld3.8  {d1, d3, d5}, [%0]                          \n"
                    "subs    %2, %2, #1                                  \n" // 16 processed per loop
                    "pld [%0, #128]                                       \n"
                    "vrev64.8  d7, d0                                    \n"
                    "vrev64.8  d6, d1                                    \n"
                    "vrev64.8  d9, d2                                    \n"
                    "vrev64.8  d8, d3                                    \n"
                    "vrev64.8  d11, d4                                   \n"
                    "vrev64.8  d10, d5                                   \n"
                    "vst3.8 {d6, d8, d10}, [%1]!                         \n"
                    "vst3.8 {d7, d9, d11}, [%1]!                         \n"
                    "sub  %0, #72                                        \n"
                    "bgt       0b                                        \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5");
#endif
            }
            if (remain) {
                int paralle_num_c3 = paralle_num * 3;
                const unsigned char* src_row0 = ptr_src - paralle_num_c3 - 3;
                unsigned char* dst_row0 = ptr_dst + paralle_num_c3;

                for (j = 0; j < remain; j++) {
                    *(dst_row0++) = *(src_row0++);
                    *(dst_row0++) = *(src_row0++);
                    *(dst_row0++) = *(src_row0++);
                    src_row0 -= 6;
                }
            }
            ptr_dst += _dstep;
            ptr_src += _sstep;
        }
    }

private:
    const unsigned char* _src;
    int _src_w;
    int _sstep;
    unsigned char* _dst;
    int _dstep;
};

#if __aarch64__
static const unsigned char FlipYC4[16] = {
    12u, 13u, 14u, 15u, 8u, 9u, 10u, 11u,
    4u,  5u,  6u,  7u,  0u, 1u, 2u,  3u};
#endif

class FlipYC4Task : public ParallelTask {
public :
    FlipYC4Task(const unsigned char* src,
            int src_w,
            int sstep,
            unsigned char* dst,
            int dstep):
            _src(src),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){} 

    void operator() (const Range & range) const {
        int w_step = 64;
        int cnt = _src_w >> 4;
        int paralle_num = cnt << 4;
        int remain = _src_w - paralle_num;

        const unsigned char* ptr_src = _src + range.start() * _sstep;
        unsigned char* ptr_dst = _dst + (range.start() + 1) * _dstep;
        int size = range.size();

        int i = 0;
        int j = 0;
        for (; i < size; i++) {
            const unsigned char* src_row0 = ptr_src;
            unsigned char* dst_row0 = ptr_dst - w_step;

            int nn = cnt;
            if (nn) {
#if __aarch64__
                asm volatile(
                    "ld1         {v4.16b}, [%4]                       \n"
                    "0:                                               \n"
                    "ld1  {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64 \n"
                    "subs        %w2, %w2, #1                         \n"  // 64 processed per loop
                    "prfm   pldl1keep, [%0, 448]                      \n"
                    "tbl   v15.16b, {v0.16b}, v4.16b                  \n"
                    "tbl   v14.16b, {v1.16b}, v4.16b                  \n"
                    "tbl   v13.16b, {v2.16b}, v4.16b                  \n"
                    "tbl   v12.16b, {v3.16b}, v4.16b                  \n"
                    "st1 {v12.16b, v13.16b, v14.16b, v15.16b}, [%1]   \n"
                    "sub %1, %1, #64                                  \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn),              // %2
                    "+r"(w_step)
                    : "r"(&FlipYC4)  // %4
                : "cc", "memory", "v0", "v1", "v2", "v3", "v12", "v13", "v14", "v15");
#else
                asm volatile(
                "0:                                                  \n"
                    "vld4.8  {d0, d2, d4, d6}, [%0]!                     \n"
                    "vld4.8  {d1, d3, d5, d7}, [%0]!                     \n"
                    "subs    %2, %2, #1                                  \n" // 64 processed per loop
                    "pld [%0, #128]                                      \n"
                    "vrev64.8  d9, d0                                    \n"
                    "vrev64.8  d8, d1                                    \n"
                    "vrev64.8  d11, d2                                   \n"
                    "vrev64.8  d10, d3                                   \n"
                    "vrev64.8  d13, d4                                   \n"
                    "vrev64.8  d12, d5                                   \n"
                    "vrev64.8  d15, d6                                   \n"
                    "vrev64.8  d14, d7                                   \n"
                    "vst4.8 {d8, d10, d12, d14}, [%1]!                  \n"
                    "vst4.8 {d9, d11, d13, d15}, [%1]                   \n"
                    "sub %1, #96                                         \n"
                    "bgt         0b                                      \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
        }
            if (remain) {
                int paralle_num_c4 = paralle_num << 2;
                const unsigned char* src_row0 = ptr_src + paralle_num_c4;
                unsigned char* dst_row0 = ptr_dst - paralle_num_c4 - 4;

                for (j = 0; j < remain; j++) {
                    *(dst_row0++) = *(src_row0++);
                    *(dst_row0++) = *(src_row0++);
                    *(dst_row0++) = *(src_row0++);
                    *(dst_row0++) = *(src_row0++);

                    dst_row0 -= 8;
                }
            }

            ptr_dst += _dstep;
            ptr_src += _sstep;
        }
    }

private:
    const unsigned char* _src;
    int _src_w;
    int _sstep;
    unsigned char* _dst;
    int _dstep;
};

#if __aarch64__
static const char ShuffleUVFlipY[16] = {
    14u, 15u, 12u, 13u, 10u, 11u, 8u, 9u,
    6u, 7u, 4u, 5u, 2u, 3u, 0u, 1u};
#endif

class FlipUVYTask : public ParallelTask {

public :
    FlipUVYTask(const unsigned char* src,
            int src_w,
            int sstep,
            unsigned char* dst,
            int dstep):
            _src(src),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){} 

    void operator() (const Range & range) const {
        int w_step = 16;
        int paralle_num = _src_w & (~15);
        int remain = (_src_w - paralle_num) >> 1;
        int four_sstep = _sstep << 2;
        int four_dstep = _dstep << 2;

        const unsigned char* ptr_src = _src + range.start() * _sstep;
        unsigned char* ptr_dst = _dst + (range.start() + 1) * _dstep;
        int size = range.size();
        int h_align4 = size & (~3);

        int i = 0;
        int j = 0;
        for (; i < h_align4; i += 4) {
            const unsigned char* src_row0 = ptr_src;
            const unsigned char* src_row1 = src_row0 + _sstep;
            const unsigned char* src_row2 = src_row1 + _sstep;
            const unsigned char* src_row3 = src_row2 + _sstep;

            unsigned char* dst_row0 = ptr_dst - w_step;
            unsigned char* dst_row1 = dst_row0 + _dstep;
            unsigned char* dst_row2 = dst_row1 + _dstep;
            unsigned char* dst_row3 = dst_row2 + _dstep;

            int nn = paralle_num;
            if (nn) {
#if __aarch64__
                asm volatile(
                    "ld1         {v8.16b}, [%9]                       \n"
                    "0:                                               \n"
                    "ld1  {v0.16b}, [%0], #16     \n"
                    "subs      %w8, %w8,  #16                         \n"  // 32 processed per loop
                    "ld1  {v1.16b}, [%1], #16     \n"
                    "ld1  {v2.16b}, [%2], #16     \n"
                    "ld1  {v3.16b}, [%3], #16     \n"
                    "prfm   pldl1keep, [%0]                           \n"
                    "prfm   pldl1keep, [%1]                           \n"
                    "prfm   pldl1keep, [%2]                           \n"
                    "prfm   pldl1keep, [%3]                           \n"
                    "tbl  v4.16b, {v0.16b}, v8.16b                   \n"
                    "tbl  v5.16b, {v1.16b}, v8.16b                   \n"
                    "tbl  v6.16b, {v2.16b}, v8.16b                   \n"
                    "tbl  v7.16b, {v3.16b}, v8.16b                   \n"

                    "st1 {v4.16b}, [%4]                     \n"
                    "st1 {v5.16b}, [%5]                     \n"
                    "st1 {v6.16b}, [%6]                     \n"
                    "st1 {v7.16b}, [%7]                     \n"
                    "sub  %4, %4, #16                                 \n"
                    "sub  %5, %5, #16                                 \n"
                    "sub  %6, %6, #16                                 \n"
                    "sub  %7, %7, #16                                 \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(src_row1),        // %1
                    "+r"(src_row2),        // %2
                    "+r"(src_row3),        // %3
                    "+r"(dst_row0),        // %4
                    "+r"(dst_row1),        // %5
                    "+r"(dst_row2),        // %6
                    "+r"(dst_row3),        // %7
                    "+r"(nn)               // %8
                    : "r"(&ShuffleUVFlipY)
                    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8");
#else
                asm volatile(
                    "0:                                                  \n"
                    "vld2.8  {d0, d1}, [%0]!                             \n"
                    "vld2.8  {d2, d3}, [%1]!                             \n"
                    "subs    %8, %8, #16                                 \n" // 32 processed per loop
                    "vld2.8  {d4, d5}, [%2]!                             \n"
                    "vld2.8  {d6, d7}, [%3]!                             \n"
                    "pld [%0]                                            \n"
                    "pld [%1]                                            \n"
                    "pld [%2]                                            \n"
                    "pld [%3]                                            \n"
                    "vrev64.8  q0, q0                                    \n"
                    "vrev64.8  q1, q1                                    \n"
                    "vrev64.8  q2, q2                                    \n"
                    "vrev64.8  q3, q3                                    \n"
                    
                    "vst2.8 {d0, d1}, [%4]                              \n"
                    "vst2.8 {d2, d3}, [%5]                              \n"
                    "vst2.8 {d4, d5}, [%6]                              \n"
                    "vst2.8 {d6, d7}, [%7]                              \n"
                    "sub   %4, #16                                      \n"
                    "sub   %5, #16                                      \n"
                    "sub   %6, #16                                      \n"
                    "sub   %7, #16                                      \n"
                    "bgt        0b                                      \n"
                    : "+r"(src_row0),      // %0
                    "+r"(src_row1),        // %1
                    "+r"(src_row2),        // %2
                    "+r"(src_row3),        // %3
                    "+r"(dst_row0),        // %4
                    "+r"(dst_row1),        // %5
                    "+r"(dst_row2),        // %6
                    "+r"(dst_row3),        // %7
                    "+r"(nn)              // %8
                    :
                : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15");
#endif
            }
            if (remain) {
                const unsigned char* src_row0 = ptr_src + paralle_num;
                const unsigned char* src_row1 = src_row0 + _sstep;
                const unsigned char* src_row2 = src_row1 + _sstep;
                const unsigned char* src_row3 = src_row2 + _sstep;

                unsigned char* dst_row0 = ptr_dst - paralle_num - 2;
                unsigned char* dst_row1 = dst_row0 + _dstep;
                unsigned char* dst_row2 = dst_row1 + _dstep;
                unsigned char* dst_row3 = dst_row2 + _dstep;

                for (j = 0; j < remain; j++) {
                    dst_row0[0] = src_row0[0];
                    dst_row0[1] = src_row0[1];
                    dst_row1[0] = src_row1[0];
                    dst_row1[1] = src_row1[1];
                    dst_row2[0] = src_row2[0];
                    dst_row2[1] = src_row2[1];
                    dst_row3[0] = src_row3[0];
                    dst_row3[1] = src_row3[1];
                    dst_row0 -= 2;
                    src_row0 += 2;
                    dst_row1 -= 2;
                    src_row1 += 2;
                    dst_row2 -= 2;
                    src_row2 += 2;
                    dst_row3 -= 2;
                    src_row3 += 2;
                }
            }

            ptr_dst += four_dstep;
            ptr_src += four_sstep;
        }

        for (; i < size; i++) {
            const unsigned char* src_row0 = ptr_src;
            unsigned char* dst_row0 = ptr_dst - w_step;

            int nn = paralle_num;
            if (nn) {
#if __aarch64__
                asm volatile(
                    "ld1         {v8.16b}, [%3]                       \n"
                    "0:                                               \n"
                    "ld1  {v0.16b}, [%0], #16                         \n"
                    "subs        %w2, %w2, #16                        \n"  // 32 processed per loop
                    "prfm   pldl1keep, [%0]                           \n"
                    "tbl  v1.16b, {v0.16b}, v8.16b                    \n"
                    "st1 {v1.16b}, [%1]                               \n"
                    "sub  %1, %1, #16                                 \n"
                    "b.gt        0b                                   \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    : "r"(&ShuffleUVFlipY)
                : "cc", "memory", "v0", "v1", "v8");
#else
                asm volatile(
                    "0:                                                  \n"
                    "vld2.8  {d0, d1}, [%0]!                             \n"
                    "subs    %2, %2, #16                                  \n" // 32 processed per loop
                    "pld [%0]                                            \n"
                    "vrev64.8  d0, d0                                   \n"
                    "vrev64.8  d1, d1                                   \n"
                    "vst2.8 {d0, d1}, [%1]                               \n"
                    "sub   %1, #16                                       \n"
                    "bgt        0b                                       \n"
                    : "+r"(src_row0),      // %0
                    "+r"(dst_row0),        // %1
                    "+r"(nn)               // %2
                    :
                : "cc", "memory", "d0", "d1");
#endif
            }
            if (remain) {
                const unsigned char* src_row0 = ptr_src + paralle_num;
                unsigned char* dst_row0 = ptr_dst - paralle_num - 2;

                for (j = 0; j < remain; j++) {
                    dst_row0[0] = src_row0[0];
                    dst_row0[1] = src_row0[1];
                    dst_row0 -= 2;
                    src_row0 += 2;
                }
            }

            ptr_dst += _dstep;
            ptr_src += _sstep;
        }
    }

private:
    const unsigned char* _src;
    int _src_w;
    int _sstep;
    unsigned char* _dst;
    int _dstep;
};

void flip_y_neon_u8(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        unsigned char* dst,
        int dstep) {
    if (1 == sc) {
        FlipYC1Task task(src, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else if (3 == sc) {
        FlipYC3Task task(src, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else if (4 == sc) {
        FlipYC4Task task(src, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else {
        LOG_ERR("flip y not support the channel yet !");
    }
}

void flip_neon_u8(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        unsigned char* dst,
        int dstep,
        FlipType type) {
    if (type == FlipType::X) {
        FlipXTask task(src, src_h, src_w, sc, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else if (FlipType::Y == type) {
        flip_y_neon_u8(src, src_h, src_w, sc, sstep, dst, dstep);
    } else {
        LOG_ERR( "flip type not support yet !");
    }
    return;
}

void flip_neon_uv_u8(
        const unsigned char* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        unsigned char* dst,
        int dstep,
        FlipType type) {
    if (type == FlipType::X) {
        FlipXTask task(src, src_h, src_w, sc, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else if (FlipType::Y == type) {
        FlipUVYTask task(src, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else {
        LOG_ERR( "flip type not support yet !");
    }
    return;
}

int flip_neon(const Mat& src, Mat& dst, FlipType type) {
    const int src_w = src.width();
    const int src_h = src.height();
    const void* src_ptr = (const void *)src.data();
    void* dst_ptr = (void *)dst.data();

    int sc = src.channels();
    const int s_stride = src.stride();
    const int d_stride = dst.stride();

    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        flip_neon_u8((unsigned char*)src_ptr, src_h, src_w, sc, s_stride, 
        (unsigned char*)dst_ptr, d_stride, type);
        break;

    case FCVImageType::NV12:
    case FCVImageType::NV21:
        flip_neon_u8((unsigned char*)src_ptr, src_h, src_w, 1,
                s_stride, (unsigned char *)dst_ptr, d_stride, type);
        flip_neon_uv_u8(((unsigned char*)src_ptr + src_w * src_h), (src_h >> 1),
                src_w, 1, s_stride, ((unsigned char *)dst_ptr + src_w * src_h), d_stride, type);
        break;
    default:
        LOG_ERR("flip type not support yet!");
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
