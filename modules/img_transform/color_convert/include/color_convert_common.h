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

#pragma once

#include "modules/core/mat/interface/mat.h"
#include "modules/img_transform/color_convert/interface/color_convert.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define Q                   (8)

#define YR                  (66)    //(0.257  * 256)
#define YG                  (129)   //(0.504  * 256)
#define YB                  (25)    //(0.098  * 256)
#define VR                  (112)   //(0.439  * 256)
#define VG                  (-94)   //(-0.368 * 256)
#define VB                  (-18)   //(-0.071 * 256)
#define UR                  (-38)   //(-0.148 * 256)
#define UG                  (-74)   //(-0.291 * 256)
#define UB                  (112)   //(0.439  * 256)
#define UVC                 (0x8000)//(128  * 256)
#define YC                  (0x1080)//(16.5 * 256)

#define R_RATION            (77) //(0.299 * 256)
#define G_RATION            (150)//(0.587 * 256)
#define B_RATION            (29) //(0.114 * 256)

#define CHECK_CVT_SIZE(param)                                        \
    do {                                                               \
        if (!(param)) {                                                  \
            LOG_ERR("src or dst size is not right!\n");                    \
            return;                                                        \
        }                                                                \
    } while (0)

int get_cvt_color_dst_mat_type(ColorConvertType type);

inline void convet_yuv_to_one_col(
        int y0,
        int ub,
        int uvg,
        int vr,
        unsigned char *dst,
        int b_idx,
        int r_idx,
        int channel) {
    int res_y00 = (149 * y0) >> 1;
    dst[b_idx] = fcv_cast_u8(((res_y00 + ub) >> 1) >> 5);
    dst[1] = fcv_cast_u8(((res_y00 + uvg) >> 1) >> 5);
    dst[r_idx] = fcv_cast_u8(((res_y00 + vr) >> 1) >> 5);
    if(channel == 4) {
        dst[3] = 255;
    }
}

void convet_yuv_to_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        const unsigned char *vu,
        int src_w,
        int inter_start,
        int src_stride,
        int dst_stride,
        bool is_nv12,
        int b_idx,
        int r_idx,
        int channel);

void convet_yuv420_to_bgr_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        const unsigned char *u,
        const unsigned char *v,
        int src_w,
        int src_stride,
        int dst_stride,
        int inter_start,
        int channel);

void convert_to_yuv_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        unsigned char *ptr_vu,
        int src_w,
        int s_stride,
        int iter_start,
        bool is_nv12,
        int b_id,
        int r_id,
        int channel);

void convertTo565(const unsigned short b,
        const unsigned short g,
        const unsigned short r,
        unsigned char *dst);

int cvt_color_c(const Mat& src, Mat& dst, ColorConvertType cvt_type);

int cvt_color_c(
        const Mat& src,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type);

G_FCV_NAMESPACE1_END()
