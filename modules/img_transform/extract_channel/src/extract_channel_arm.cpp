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

#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class ExtractWithMemcpyNeonTask : public ParallelTask {
public:
    ExtractWithMemcpyNeonTask(const unsigned char* src, int channel, int count, int index, unsigned char* dst)
            : _src(src), _dst(dst), _channel(channel), _count(count), _index(index) {}

    void operator()(const Range& range) const {
        const unsigned char* src_ptr = _src + (range.start() * _channel);
        unsigned char* dst_ptr = _dst + range.start();
        int cnt = range.size();
        int nn = cnt >> 4;
        int remain = cnt - (nn << 4);
        // std::cout << "remain: " << remain << std::endl;

        if (_channel == 3) {
            if (nn != 0) {
                switch (_index) {
                case 0:
#if __aarch64__
                    asm volatile(
                            "0:                                                \n"  // label 0
                            "prfm  pldl1keep, [%0, #128]                       \n"
                            "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n"  // load 16 pixels of BGR.
                            "subs        %w2, %w2, #1                          \n"  // 16 processed per loop
                            "st1 {v0.16b}, [%1], #16                           \n"
                            "b.gt        0b                                    \n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "v0", "v1", "v2");
#else
                    asm volatile(
                            "0:                                                \n"
                            "vld3.8      {d0, d2, d4}, [%0]!                   \n"  // load 16 u8 pixels of BGR.
                            "vld3.8      {d1, d3, d5}, [%0]!                   \n"  // next 2 RGB
                            "subs         %2, %2, #1                           \n"
                            "vst1.8       {q0}, [%1]!                          \n"  // store 16 pixels of index
                            "bgt          0b                                   \n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "q0", "q1", "q2");
#endif
                    break;
                case 1:
#if __aarch64__
                    asm volatile(
                            "0:                                                \n"  // label 0
                            "prfm  pldl1keep, [%0, #128]                       \n"
                            "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n"  // load 16 pixels of BGR.
                            "subs        %w2, %w2, #1                          \n"  // 16 processed per loop
                            "st1 {v1.16b}, [%1], #16                           \n"
                            "b.gt        0b                                    \n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "v0", "v1", "v2");
#else
                    asm volatile(
                            "0:                                                \n"
                            "vld3.8      {d0, d2, d4}, [%0]!                   \n"  // load 16 u8 pixels of BGR.
                            "vld3.8      {d1, d3, d5}, [%0]!                   \n"  // next 2 RGB
                            "subs         %2, %2, #1                           \n"
                            "vst1.8       {q1}, [%1]!                          \n"  // store 16 pixels of index
                            "bgt          0b                                   "
                            "\n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "q0", "q1", "q2");
#endif
                    break;
                case 2:
#if __aarch64__
                    asm volatile(
                            "0:                                                \n"  // label 0
                            "prfm  pldl1keep, [%0, #128]                       \n"
                            "ld3 {v0.16b, v1.16b, v2.16b}, [%0], #48           \n"  // load 16 pixels of BGR.
                            "subs        %w2, %w2, #1                          \n"  // 16 processed per loop
                            "st1 {v2.16b}, [%1], #16                           \n"
                            "b.gt        0b                                    \n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "v0", "v1", "v2");
#else
                    asm volatile(
                            "0:                                                \n"
                            "vld3.8      {d0, d2, d4}, [%0]!                   \n"  // load 16 u8 pixels of BGR.
                            "vld3.8      {d1, d3, d5}, [%0]!                   \n"  // next 2 RGB
                            "subs         %2, %2, #1                           \n"
                            "vst1.8       {q2}, [%1]!                          \n"  // store 16 pixels of index
                            "bgt          0b                                   "
                            "\n"
                            : "+r"(src_ptr),  // %0
                              "+r"(dst_ptr),  // %1
                              "+r"(nn)        // %2
                            :
                            : "cc", "memory", "q0", "q1", "q2");
#endif
                    break;
                default:
                    break;
                }
            }

            for (int i = 0; i < remain; ++i) {
                *(dst_ptr + i) = *(src_ptr + _index + i * _channel);
            }
        } else {
            LOG_ERR("the channel count is not supported!\n");
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    int _channel;
    int _count;
    int _index;
};

int extract_channel_neon(Mat& src, Mat& dst, int index) {
    const void* src_ptr = (const void*)src.data();
    void* dst_ptr = (void*)dst.data();

    int src_w = src.width();
    int src_h = src.height();
    int src_c = src.channels();
    int src_s = src.stride() / sizeof(unsigned char);
    src_w = FCV_MAX(src_w, src_s / src_c);
    int count = src_w * src_h;

    ExtractWithMemcpyNeonTask task((const unsigned char*)src_ptr, src_c, count, index, (unsigned char*)dst_ptr);
    parallel_run(Range(0, count), task);

    return 0;
}

G_FCV_NAMESPACE1_END()
