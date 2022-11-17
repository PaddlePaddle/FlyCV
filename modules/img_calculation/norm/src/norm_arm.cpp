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

#include "modules/img_calculation/norm/include/norm_arm.h"

#include <cmath>
#include <arm_neon.h>
#include <map>

#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double norm_l1_neon_u8(const unsigned char* data, int n) {
    int sum = 0;
    int i = 0;
    int n_align4 = n & (~31); //32 pixels each loop

    uint32x4_t v_sum = vdupq_n_u32(0);
    uint8x8_t v0_u8, v1_u8, v2_u8, v3_u8;
    for(; i < n_align4; i += 32) {
        v0_u8 = vld1_u8(data);
        v1_u8 = vld1_u8(data + 8);
        uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
        v2_u8 = vld1_u8(data + 16);
        v3_u8 = vld1_u8(data + 24);
        uint16x8_t vtmp1_u16 = vaddl_u8(v2_u8, v3_u8);
        v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
        v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));

        data += 32;
    }

    //simd process, 16 pixels each loop
    for (; i <= n - 16; i += 16) {
        v0_u8 = vld1_u8(data);
        v1_u8 = vld1_u8(data + 8);
        uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
        uint32x4_t vtmp0_u32 = vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16));
        v_sum = vaddq_u32(v_sum, vtmp0_u32);

        data += 16;
    }
    //remain pixels process
    for(; i < n; i++) {
        unsigned char v = *(data++);
        sum += (double)FCV_ABS(v);
    }

    sum += v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
    return (double)sum;
}

double norm_l2_neon_u8(const unsigned char* data, int n) {
    double sum = 0.f;
    int i = 0;
    int n_align16 = n & (~15); //16 pixels each loop
    n_align16 = FCV_MIN(n_align16, (2 << 15));

    uint32x4_t v_sum = vdupq_n_u32(0);
    uint8x8_t v0_u8, v1_u8;
    for(; i < n_align16; i += 16) {
        v0_u8 = vld1_u8(data);
        v1_u8 = vld1_u8(data + 8);

        uint16x8_t vtmp0_u16 = vmull_u8(v0_u8, v0_u8);
        uint16x8_t vtmp1_u16 = vmull_u8(v1_u8, v1_u8);

        v_sum = vaddq_u32(v_sum, vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
        v_sum = vaddq_u32(v_sum, vaddl_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));

        data += 16;
    }

    for(; i <= n - 8; i += 8) {
        v0_u8 = vld1_u8(data);

        uint16x8_t vtmp0_u16 = vmull_u8(v0_u8, v0_u8);
        uint32x4_t vsum_u32 = vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16));
        sum += (vsum_u32[0] + vsum_u32[1] + vsum_u32[2] + vsum_u32[3]);
        data += 8;
    }

    //remain pixels process
    for(; i < n; i++) {
        unsigned char v = *(data++);
        sum += (double)v * v;
    }

    sum += ((double)v_sum[0] + (double)v_sum[1] + (double)v_sum[2] + (double)v_sum[3]);
    return std::sqrt(sum);
}

double norm_inf_neon_u8(const unsigned char* data, int n) {
    unsigned char max = 0;
    int i = 0;

    int n_align16 = n & (~31); //16 pixels each loop
    uint8x8_t v_res = vdup_n_u8(0);

    uint8x16_t v0_u8, v1_u8;
    for(; i < n_align16; i += 32) {
        v0_u8 = vld1q_u8(data);
        v1_u8 = vld1q_u8(data + 16);

        uint8x16_t vmax_v0v1 = vmaxq_u8(v0_u8, v1_u8);
        uint8x8_t vmax = vmax_u8(vget_low_u8(vmax_v0v1), vget_high_u8(vmax_v0v1));
        v_res = vmax_u8(v_res, vmax);
        data += 32;
    }

    for (; i <= n - 16; i += 16) {
        v0_u8 = vld1q_u8(data);
        uint8x8_t vmax = vmax_u8(vget_low_u8(v0_u8), vget_high_u8(v0_u8));
        v_res = vmax_u8(v_res, vmax);
        data += 16;
    }

    unsigned char max0 = FCV_MAX(v_res[0], v_res[1]);
    unsigned char max1 = FCV_MAX(v_res[2], v_res[3]);
    unsigned char max2 = FCV_MAX(v_res[4], v_res[5]);
    unsigned char max3 = FCV_MAX(v_res[6], v_res[7]);

    max1 = FCV_MAX(max0, max1);
    max2 = FCV_MAX(max2, max3);
    max  = FCV_MAX(max1, max2);

    for(; i < n; i++) {
        unsigned char v = *(data++);
        max = FCV_MAX(max, v);
    }

    return (double)max;
}

typedef double (*NormFuncNeon)(const unsigned char*, int);

static NormFuncNeon get_norm_func(NormTypes norm_type) {
    static std::map<NormTypes, NormFuncNeon> norm_funcs = {
        {NormTypes::NORM_L1, norm_l1_neon_u8},
        {NormTypes::NORM_L2, norm_l2_neon_u8},
        {NormTypes::NORM_INF, norm_inf_neon_u8}
    };

    if (norm_funcs.find(norm_type) != norm_funcs.end()) {
        return norm_funcs[norm_type];
    } else {
        return nullptr;
    }
}

int norm_neon(Mat& src, NormTypes norm_type, double& result) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    // only support uint8 neon optimization currently.
    if (status != 0 || type_info.data_type != DataType::UINT8) {
        return -1;
    }

    int cnt = src.height() * src.stride();

    // Maximum Processing 4K image
    if (cnt > 12000000) {
        return -1;
    }

    NormFuncNeon norm_func = get_norm_func(norm_type);

    if (norm_func == nullptr) {
        return -1;
    }

    result = norm_func(reinterpret_cast<unsigned char*>(src.data()), cnt);

    return 0;
}

G_FCV_NAMESPACE1_END()
