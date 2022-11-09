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

#include "modules/fusion_api/bgr_to_rgba_with_mask/include/bgr_to_rgba_with_mask_sse.h"

#include "modules/core/base/include/common_sse.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int bgr_to_rgba_with_mask_sse(Mat& src, Mat& mask, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int mask_stride = mask.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *mask_ptr = (unsigned char *)mask.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int width_align16 = src_w & (~15);
    int remain = src_w & (15); // calculate the remainder
    unsigned char b00 = 0;
    unsigned char g00 = 0;
    unsigned char r00 = 0;
    unsigned char a00 = 0;

    for (int i = 0; i < src_h; i++) {
        unsigned char *src_row = src_ptr;
        unsigned char *dst_row = dst_ptr;
        unsigned char *mask_row = mask_ptr;

        for (int j = 0; j < width_align16; j += 16) {
            __m128i bgr0, bgr1, bgr2, mask;

            vld3_u8x16_sse(src_row, &bgr0, &bgr1, &bgr2);
            mask = _mm_loadu_si128((const __m128i*)mask_row);
            vst4_u8x16_sse(&bgr2, &bgr1, &bgr0, &mask, dst_row);

            src_row += 48;
            mask_row += 16;
            dst_row += 64;
        }

        if (remain) {
            const unsigned char *srcl_ptr = src_ptr + width_align16 * 3;
            const unsigned char *merl_ptr = mask_ptr + width_align16;
            unsigned char *dstl_ptr = dst_ptr + (width_align16 << 2);

            for (int j = 0; j < remain; ++j) {
                b00 = srcl_ptr[0];
                g00 = srcl_ptr[1];
                r00 = srcl_ptr[2];
                a00 = merl_ptr[0];

                dstl_ptr[0] = r00;
                dstl_ptr[1] = g00;
                dstl_ptr[2] = b00;
                dstl_ptr[3] = a00;

                srcl_ptr += 3;
                dstl_ptr += 4;
                merl_ptr += 1;
            }
        }

        src_ptr += src_stride;
        dst_ptr += dst_stride;
        mask_ptr += mask_stride;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
