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

#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_common.h"
#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class BgrToRgbaWithMaskCommonParallelTask : public ParallelTask {
public:
    BgrToRgbaWithMaskCommonParallelTask(
            const unsigned char *src_ptr,
            unsigned char *dst_ptr,
            const unsigned char *mask_ptr,
            int src_stride,
            int dst_stride,
            int mask_stride,
            int src_w) : 
            _src_ptr(src_ptr),
            _dst_ptr(dst_ptr),
            _mask_ptr(mask_ptr),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _mask_stride(mask_stride),
            _src_w(src_w) {}

    void operator()(const Range& range) const override {
        for (int i = range.start(); i < range.end(); i++) {
            // caculate the start address of every row
            const unsigned char *src_ptr0 = _src_ptr + i * _src_stride;
            const unsigned char *mer_ptr0 = _mask_ptr + i * _mask_stride;
            unsigned char *dst_ptr0 = _dst_ptr + i * _dst_stride;

            for (int j = 0; j < _src_w; j ++) {
                // bgr to rgb pixel storage format, the mer_ptr0[0] store in the fourth channel

                dst_ptr0[0] = src_ptr0[2];
                dst_ptr0[1] = src_ptr0[1];
                dst_ptr0[2] = src_ptr0[0];
                dst_ptr0[3] = mer_ptr0[0];

                src_ptr0 += 3;
                dst_ptr0 += 4;
                mer_ptr0 += 1;
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
    int _src_w;
};

int bgr_to_rgba_with_mask_common(Mat& src, Mat& mask, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    unsigned char *mask_ptr = (unsigned char *)mask.data();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int mask_stride = mask.stride();

    BgrToRgbaWithMaskCommonParallelTask task(
            src_ptr,
            dst_ptr,
            mask_ptr,
            src_stride,
            dst_stride,
            mask_stride,
            src_w);

    parallel_run(Range(0, src_h), task);

    return 0;
}

G_FCV_NAMESPACE1_END()
