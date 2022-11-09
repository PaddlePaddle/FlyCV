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

#include "modules/img_transform/resize/interface/resize.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define INTER_RESIZE_COEF_SCALE   (2048)

void hresize_bn_one_row(
        const unsigned char* src_ptr,
        int* xofs,
        int sy,
        int stride,
        int w,
        int c,
        unsigned short* alphap,
        unsigned short* rows0,
        unsigned short* rows1);

void get_resize_bilinear_buf(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        int** buf);

void get_resize_bilinear_buf_c1(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        int** buf);

void get_resize_cubic_buf(
        int src_w,
        int src_h,
        int dst_w,
        int dst_h,
        int c,
        int** buf);

void nearest_cal_offset(
        int* offset,
        const int dst_size,
        const int src_size);

void get_resize_area_buf(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        double inv_scale_x,
        double inv_scale_y,
        int** buf);

void get_resize_area_buf_c1(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        double inv_scale_x,
        double inv_scale_y,
        int** buf);

int resize_bilinear_common(Mat& src, Mat& dst);

int resize_cubic_common(Mat& src, Mat& dst);

int resize_area_common(Mat& src, Mat& dst);

G_FCV_NAMESPACE1_END()
