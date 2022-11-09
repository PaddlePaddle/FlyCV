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

#include "modules/fusion_api/split_to_memcpy/include/split_to_memcpy_common.h"
#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class SplitToMemcpyTask : public ParallelTask {

public:
    SplitToMemcpyTask(const float* src, 
            float * dst,
            int channel,
            int count) : 
            _src(src), 
            _dst(dst),
            _channel(channel),
            _count(count){}

    void operator() (const Range & range) const {
        const float* src_ptr = _src + (range.start() * _channel);
        float * dst_ptr = _dst + range.start();
        int cnt = range.size();
        
        if (_channel == 3) {
            float *dstc1_ptr = dst_ptr;
            float *dstc2_ptr = dstc1_ptr + _count;
            float *dstc3_ptr = dstc2_ptr + _count; 
            for (int n = 0; n< cnt; n++) {
                *(dstc1_ptr++) = *(src_ptr++);
                *(dstc2_ptr++) = *(src_ptr++);
                *(dstc3_ptr++) = *(src_ptr++);              
            }
        } else if (_channel == 4) {
            float *dstc1_ptr = dst_ptr;
            float *dstc2_ptr = dstc1_ptr + _count;
            float *dstc3_ptr = dstc2_ptr + _count; 
            float *dstc4_ptr = dstc3_ptr + _count;
            for (int n = 0; n< cnt; n++) {
                *(dstc1_ptr++) = *(src_ptr++);
                *(dstc2_ptr++) = *(src_ptr++);
                *(dstc3_ptr++) = *(src_ptr++);
                *(dstc4_ptr++) = *(src_ptr++);
            }
        }
    }

private:
    const float* _src;
    float* _dst;
    int _channel;
    int _count;

};

int split_to_memcpy_common(
        const float* src,
        int width,
        int height,
        int channel,
        float* dst) {
            
    int count = width * height;
    if (channel == 1) {
        memcpy(dst, src, count * sizeof(float));
        return 0;
    }        
    SplitToMemcpyTask task(src, dst, channel, count);
    parallel_run(Range(0, count), task);
    return 0;
}

G_FCV_NAMESPACE1_END()
