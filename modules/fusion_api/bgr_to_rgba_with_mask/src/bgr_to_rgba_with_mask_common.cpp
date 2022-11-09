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

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int bgr_to_rgba_with_mask_common(Mat& src, Mat& mask, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    unsigned char *mask_ptr = (unsigned char *)mask.data();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int mask_stride = mask.stride();

    unsigned char b00 = 0;
    unsigned char g00 = 0;
    unsigned char r00 = 0;
    unsigned char a00 = 0;

    for (int i = 0; i < src_h; i ++) {
        // caculate the start address of every row
        const unsigned char *src_ptr0 = src_ptr;
        const unsigned char *mer_ptr0 = mask_ptr;
        unsigned char *dst_ptr0 = dst_ptr;

        for (int j = 0; j < src_w; j ++) {
            // bgr to rgb pixel storage format, the mer_ptr0[0] store in the fourth channel
            b00 = src_ptr0[0];
            g00 = src_ptr0[1];
            r00 = src_ptr0[2];
            a00 = mer_ptr0[0];

            dst_ptr0[0] = r00;
            dst_ptr0[1] = g00;
            dst_ptr0[2] = b00;
            dst_ptr0[3] = a00;

            src_ptr0 += 3;
            dst_ptr0 += 4;
            mer_ptr0 += 1;
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
        mask_ptr += mask_stride;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
