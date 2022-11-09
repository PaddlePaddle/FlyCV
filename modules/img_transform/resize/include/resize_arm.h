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
#include "modules/core/base/include/macro_utils.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static inline uint8x8x2_t v_bilinear_horizonal_elems(
        const unsigned char* tab,
        const int* idx) {
    unsigned char FCV_ALIGNED(16) elems[16] = {
        tab[idx[0]],
        tab[idx[0] + 1],
        tab[idx[1]],
        tab[idx[1] + 1],
        tab[idx[2]],
        tab[idx[2] + 1],
        tab[idx[3]],
        tab[idx[3] + 1],
        tab[idx[4]],
        tab[idx[4] + 1],
        tab[idx[5]],
        tab[idx[5] + 1],
        tab[idx[6]],
        tab[idx[6] + 1],
        tab[idx[7]],
        tab[idx[7] + 1]
    };

    return uint8x8x2_t(vld2_u8(elems));
}

void horizontal_resize_bn(
        const unsigned char *src0,
        const unsigned char *src1,
        unsigned short *rows0,
        unsigned short *rows1,
        const unsigned short *alphap,
        int* xofs,
        int dst_w);

int resize_bilinear_neon(Mat& src, Mat& dst);

int resize_cubic_neon(Mat& src, Mat& dst);

int resize_area_neon(Mat& src, Mat& dst);

G_FCV_NAMESPACE1_END()
