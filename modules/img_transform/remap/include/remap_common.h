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

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define BLOCK_SIZE_HEIGHT  (32)
#define BLOCK_SIZE_WIDTH  (128)
#define AREA_SZ           (BLOCK_SIZE_HEIGHT * BLOCK_SIZE_WIDTH)
#define AB_SCALE          (1024)

#define WARP_SCALE_BITS      (10)
#define WARP_SCALE_BITS_HALF (5)
#define WARP_INTER_TAB_SIZE  (1 << WARP_SCALE_BITS_HALF) //32
#define WARP_SCALE_REMAP_BITS (15) //the fixed-point scale for remap interpolation calculate  

int inverse_matrix_2x3_float(const double *src_mat, double *dst_mat);
void init_table_2d_coeff_u8(short *tab, int tabsz);
void init_table_2d_coeff_f32(float *tab, int tabsz);

void remap_linear_u8_const(
        const int height,
        const int width,
        const unsigned char *src,
        const int src_width,
        const int src_height,
        const int src_stride,
        unsigned char *dst,
        const int dst_stride,
        const short *src_xy,
        const short *coeffs,
        const int channel,
        const short *tab,
        const Scalar border_value);

void remap_linear_f32_const(
        const int height,
        const int width,
        const float *src,
        const int src_width,
        const int src_height,
        const int src_stride,
        float *dst,
        const int dst_stride,
        const short *src_xy,
        const short *coeffs,
        const int channel,
        const float *tab,
        const Scalar border_value);

int remap_common(
        const Mat& src,
        Mat& dst,
        const Mat& map,
        const Mat& coeffs,
        InterpolationType inter_type,
        BorderType border_method,
        const Scalar border_value);

G_FCV_NAMESPACE1_END()
