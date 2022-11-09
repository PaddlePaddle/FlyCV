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

#include "modules/fusion_api/y420sp_to_resize_to_bgr/include/y420sp_to_resize_to_bgr_common.h"

#include <stdlib.h>

#include "modules/img_transform/resize/include/resize_common.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int bgra_to_resize_bilinear_to_bgr_common(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int dst_bgra_stride = (dst_stride / 3) << 2;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = dst_bgra_stride << 2; // dst_w * 2 * 4 * sizeof(short);
    int* buf = (int *)malloc(buf_size);
    unsigned short *rows = (unsigned short*)malloc(rows_size);

    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);

    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;
    unsigned char *ptr_dst = nullptr;
    int sy0 = 0;
    for (int dy = 0; dy < dst_h; dy++) {
        rows0 = rows;
        rows1 = rows + dst_bgra_stride; // the stride of rows should calculate according to bgra
        sy0 = *(yofs + dy);
        unsigned short *alphap = alpha;
        ptr_dst = dst_ptr + dst_stride * dy;

        hresize_bn_one_row(src_ptr, xofs, sy0, src_stride,
                dst_w, 4, alphap, rows0, rows1);

        unsigned int b0 = *(beta++);
        unsigned int b1 = *(beta++);
        for (int dx = 0; dx < dst_w; dx++) {
            ptr_dst[0] = fcv_cast_u8((rows0[0] * b0 + rows1[0] * b1 + (1 << 17)) >> 18);
            ptr_dst[1] = fcv_cast_u8((rows0[1] * b0 + rows1[1] * b1 + (1 << 17)) >> 18);
            ptr_dst[2] = fcv_cast_u8((rows0[2] * b0 + rows1[2] * b1 + (1 << 17)) >> 18);

            ptr_dst += 3;
            rows0 += 4;
            rows1 += 4;
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }

    return 0;
}


int bgra_to_resize_nearest_to_bgr_common(
        Mat& src,
        Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    int buf_size = (dst_w + dst_h) << 2; //(dst_w + dst_h) * sizeof(int)
    int* buf = (int *)malloc(buf_size);

    int* xofs = buf;
    int* yofs = buf + dst_w;

    nearest_cal_offset(xofs, dst_w, src_w);
    nearest_cal_offset(yofs, dst_h, src_h);

    int dy = 0, dx= 0;
    for (; dy < dst_h; dy++) {
        const unsigned char* src0 = src_ptr + (yofs[dy] >> 16) * src_stride;
        unsigned char * ptr_dst0 = dst_ptr + dst_stride * dy;

        for (dx = 0; dx < dst_w; dx++) {
            int idx0 = (xofs[dx] >> 16) << 2; // (<< 16) << 2;

            ptr_dst0[dx * 3] = src0[idx0];
            ptr_dst0[dx * 3 + 1] = src0[idx0 + 1];
            ptr_dst0[dx * 3 + 2] = src0[idx0 + 2];
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()