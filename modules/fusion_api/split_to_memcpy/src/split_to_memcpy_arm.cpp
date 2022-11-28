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

#include "modules/fusion_api/split_to_memcpy/include/split_to_memcpy_arm.h"

#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/macro_utils.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class SplitToMemcpyArmTask : public ParallelTask {

public:
    SplitToMemcpyArmTask(const float *src,
            float *dst,
            int channel,
            int count) : 
            _src(src),
            _dst(dst),
            _channel(channel),
            _count(count){}

    void operator ()(const Range & range) const {
        const float* src_ptr = _src + (range.start() * _channel);
        float * dst_ptr = _dst + range.start();
        int cnt = range.size();
        int nn = cnt >> 2;

        int remain = cnt - (nn << 2);
        
        if (_channel == 3) {
            float *dstc1_ptr = dst_ptr;
            float *dstc2_ptr = dstc1_ptr + _count;
            float *dstc3_ptr = dstc2_ptr + _count; 

            if (nn != 0) {

#if __aarch64__
        asm volatile(
            "0:                                                \n"
            "prfm  pldl1keep, [%0, #128]                       \n"
            "ld3 {v0.4s, v1.4s, v2.4s}, [%0], #48              \n" // load 4 pixels of BGR.
            "subs        %w4, %w4, #1                          \n" // 4 processed per loop
            "st1 {v0.4s}, [%1], #16                            \n"
            "st1 {v1.4s}, [%2], #16                            \n"
            "st1 {v2.4s}, [%3], #16                            \n"
            "b.gt        0b                                    \n"
            : "+r"(src_ptr),              // %0
            "+r"(dstc1_ptr),                // %1
            "+r"(dstc2_ptr),                // %2
            "+r"(dstc3_ptr),                // %3
            "+r"(nn)                       // %4
            :
        : "cc", "memory", "v0", "v1", "v2");
#else
            asm volatile(
            "0:                                              \n"
            "vld3.32      {d0, d2, d4}, [%0]!                 \n"  // load 2 FLOAT pixels of BGR.
            "vld3.32      {d1, d3, d5}, [%0]!                 \n"  // next 2 RGB
            "subs         %4, %4, #1                         \n"

            "vst1.32 {q0}, [%1]!                              \n" // store 4 float pixels of B
            "vst1.32 {q1}, [%2]!                              \n" // store 4 float pixels of G
            "vst1.32 {q2}, [%3]!                              \n" // store 4 float pixels of R
            "bgt        0b                                   \n"

             : "+r"(src_ptr),                // %0
            "+r"(dstc1_ptr),                 // %1
            "+r"(dstc2_ptr),                 // %2
            "+r"(dstc3_ptr),                 // %3
            "+r"(nn)                        // %4
            :
        : "cc", "memory", "d0", "d1", "d2");
#endif  
            }

            for (int i = 0; i < remain; ++i) {
                *(dstc1_ptr++) = *(src_ptr++);
                *(dstc2_ptr++) = *(src_ptr++);
                *(dstc3_ptr++) = *(src_ptr++);
            }      
        } else if (_channel == 4) {
            float *dstc1_ptr = dst_ptr;
            float *dstc2_ptr = dstc1_ptr + _count;
            float *dstc3_ptr = dstc2_ptr + _count; 
            float *dstc4_ptr = dstc3_ptr + _count;

            if (nn != 0) {
#if __aarch64__
        asm volatile(
            "0:                                                 \n"
            "prfm  pldl1keep, [%0, #128]                        \n"

            "ld4 {v0.4s, v1.4s, v2.4s, v3.4s}, [%0], #64       \n" // load 4 float pixels of BGR.
            "subs        %w5, %w5, #1                          \n"  // 4 processed per loop

            "st1 {v0.4s}, [%1], #16                          \n"
            "st1 {v1.4s}, [%2], #16                          \n"
            "st1 {v2.4s}, [%3], #16                          \n"
            "st1 {v3.4s}, [%4], #16                          \n"
            "b.gt        0b                                  \n"
            : "+r"(src_ptr),               // %0
            "+r"(dstc1_ptr),                 // %1
            "+r"(dstc2_ptr),                 // %2
            "+r"(dstc3_ptr),                 // %3
            "+r"(dstc4_ptr),                 // %4
            "+r"(nn)                        // %5
            :
        : "cc", "memory", "v0", "v1", "v2", "v3");
#else
            asm volatile(
            "0:                                                \n"
            "vld4.32      {d0, d2, d4, d6}, [%0]!               \n"  // load 2 float pixels of BGR.
            "vld4.32      {d1, d3, d5, d7}, [%0]!               \n"  // load next 2 pixels of BGR .

            "subs         %5, %5, #1                          \n"
            "vst1.32 {q0}, [%1]!                               \n" // atore 4 float pixels of B
            "vst1.32 {q1}, [%2]!                               \n" // atore 4 float pixels of G
            "vst1.32 {q2}, [%3]!                               \n" // atore 4 float pixels of R
            "vst1.32 {q3}, [%4]!                               \n" // atore 4 float pixels of A
            "bgt        0b                                    \n"

             : "+r"(src_ptr),                // %0
            "+r"(dstc1_ptr),                  // %1
            "+r"(dstc2_ptr),                  // %2
            "+r"(dstc3_ptr),                  // %3
            "+r"(dstc4_ptr),                  // %4
            "+r"(nn)                         // %5
            :
        : "cc", "memory", "q0", "q1", "q2", "q3");
#endif
            }

            for (int i = 0; i < remain; ++i) {
                *(dstc1_ptr++) = *(src_ptr++);
                *(dstc2_ptr++) = *(src_ptr++);
                *(dstc3_ptr++) = *(src_ptr++);
                *(dstc4_ptr++) = *(src_ptr++);
            } 
        } else {
            LOG_ERR("The channel is not supported!");
       }
    }  

private:
    const float* _src;
    float* _dst;
    int _channel;
    int _count;
};

int split_to_memcpy_neon(
        const float * src,
        int width,
        int height,
        int channel,
        float* dst) {
    int count = width * height;
    if (channel == 1) {
        memcpy(dst, src, count * sizeof(float));
        return 0;
    }

    SplitToMemcpyArmTask task(src, dst, channel, count);
    parallel_run(Range(0, count), task);
    return 0;
}

G_FCV_NAMESPACE1_END()
