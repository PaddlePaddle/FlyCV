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
#include "modules/img_transform/color_convert/include/color_convert_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

struct YuvConstants {
    unsigned char UVCoeffBR[16];
    unsigned char UVCoeffG[16];
    unsigned short UVBiasBGR[24];
    unsigned char UVcoeffY[8];
};

#define MAKEYUVCONSTANTS(name, VTY, UTB, UTG, VTG, VTR, BTB, BTG, BTR)    \
    const struct YuvConstants(Yuv##name##Constants) = {      \
        {UTB, VTR, UTB, VTR, UTB, VTR, UTB, VTR, UTB, VTR, UTB, VTR, UTB, VTR, UTB, VTR},  \
        {UTG, VTG, UTG, VTG, UTG, VTG, UTG, VTG, UTG, VTG, UTG, VTG, UTG, VTG, UTG, VTG},  \
        {BTB, BTG, BTR, BTB, BTG, BTR, BTB, BTG, BTR, BTB, BTG, BTR, BTB, BTG, BTR, BTB,  \
        BTG, BTR, BTB, BTG, BTR, BTB, BTG, BTR}, \
        {VTY, VTY, VTY, VTY, VTY, VTY, VTY, VTY}}; \

#define VTY 149  /* round(1.164 * 64) */
#define UTB 129 /* round(2.017 * 64) */
#define UTG 25  /* round(0.391 * 64) */
#define VTG 52  /* round(0.813 * 64) */
#define VTR 102 /* round(1.596 * 64) */
#define BTB 17672
#define BTG 8696
#define BTR 14216

MAKEYUVCONSTANTS(I601, VTY, UTB, UTG, VTG, VTR, BTB, BTG, BTR)

int cvt_color_neon(const Mat& src, Mat& dst, ColorConvertType cvt_type);

int cvt_color_neon(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type);

G_FCV_NAMESPACE1_END()
