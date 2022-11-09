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

#include "modules/core/mat/interface/mat.h"
#include "modules/core/base/include/type_info.h"
#include "modules/core/mat/include/mat_dot_common.h"

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

double dot_product_u8_neon(void* src1, void* src2, int len) {
    unsigned char* src_a = reinterpret_cast<unsigned char*>(src1);
    unsigned char* src_b = reinterpret_cast<unsigned char*>(src2);

    int block = len & (~15); // guarantee the loop size is the multiple of 16
    block = FCV_MIN(block, (1 << 15));
    double res = 0.f;
    int loop = block - 16;

    uint32x4_t v_sum = vdupq_n_u32(0);
    __attribute__((aligned(16))) unsigned int buf[4];

    //deal with 16 pixels for every loop
    int i = 0;
    for (; i <= loop; i += 16) {
        uint8x16_t v_src1 = vld1q_u8(src_a + i);
        uint16x8_t v_src10 = vmovl_u8(vget_low_u8(v_src1));
        uint8x16_t v_src2 = vld1q_u8(src_b + i);
        uint16x8_t v_src20 = vmovl_u8(vget_low_u8(v_src2));

        v_sum = vmlal_u16(v_sum, vget_low_u16(v_src10), vget_low_u16(v_src20));
        v_sum = vmlal_u16(v_sum, vget_high_u16(v_src10), vget_high_u16(v_src20));

        v_src10 = vmovl_u8(vget_high_u8(v_src1));
        v_src20 = vmovl_u8(vget_high_u8(v_src2));
        v_sum = vmlal_u16(v_sum, vget_low_u16(v_src10), vget_low_u16(v_src20));
        v_sum = vmlal_u16(v_sum, vget_high_u16(v_src10), vget_high_u16(v_src20));
    }

    for (; i <= block - 8; i += 8) {
        uint8x8_t v_src1 = vld1_u8(src_a + i);
        uint8x8_t v_src2 = vld1_u8(src_b + i);
        uint16x8_t v_src10 = vmovl_u8(v_src1);
        uint16x8_t v_src20 = vmovl_u8(v_src2);

        v_sum = vmlal_u16(v_sum, vget_low_u16(v_src10), vget_low_u16(v_src20));
        v_sum = vmlal_u16(v_sum, vget_high_u16(v_src10), vget_high_u16(v_src20));
    }

    vst1q_u32(buf, v_sum);

    res += buf[0] + buf[1] + buf[2] + buf[3];

    //deal with remaining pixels
    src_a += block;
    src_b += block;

    return res + dot_product(src_a, src_b, len - block);
}

double dot_neon(const Mat& ma, const Mat& mb) {
    double result = 0.;

    TypeInfo type_info;
    int status = get_type_info(ma.type(), type_info);

    if (status != 0) {
        LOG_ERR("The mat type is not supported!");
        return result;
    }

    int len = ma.width() * ma.height() * ma.channels();
    void* ma_data = ma.data();
    void* mb_data = mb.data();

    switch (type_info.data_type) {
    case DataType::UINT8:
        result = dot_product_u8_neon(ma_data, mb_data, len);
        break;
    case DataType::UINT16:
        result = dot_product<unsigned short>(reinterpret_cast<unsigned short*>(ma_data),
                reinterpret_cast<unsigned short*>(mb_data), len);
        break;
    case DataType::SINT32:
        result = dot_product<int>(reinterpret_cast<int*>(ma_data),
                reinterpret_cast<int*>(mb_data), len);
        break;
    case DataType::F32:
        result =  dot_product<float>(reinterpret_cast<float*>(ma_data),
                reinterpret_cast<float*>(mb_data), len);
        break;
    default:
        LOG_ERR("The src type is not supported!");
        break;
    };

    return result;
}

G_FCV_NAMESPACE1_END()
