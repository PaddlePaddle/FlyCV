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

void init_table_2d_coeff_u8_neon(short *tab, int tabsz);
void init_table_2d_coeff_f32_neon(float *tab, int tabsz);

void remap_linear_const_u8_neon(
        int start_height,
        int end_height,
        const int width,
        const Mat& src,
        unsigned char* dst,
        const int dst_stride,
        const short *src_xy,
        const short *coeffs,
        short *tab,
        const Scalar& border_value);

void remap_linear_const_f32_neon(
        int start_height,
        int end_height,
        const int width,
        const Mat& src,
        float* dst,
        const int dst_stride,
        const short *src_xy,
        const short *coeffs,
        float *tab,
        const Scalar& border_value);

int remap_neon(
        const Mat& src,
        Mat& dst,
        const Mat& map,
        const Mat& coeffs,
        InterpolationType inter_type,
        BorderTypes border_method,
        const Scalar border_value);

G_FCV_NAMESPACE1_END()
