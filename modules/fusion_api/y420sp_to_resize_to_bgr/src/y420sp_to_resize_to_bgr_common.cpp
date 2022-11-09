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

int y420sp_to_resize_bilinear_to_bgr_common(
        Mat& src,
        Mat& dst,
        bool is_nv12) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();

    //const int dst_stride = dst.stride();
    const int dst_stride = dst.width();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;

    unsigned char *src_uv_ptr = src_ptr + src_w * src_h;

    const int d2_stride = dst_stride << 1;
    const int d3_stride = dst_stride + d2_stride;

    int buf_size = (dst_w + dst_h) << 3; //(dst_w + dst_h) * 2 * sizeof(int)
    int rows_size = dst_stride << 3;

    int* buf = (int *)malloc(buf_size);
    memset(buf, 0, buf_size);

    unsigned short* rows = (unsigned short*)malloc(rows_size);
    memset(rows, 0, rows_size);

    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 1, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows00 = nullptr;
    unsigned short *rows01 = nullptr;
    unsigned short *rows10 = nullptr;
    unsigned short *rows11 = nullptr;
    unsigned char *ptr_dst0 = nullptr;
    unsigned char *ptr_dst1 = nullptr;

    int dy = 0, dx= 0, sy0 = 0, sy1 = 0, channel = 3;
    for (; dy < dst_h; dy += 2) {
        const int dst_offset = d3_stride * dy;
        rows00 = rows;
        rows01 = rows + dst_stride;
        rows10 = rows + d2_stride;
        rows11 = rows + d3_stride;
        sy0 = *(yofs + dy);
        sy1 = *(yofs + dy + 1);

        unsigned short *alphap = alpha;
        ptr_dst0 = dst_ptr + dst_offset;
        ptr_dst1 = dst_ptr + dst_offset + d3_stride;

        hresize_bn_one_row(src_ptr, xofs, sy0, src_w, dst_w, 1, alphap, rows00, rows01);
        hresize_bn_one_row(src_ptr, xofs, sy1, src_w, dst_w, 1, alphap, rows10, rows11);

        unsigned int b0 = *(beta++);
        unsigned int b1 = *(beta++);

        unsigned int b2 = *(beta++);
        unsigned int b3 = *(beta++);

        int src_y = (int)((dy >> 1) * scale_y + 0.5);
        unsigned char uv0 = 0, uv1 = 0, y00 = 0, y10 = 0, y01 = 0, y11 = 0;
        unsigned char *uv_ptr = src_uv_ptr + src_y * src_w;

        for (dx = 0; dx < dst_w; dx += 2) {
            int src_x = (int)((dx / 2) * scale_x + 0.5) * 2;
            // calculate y
            y00 = static_cast<unsigned char>((*(rows00++) * b0 +
                    *(rows01++) * b1 + (1 << 17)) >> 18);
            y01 = static_cast<unsigned char>((*(rows00++) * b0 +
                    *(rows01++) * b1 + (1 << 17)) >> 18);
            y10 = static_cast<unsigned char>((*(rows10++) * b2 +
                    *(rows11++) * b3 + (1 << 17)) >> 18);
            y11 = static_cast<unsigned char>((*(rows10++) * b2 +
                    *(rows11++) * b3 + (1 << 17)) >> 18);

            // calculate uv, nearest interpolation
            uv0 = is_nv12 ? uv_ptr[src_x] : uv_ptr[src_x + 1];           //u
            uv1 = is_nv12 ? uv_ptr[src_x + 1] : uv_ptr[src_x];           //v

            int vr  = uv1 * 102 - 14216;
            int ub  = 129 * uv0 - 17672;
            int uvg = 8696 - 52 * uv1 - 25 * uv0;

            convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
            ptr_dst0 += channel;
            convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
            ptr_dst0 += channel;
            convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
            ptr_dst1 += channel;
            convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
            ptr_dst1 += channel;
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

int y420sp_to_resize_nearest_to_bgr_common(
        Mat& src,
        Mat& dst,
        bool is_nv12) {

    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    unsigned char *src_uv_ptr = src_ptr + src_w * src_h;

    int buf_size = (dst_w + dst_h) << 3; //(dst_w + dst_h + (dst_w + 1) / 2 + (dst_h + 1) / 2) * sizeof(int)
    int* buf = (int *)malloc(buf_size);

    int* y_xofs = buf;
    int* y_yofs = buf + dst_w;

    int* uv_xofs = y_yofs + dst_h;
    int* uv_yofs = uv_xofs + (dst_w + 1) / 2;

    nearest_cal_offset(y_xofs, dst_w, src_w);
    nearest_cal_offset(y_yofs, dst_h, src_h);

    nearest_cal_offset(uv_xofs, (dst_w + 1) / 2, (src_w + 1) / 2);
    nearest_cal_offset(uv_yofs, (dst_h + 1) / 2, (src_h + 1) / 2);

    int dy = 0, dx= 0, channel = 3;
    for (; dy < dst_h; dy += 2) {
        const unsigned char* src0 = src_ptr + (y_yofs[dy] >> 16) * src_stride;
        const unsigned char* src1 = src_ptr + (y_yofs[dy + 1] >> 16) * src_stride;
        int uv_dy = uv_yofs[dy >> 1] >> 16;
        unsigned char *uv_ptr = src_uv_ptr + uv_dy * src_stride;

        unsigned char * ptr_dst0 = dst_ptr + dst_stride * dy;
        unsigned char * ptr_dst1 = ptr_dst0 + dst_stride;

        unsigned char uv0 = 0, uv1 = 0, y00 = 0, y10 = 0, y01 = 0, y11 = 0;
        for (dx = 0; dx < dst_w; dx += 2) {
            int idx0 = y_xofs[dx] >> 16;
            int idx1 = y_xofs[dx + 1] >> 16;

            int uv_idx = uv_xofs[dx >> 1] >> 15; // (x >> 16) << 1
            // calculate y, nearest interpolation
            y00 = src0[idx0];
            y01 = src0[idx1];

            y10 = src1[idx0];
            y11 = src1[idx1];

            // calculate uv
            uv0 = is_nv12 ? uv_ptr[uv_idx] : uv_ptr[uv_idx + 1];           //u
            uv1 = is_nv12 ? uv_ptr[uv_idx + 1] : uv_ptr[uv_idx];           //v

            // calculate uv coefficients 
            int vr  = uv1 * 102 - 14216;
            int ub  = 129 * uv0 - 17672;
            int uvg = 8696 - 52 * uv1 - 25 * uv0;

            convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
            ptr_dst0 += channel;
            convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
            ptr_dst0 += channel;
            convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
            ptr_dst1 += channel;
            convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
            ptr_dst1 += channel;
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
