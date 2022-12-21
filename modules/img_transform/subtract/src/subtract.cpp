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

#include "modules/img_transform/subtract/interface/subtract.h"

#include "modules/core/base/include/type_info.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/subtract/include/subtract_common.h"
#ifdef HAVE_NEON
#include "modules/img_transform/subtract/include/subtract_arm.h"
#endif

#ifdef HAVE_SVE2
#include "modules/img_transform/subtract/include/subtract_sve.h"
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class SubtractParallelTask : public ParallelTask {
public:
    SubtractParallelTask(
            int src_stride,
            const float* src_ptr,
            int dst_stride,
            float* dst_ptr,
            int src_w, 
            int channel, 
            Scalar scalar)
            : _src_stride(src_stride),
            _src_ptr(src_ptr),
            _dst_stride(dst_stride),
            _dst_ptr(dst_ptr),
            _src_w(src_w),
            _channel(channel),  
            _scalar(scalar) {}

    void operator() (const Range& range) const override {
        int src_offset = range.start() * (_src_stride / sizeof(float));
        int dst_offset = range.start() * (_dst_stride / sizeof(float));
        const float* ptr_cur_src = _src_ptr + src_offset;
        float* ptr_cur_dst = _dst_ptr + dst_offset;
        int valid_h = range.end() - range.start();

        int valid_stride = _src_stride >> 2;

#ifdef HAVE_SVE2
        subtract_sve(ptr_cur_src, _src_w, valid_h, valid_stride, _channel, _scalar, ptr_cur_dst);
#elif defined(HAVE_NEON)
        subtract_neon(ptr_cur_src, _src_w, valid_h, valid_stride, _channel, _scalar, ptr_cur_dst);
#else
        subtract_common(ptr_cur_src, _src_w, valid_h, valid_stride, _channel, _scalar, ptr_cur_dst);
#endif

    }
private:
    int _src_stride;
    const float* _src_ptr;
    int _dst_stride;
    float* _dst_ptr;
    int _src_w;
    int _channel;
    Scalar _scalar;
};

static void subtract_multi_thread(const Mat& src, Scalar scalar, Mat& dst) {
    int src_w = src.width();
    int src_h = src.height();
    int channel = src.channels();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    const float* src_ptr = (const float*)src.data();
    float* dst_ptr = (float*)dst.data();

    SubtractParallelTask task(src_stride, src_ptr, dst_stride, dst_ptr, src_w, channel, scalar);
    parallel_run(Range(0, src_h), task);
}

int subtract(const Mat& src, Scalar scalar, Mat& dst) {
    if (src.empty()) {
        LOG_ERR("the src is empty!");
        return -1;
    }
    TypeInfo cur_type_info;
    if (get_type_info(src.type(), cur_type_info)) {
        LOG_ERR("failed to get type info from src mat while get_type_info");
        return -1;
    }
    if (cur_type_info.data_type != DataType::F32) {
        LOG_ERR("subtract only support f32 data, the current src element data type is %d", 
                int(cur_type_info.data_type));
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(src.width(), src.height(), src.type());
    }

    if (dst.width() != src.width() 
            || dst.height() != src.height() 
            || dst.channels() != src.channels() 
            || dst.type() != src.type()) {
        LOG_ERR("illegal format of dst mat to subtract, which should be same size and type with src");
    }

    subtract_multi_thread(src, scalar, dst);

    return 0;
}

G_FCV_NAMESPACE1_END()