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

#include <cmath>
#include <stdlib.h>

#include "modules/core/base/include/utils.h"
#include "modules/core/base/include/type_info.h"
#include "modules/img_calculation/mean/include/mean_arm.h"
#include "modules/img_calculation/mean/include/mean_common.h"

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

typedef int (*SumNeonFunc)(const void*, double*, int, int);
typedef int (*SumMaskNeonFunc)(const void*,
        const unsigned char* mask, double*, int, int);
typedef int (*SumRectNeonFunc)(const void*, int, double*,
        int, int, int, int, int);
typedef int (*SumSqrNeonFunc)(const void*, double*, double*, int, int);

template<typename T>
static int sum_neon(
        const T* src0,
        double* dst,
        int len,
        int cn) {
    const T* src = src0;
    int k = cn % 4;
    if (k == 1) {
        double s0 = 0;
        int i = 0;
        for (; i <= len - 4; i += 4, src += cn * 4) {
            s0 += src[0] + src[cn] + src[cn * 2] + src[cn * 3];
        }
        for (; i < len; i++, src += cn) {
            s0 += src[0];
        }
        dst[0] = s0;
    } else if (k == 2) {
        double s0 = 0;
        double s1 = 0;
        for (int i = 0; i < len; i++, src += cn) {
            s0 += src[0];
            s1 += src[1];
        }
        dst[0] = s0;
        dst[1] = s1;
    } else if (k == 3) {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        for (int i = 0 ; i < len; i++, src += cn) {
            s0 += src[0];
            s1 += src[1];
            s2 += src[2];
        }
        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
    } else {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        double s3 = 0;
        for (int i = 0; i < len; i++, src += cn) {
            s0 += src[0]; s1 += src[1];
            s2 += src[2]; s3 += src[3];
        }
        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
        dst[3] = s3;
    }
    return len;
}

// neon intrinsic process
static int sum_neon_u8(
        const void* src,
        double* dst,
        int len,
        int cn) {
    const unsigned char* src_ptr = static_cast<const unsigned char*>(src);
    uint32x4_t vzero = vdupq_n_u32(0);
    int k = cn % 4;
    if (k == 1) {
        double s0 = 0;

        int i = 0;
        uint8x8_t v0_u8, v1_u8, v2_u8, v3_u8;
        uint32x4_t v_sum = vzero;
        //simd process, 32 pixels each loop
        for (; i <= len - 32; i += 32, src_ptr += 32) {
            v0_u8 = vld1_u8(src_ptr);
            v1_u8 = vld1_u8(src_ptr + 8);
            v2_u8 = vld1_u8(src_ptr + 16);
            v3_u8 = vld1_u8(src_ptr + 24);

            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
            uint16x8_t vtmp1_u16 = vaddl_u8(v2_u8, v3_u8);
            v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp0_u16),
                    vget_high_u16(vtmp0_u16)));
            v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp1_u16),
                    vget_high_u16(vtmp1_u16)));
        }

        //simd process, 16 pixels each loop
        for (; i <= len - 16; i += 16, src_ptr += 16) {
            v0_u8 = vld1_u8(src_ptr);
            v1_u8 = vld1_u8(src_ptr + 8);
            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
            uint32x4_t vtmp0_u32 = vaddl_u16(vget_low_u16(vtmp0_u16),
                    vget_high_u16(vtmp0_u16));
            v_sum = vaddq_u32(v_sum, vtmp0_u32);
        }

        //remain pixels process, 1 piex each loop
        for ( ; i < len; i++, src_ptr += 1) {
            s0 += src_ptr[0];
        }

        s0 = s0 + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
        dst[0] = s0;
    } else if (k == 2) {
        double s0 = 0;
        double s1 = 0;
        uint8x8x2_t v0_u8, v1_u8;
        uint32x4_t v0_sum = vzero;
        uint32x4_t v1_sum = vzero;
        int i = 0;

        //simd process, 32 pixels of two channels each loop
        for (; i <= len - 16; i += 16, src_ptr += 32) {
            v0_u8 = vld2_u8(src_ptr);
            v1_u8 = vld2_u8(src_ptr + 16);

            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
            uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);

            v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                    vget_high_u16(vtmp0_u16)));
            v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                    vget_high_u16(vtmp1_u16)));
        }

        //remain pixels process, 2 piexs of two channels each loop
        for (; i < len; i += 2, src_ptr += 2) {
            s0 += src_ptr[0];
            s1 += src_ptr[1];
        }

        dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
        dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
    } else if (k == 3) {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        uint8x8x3_t v0_u8, v1_u8;
        uint32x4_t v0_sum = vzero;
        uint32x4_t v1_sum = vzero;
        uint32x4_t v2_sum = vzero;
        int i = 0;
        //simd process, 48 pixels of three channels each loop
        for (; i <= len - 16; i += 16, src_ptr += 48) {
            v0_u8 = vld3_u8(src_ptr);
            v1_u8 = vld3_u8(src_ptr + 24);

            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
            uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
            uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);

            v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                    vget_high_u16(vtmp0_u16)));
            v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                    vget_high_u16(vtmp1_u16)));
            v2_sum = vaddq_u32(v2_sum, vaddl_u16(vget_low_u16(vtmp2_u16),
                    vget_high_u16(vtmp2_u16)));
        }
        //remain pixels process, 3 piexs of three channels each loop
        for (; i < len; i++, src_ptr += 3) {
            s0 += src_ptr[0];
            s1 += src_ptr[1];
            s2 += src_ptr[2];
        }
        dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
        dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
        dst[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];
    } else {
        int i = 0;
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        double s3 = 0;
        uint8x8x4_t v0_u8, v1_u8;
        uint32x4_t v0_sum = vzero;
        uint32x4_t v1_sum = vzero;
        uint32x4_t v2_sum = vzero;
        uint32x4_t v3_sum = vzero;

        //simd process, 64 pixels of four channels each loop
        for (; i <= len - 16; i += 16, src_ptr += 64) {
            v0_u8 = vld4_u8(src_ptr);
            v1_u8 = vld4_u8(src_ptr + 32);

            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
            uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
            uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);
            uint16x8_t vtmp3_u16 = vaddl_u8(v0_u8.val[3], v1_u8.val[3]);

            v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                    vget_high_u16(vtmp0_u16)));
            v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                    vget_high_u16(vtmp1_u16)));
            v2_sum = vaddq_u32(v2_sum, vaddl_u16(vget_low_u16(vtmp2_u16),
                    vget_high_u16(vtmp2_u16)));
            v3_sum = vaddq_u32(v3_sum, vaddl_u16(vget_low_u16(vtmp3_u16),
                    vget_high_u16(vtmp3_u16)));
        }

        //remain pixels process, 4 piexs of four channels each loop
        for (; i < len; i++, src_ptr += 4) {
            s0 += src_ptr[0];
            s1 += src_ptr[1];
            s2 += src_ptr[2];
            s3 += src_ptr[3];
        }
        dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
        dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
        dst[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];
        dst[3] = s3 + v3_sum[0] + v3_sum[1] + v3_sum[2] + v3_sum[3];
    }

    return len;
}

static int sum_neon_u16(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_neon(static_cast<const unsigned short*>(src), dst, len, cn);
}

static int sum_neon_s32(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_neon(static_cast<const int*>(src), dst, len, cn);
}

static int sum_neon_f32(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_neon(static_cast<const float*>(src), dst, len, cn);
}

template<typename T>
static int sum_mask_neon(
        const T* src0,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    const T* src = src0;
    if (!mask) {
        int k = cn % 4;
        if (k == 1) {
            double s0 = 0;
            int i = 0;
            for (; i <= len - 4; i += 4, src += cn * 4) {
                s0 += src[0] + src[cn] + src[cn * 2] + src[cn * 3];
            }
            for (; i < len; i++, src += cn) {
                s0 += src[0];
            }
            dst[0] = s0;
        } else if (k == 2) {
            double s0 = 0;
            double s1 = 0;
            for (int i = 0; i < len; i++, src += cn) {
                s0 += src[0];
                s1 += src[1];
            }
            dst[0] = s0;
            dst[1] = s1;
        } else if (k == 3) {
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            for (int i = 0 ; i < len; i++, src += cn) {
                s0 += src[0];
                s1 += src[1];
                s2 += src[2];
            }
            dst[0] = s0;
            dst[1] = s1;
            dst[2] = s2;
        } else {
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            double s3 = 0;
            for (int i = 0; i < len; i++, src += cn) {
                s0 += src[0]; s1 += src[1];
                s2 += src[2]; s3 += src[3];
            }
            dst[0] = s0;
            dst[1] = s1;
            dst[2] = s2;
            dst[3] = s3;
        }
        return len;
    }

    int i, nzm = 0;
    if (cn == 1) {
        double s = 0;
        for (i = 0; i < len; i++) {
            if (mask[i]) {
                s += src[i];
                nzm++;
            }
        }
        dst[0] = s;
    } else if (cn == 3) {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        for (i = 0; i < len; i++, src += 3) {
            if (mask[i]) {
                s0 += src[0];
                s1 += src[1];
                s2 += src[2];
                nzm++;
            }
        }
        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
    } else {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        double s3 = 0;
        for (int i = 0; i < len; i++, src += cn) {
            if(mask[i]) {
                s0 += src[0]; s1 += src[1];
                s2 += src[2]; s3 += src[3];
                nzm++;
            }
        }
        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
        dst[3] = s3;
    }
    return nzm;
}

// neon intrinsic process
static int sum_mask_neon_u8(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    const unsigned char* src_ptr = static_cast<const unsigned char*>(src);
    uint32x4_t vzero = vdupq_n_u32(0);
    if (!mask) {
        int k = cn % 4;
        if (k == 1) {
            double s0 = 0;

            int i = 0;
            uint8x8_t v0_u8, v1_u8, v2_u8, v3_u8;
            uint32x4_t v_sum = vzero;
            //simd process, 32 pixels each loop
            for (; i <= len - 32; i += 32, src_ptr += 32) {
                v0_u8 = vld1_u8(src_ptr);
                v1_u8 = vld1_u8(src_ptr + 8);
                v2_u8 = vld1_u8(src_ptr + 16);
                v3_u8 = vld1_u8(src_ptr + 24);

                uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
                uint16x8_t vtmp1_u16 = vaddl_u8(v2_u8, v3_u8);
                v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp0_u16),
                        vget_high_u16(vtmp0_u16)));
                v_sum = vaddw_u16(v_sum, vpadd_u16(vget_low_u16(vtmp1_u16),
                        vget_high_u16(vtmp1_u16)));
            }

            //simd process, 16 pixels each loop
            for (; i <= len - 16; i += 16, src_ptr += 16) {
                v0_u8 = vld1_u8(src_ptr);
                v1_u8 = vld1_u8(src_ptr + 8);
                uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
                uint32x4_t vtmp0_u32 = vaddl_u16(vget_low_u16(vtmp0_u16),
                        vget_high_u16(vtmp0_u16));
                v_sum = vaddq_u32(v_sum, vtmp0_u32);
            }

            //remain pixels process, 1 piex each loop
            for ( ; i < len; i++, src_ptr += 1) {
                s0 += src_ptr[0];
            }

            s0 = s0 + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
            dst[0] = s0;
        } else if (k == 2) {
            double s0 = 0;
            double s1 = 0;
            uint8x8x2_t v0_u8, v1_u8;
            uint32x4_t v0_sum = vzero;
            uint32x4_t v1_sum = vzero;
            int i = 0;

            //simd process, 32 pixels of two channels each loop
            for (; i <= len - 16; i += 16, src_ptr += 32) {
                v0_u8 = vld2_u8(src_ptr);
                v1_u8 = vld2_u8(src_ptr + 16);

                uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
                uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);

                v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                        vget_high_u16(vtmp0_u16)));
                v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                        vget_high_u16(vtmp1_u16)));
            }

            //remain pixels process, 2 piexs of two channels each loop
            for (; i < len; i += 2, src_ptr += 2) {
                s0 += src_ptr[0];
                s1 += src_ptr[1];
            }

            dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
            dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
        } else if (k == 3) {
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            uint8x8x3_t v0_u8, v1_u8;
            uint32x4_t v0_sum = vzero;
            uint32x4_t v1_sum = vzero;
            uint32x4_t v2_sum = vzero;
            int i = 0;
            //simd process, 48 pixels of three channels each loop
            for (; i <= len - 16; i += 16, src_ptr += 48) {
                v0_u8 = vld3_u8(src_ptr);
                v1_u8 = vld3_u8(src_ptr + 24);

                uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
                uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
                uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);

                v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                        vget_high_u16(vtmp0_u16)));
                v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                        vget_high_u16(vtmp1_u16)));
                v2_sum = vaddq_u32(v2_sum, vaddl_u16(vget_low_u16(vtmp2_u16),
                        vget_high_u16(vtmp2_u16)));
            }
            //remain pixels process, 3 piexs of three channels each loop
            for (; i < len; i++, src_ptr += 3) {
                s0 += src_ptr[0];
                s1 += src_ptr[1];
                s2 += src_ptr[2];
            }
            dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
            dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
            dst[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];
        } else {
            int i = 0;
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            double s3 = 0;
            uint8x8x4_t v0_u8, v1_u8;
            uint32x4_t v0_sum = vzero;
            uint32x4_t v1_sum = vzero;
            uint32x4_t v2_sum = vzero;
            uint32x4_t v3_sum = vzero;

            //simd process, 64 pixels of four channels each loop
            for (; i <= len - 16; i += 16, src_ptr += 64) {
                v0_u8 = vld4_u8(src_ptr);
                v1_u8 = vld4_u8(src_ptr + 32);

                uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
                uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
                uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);
                uint16x8_t vtmp3_u16 = vaddl_u8(v0_u8.val[3], v1_u8.val[3]);

                v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                        vget_high_u16(vtmp0_u16)));
                v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                        vget_high_u16(vtmp1_u16)));
                v2_sum = vaddq_u32(v2_sum, vaddl_u16(vget_low_u16(vtmp2_u16),
                        vget_high_u16(vtmp2_u16)));
                v3_sum = vaddq_u32(v3_sum, vaddl_u16(vget_low_u16(vtmp3_u16),
                        vget_high_u16(vtmp3_u16)));
            }

            //remain pixels process, 4 piexs of four channels each loop
            for (; i < len; i++, src_ptr += 4) {
                s0 += src_ptr[0];
                s1 += src_ptr[1];
                s2 += src_ptr[2];
                s3 += src_ptr[3];
            }
            dst[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
            dst[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
            dst[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];
            dst[3] = s3 + v3_sum[0] + v3_sum[1] + v3_sum[2] + v3_sum[3];
        }
        return len;
    } else {
        int i = 0;
        int nzm = 0;
        if (cn == 1) {
            int s = dst[0];
            for (i = 0; i < len; i++) {
                if (mask[i]) {
                    s += src_ptr[i];
                    nzm++;
                }
            }
            dst[0] = s;
        } else if (cn == 3) {
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            for (i = 0; i < len; i++, src_ptr += 3) {
                if (mask[i]) {
                    s0 += src_ptr[0];
                    s1 += src_ptr[1];
                    s2 += src_ptr[2];
                    nzm++;
                }
            }
            dst[0] = s0;
            dst[1] = s1;
            dst[2] = s2;
        } else {
            double s0 = 0;
            double s1 = 0;
            double s2 = 0;
            double s3 = 0;
            int i = 0;
            for (; i < len; i++, src_ptr += cn) {
                if(mask[i]) {
                    s0 += src_ptr[0];
                    s1 += src_ptr[1];
                    s2 += src_ptr[2];
                    s3 += src_ptr[3];
                    nzm++;
                }
            }
            dst[0] = s0;
            dst[1] = s1;
            dst[2] = s2;
            dst[3] = s3;
        }

        return nzm;
    }
}

static int sum_mask_neon_u16(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask_neon(static_cast<const unsigned short*>(src),
            mask, dst, len, cn);
}

static int sum_mask_neon_s32(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask_neon(static_cast<const int*>(src), mask, dst, len, cn);
}

static int sum_mask_neon_f32(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask_neon(static_cast<const float*>(src), mask, dst, len, cn);
}

template<typename T>
static int sum_rect(
        const T* src,
        int src_stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    int y_end = y_start + height;

    if (cn == 1) {
        double s0 = 0;
        for (int i = y_start; i < y_end; i++) {
            const T* src_start = src + i * src_stride + x_start;
            int j = 0;

            for (;j <= width - 4; j += 4, src_start += cn * 4) {
                s0 += src_start[0] + src_start[cn] +
                        src_start[cn * 2] + src_start[cn * 3];
            }

            for (; j < width; j++, src_start += cn) {
                s0 += src_start[0];
            }
        }
        dst[0] = s0;
    } else if (cn == 2) {
        double s0 = 0;
        double s1 = 0;
        for (int i = y_start; i < y_end; i++) {
            const T* src_start = src + i * src_stride + (x_start << 1);
            for (int j = 0; j < width; j++, src_start += cn) {
                s0 += src_start[0];
                s1 += src_start[1];
            }
        }
        dst[0] = s0;
        dst[1] = s1;
    } else if (cn == 3) {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        for (int i = y_start; i < y_end; i++) {
            const T* src_start = src + i * src_stride + (x_start * 3);
            for (int j = 0 ; j < width; j++, src_start += cn) {
                s0 += src_start[0];
                s1 += src_start[1];
                s2 += src_start[2];
            }
        }

        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
    } else {
        double s0 = 0;
        double s1 = 0;
        double s2 = 0;
        double s3 = 0;

        for (int i = y_start; i < y_end; i++) {
            const T* src_start = src + i * src_stride + (x_start << 2);
            for (int j = 0; j < width; j++, src_start += cn) {
                s0 += src_start[0];
                s1 += src_start[1];
                s2 += src_start[2];
                s3 += src_start[3];
            }
        }
        dst[0] = s0;
        dst[1] = s1;
        dst[2] = s2;
        dst[3] = s3;
    }

    return 0;
}

static int sum_rect_neon_u8(
        const void* src,
        int stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    return sum_rect(static_cast<const unsigned char*>(src),
            stride, dst, x_start, y_start, width, height, cn);
}

static int sum_rect_neon_u16(
        const void* src,
        int stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    return sum_rect(static_cast<const unsigned short*>(src),
            stride, dst, x_start, y_start, width, height, cn);
}

static int sum_rect_neon_s32(
        const void* src,
        int stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    return sum_rect(static_cast<const int*>(src), stride,
            dst, x_start, y_start, width, height, cn);
}

static int sum_rect_neon_f32(
        const void* src,
        int stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    return sum_rect(static_cast<const float*>(src), stride,
            dst, x_start, y_start, width, height, cn);
}

static int sum_sqr_u8_c1_neon(
        const unsigned char* src_ptr,
        double* sum,
        double* square_sum,
        int block_size,
        int len) {
    int loop_cnt = len / block_size + 1;

    for (int i = 0; i < loop_cnt; ++i) {
        int size = (i + 1) * block_size > len ? len - i * block_size : block_size;

        int count = size / 16;
        uint8x8_t v0_u8;
        uint8x8_t v1_u8;
        uint32x4_t v_sum = vdupq_n_u32(0);
        uint32x4_t sq_sum = vdupq_n_u32(0);

        //simd process, 16 pixels each loop
        for (int j = 0; j < count; ++j, src_ptr += 16) {
            v0_u8 = vld1_u8(src_ptr);
            v1_u8 = vld1_u8(src_ptr + 8);

            uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8, v1_u8);
            uint32x4_t vtmp0_u32 = vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16));
            v_sum = vaddq_u32(v_sum, vtmp0_u32);

            uint16x8_t vmul0_u16 = vmull_u8(v0_u8, v0_u8);
            uint16x8_t vmul1_u16 = vmull_u8(v1_u8, v1_u8);
            sq_sum = vaddq_u32(sq_sum, vaddl_u16(vget_low_u16(vmul0_u16), vget_high_u16(vmul0_u16)));
            sq_sum = vaddq_u32(sq_sum, vaddl_u16(vget_low_u16(vmul1_u16), vget_high_u16(vmul1_u16)));
        }

        for (int j = count * 16; j < size; ++j) {
            *sum = *sum + src_ptr[0];
            *square_sum = *square_sum + src_ptr[0] * src_ptr[0];
            src_ptr++;
        }

        *sum = *sum + v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
        *square_sum = *square_sum + sq_sum[0] + sq_sum[1] + sq_sum[2] + sq_sum[3];
    }

    return len;
}

static int sum_sqr_u8_c2_neon(
        const unsigned char* src_ptr,
        double* sum,
        double* square_sum,
        int block_size,
        int len) {
    double s0 = 0;
    double s1 = 0;
    double sq0 = 0;
    double sq1 = 0;
    uint8x8x2_t v0_u8, v1_u8;
    uint32x4_t vzero = vdupq_n_u32(0);
    uint32x4_t v0_sum = vzero;
    uint32x4_t v1_sum = vzero;
    uint32x4_t sq_sum0 = vzero;
    uint32x4_t sq_sum1 = vzero;
    int i = 0;

    //simd process, 32 pixels of two channels each loop
    for (; i <= block_size - 16; i += 16, src_ptr += 32) {
        v0_u8 = vld2_u8(src_ptr);
        v1_u8 = vld2_u8(src_ptr + 16);

        uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
        uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);

        v0_sum = vaddq_u32(v0_sum, vaddl_u16(vget_low_u16(vtmp0_u16),
                vget_high_u16(vtmp0_u16)));

        v1_sum = vaddq_u32(v1_sum, vaddl_u16(vget_low_u16(vtmp1_u16),
                vget_high_u16(vtmp1_u16)));

        uint16x8_t vmul0_u16 = vmull_u8(v0_u8.val[0], v0_u8.val[0]);
        uint16x8_t vmul1_u16 = vmull_u8(v0_u8.val[1], v0_u8.val[1]);
        uint16x8_t vmul2_u16 = vmull_u8(v1_u8.val[0], v1_u8.val[0]);
        uint16x8_t vmul3_u16 = vmull_u8(v1_u8.val[1], v1_u8.val[1]);

        vaddl_u16(vget_low_u16(vmul0_u16), vget_high_u16(vmul0_u16));

        uint32x4_t vmul02_u32 = vaddq_u32(vaddl_u16(vget_low_u16(vmul0_u16),
                vget_high_u16(vmul0_u16)), vaddl_u16(vget_low_u16(vmul2_u16),
                vget_high_u16(vmul2_u16)));

        uint32x4_t vmul13_u32 = vaddq_u32(vaddl_u16(vget_low_u16(vmul1_u16),
                vget_high_u16(vmul1_u16)), vaddl_u16(vget_low_u16(vmul3_u16),
                vget_high_u16(vmul3_u16)));

        sq_sum0 = vaddq_u32(sq_sum0, vmul02_u32);
        sq_sum1 = vaddq_u32(sq_sum1, vmul13_u32);
    }

    //remain pixels process, 2 piexs of two channels each loop
    for (; i < len; i += 2, src_ptr += 2) {
        s0 += src_ptr[0];
        s1 += src_ptr[1];
        sq0 = src_ptr[0] * src_ptr[0];
        sq1 = src_ptr[1] * src_ptr[1];
    }

    sum[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
    sum[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];

    square_sum[0] = sq0 + sq_sum0[0] + sq_sum0[1] + sq_sum0[2] + sq_sum0[3];
    square_sum[1] = sq1 + sq_sum1[0] + sq_sum1[1] + sq_sum1[2] + sq_sum1[3];

    return len;
}

static int sum_sqr_u8_c3_neon(
        const unsigned char* src_ptr,
        double* sum,
        double* square_sum,
        int block_size,
        int len) {
    double s0 = 0;
    double s1 = 0;
    double s2 = 0;
    double sq0 = 0;
    double sq1 = 0;
    double sq2 = 0;
    uint8x8x3_t v0_u8, v1_u8;
    uint32x4_t vzero = vdupq_n_u32(0);
    uint32x4_t v0_sum = vzero;
    uint32x4_t v1_sum = vzero;
    uint32x4_t v2_sum = vzero;
    uint32x4_t sq_sum0 = vzero;
    uint32x4_t sq_sum1 = vzero;
    uint32x4_t sq_sum2 = vzero;
    int i = 0;
    //simd process, 48 pixels of three channels each loop
    for (; i <= block_size - 16; i += 16, src_ptr += 48) {
        v0_u8 = vld3_u8(src_ptr);
        v1_u8 = vld3_u8(src_ptr + 24);

        uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
        uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
        uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);

        v0_sum = vaddq_u32(v0_sum,
                vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
        v1_sum = vaddq_u32(v1_sum,
                vaddl_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));
        v2_sum = vaddq_u32(v2_sum,
                vaddl_u16(vget_low_u16(vtmp2_u16), vget_high_u16(vtmp2_u16)));

        uint16x8_t vmul0_u16 = vmull_u8(v0_u8.val[0], v0_u8.val[0]);
        uint16x8_t vmul3_u16 = vmull_u8(v1_u8.val[0], v1_u8.val[0]);
        uint32x4_t vmul03_u32 = vaddq_u32(vaddl_u16(vget_low_u16(vmul0_u16),
                vget_high_u16(vmul0_u16)), vaddl_u16(vget_low_u16(vmul3_u16),
                vget_high_u16(vmul3_u16)));

        uint16x8_t vmul1_u16 = vmull_u8(v0_u8.val[1], v0_u8.val[1]);
        uint16x8_t vmul4_u16 = vmull_u8(v1_u8.val[1], v1_u8.val[1]);
        uint32x4_t vmul14_u32 = vaddq_u32(vaddl_u16(vget_low_u16(vmul1_u16),
                vget_high_u16(vmul1_u16)), vaddl_u16(vget_low_u16(vmul4_u16),
                vget_high_u16(vmul4_u16)));

        uint16x8_t vmul2_u16 = vmull_u8(v0_u8.val[2], v0_u8.val[2]);
        uint16x8_t vmul5_u16 = vmull_u8(v1_u8.val[2], v1_u8.val[2]);
        uint32x4_t vmul25_u32 = vaddq_u32(vaddl_u16(vget_low_u16(vmul2_u16),
                vget_high_u16(vmul2_u16)), vaddl_u16(vget_low_u16(vmul5_u16),
                vget_high_u16(vmul5_u16)));

        sq_sum0 = vaddq_u32(sq_sum0, vmul03_u32);
        sq_sum1 = vaddq_u32(sq_sum1, vmul14_u32);
        sq_sum2 = vaddq_u32(sq_sum2, vmul25_u32);
    }

    //remain pixels process, 3 piexs of three channels each loop
    for (; i < len; i++, src_ptr += 3) {
        unsigned char sp0 = src_ptr[0], sp1 = src_ptr[1], sp2 = src_ptr[2];
        s0 += sp0;
        s1 += sp1;
        s2 += sp2;

        sq0 += sp0 * sp0;
        sq1 += sp1 * sp1;
        sq2 += sp2 * sp2;
    }

    sum[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
    sum[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
    sum[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];

    square_sum[0] = sq0 + sq_sum0[0] + sq_sum0[1] + sq_sum0[2] + sq_sum0[3];
    square_sum[1] = sq1 + sq_sum1[0] + sq_sum1[1] + sq_sum1[2] + sq_sum1[3];
    square_sum[2] = sq2 + sq_sum2[0] + sq_sum2[1] + sq_sum2[2] + sq_sum2[3];

    return len;
}

static int sum_sqr_u8_c4_neon(
        const unsigned char* src_ptr,
        double* sum,
        double* square_sum,
        int block_size,
        int len) {
    int i = 0;
    double s0 = 0;
    double s1 = 0;
    double s2 = 0;
    double s3 = 0;
    double sq0 = 0;
    double sq1 = 0;
    double sq2 = 0;
    double sq3 = 0;
    uint8x8x4_t v0_u8, v1_u8;
    uint32x4_t vzero = vdupq_n_u32(0);
    uint32x4_t v0_sum = vzero;
    uint32x4_t v1_sum = vzero;
    uint32x4_t v2_sum = vzero;
    uint32x4_t v3_sum = vzero;
    uint32x4_t sq_sum0 = vzero;
    uint32x4_t sq_sum1 = vzero;
    uint32x4_t sq_sum2 = vzero;
    uint32x4_t sq_sum3 = vzero;

    //simd process, 64 pixels of four channels each loop
    for (; i <= block_size - 16; i += 16, src_ptr += 64) {
        v0_u8 = vld4_u8(src_ptr);
        v1_u8 = vld4_u8(src_ptr + 32);

        uint16x8_t vtmp0_u16 = vaddl_u8(v0_u8.val[0], v1_u8.val[0]);
        uint16x8_t vtmp1_u16 = vaddl_u8(v0_u8.val[1], v1_u8.val[1]);
        uint16x8_t vtmp2_u16 = vaddl_u8(v0_u8.val[2], v1_u8.val[2]);
        uint16x8_t vtmp3_u16 = vaddl_u8(v0_u8.val[3], v1_u8.val[3]);

        v0_sum = vaddq_u32(v0_sum,
                vaddl_u16(vget_low_u16(vtmp0_u16), vget_high_u16(vtmp0_u16)));
        v1_sum = vaddq_u32(v1_sum,
                vaddl_u16(vget_low_u16(vtmp1_u16), vget_high_u16(vtmp1_u16)));
        v2_sum = vaddq_u32(v2_sum,
                vaddl_u16(vget_low_u16(vtmp2_u16), vget_high_u16(vtmp2_u16)));
        v3_sum = vaddq_u32(v3_sum,
                vaddl_u16(vget_low_u16(vtmp3_u16), vget_high_u16(vtmp3_u16)));

        uint16x8_t vmul0_u16 = vmull_u8(v0_u8.val[0], v0_u8.val[0]);
        uint16x8_t vmul1_u16 = vmull_u8(v0_u8.val[1], v0_u8.val[1]);
        uint16x8_t vmul2_u16 = vmull_u8(v0_u8.val[2], v0_u8.val[2]);
        uint16x8_t vmul3_u16 = vmull_u8(v0_u8.val[3], v0_u8.val[3]);
        uint16x8_t vmul4_u16 = vmull_u8(v1_u8.val[0], v1_u8.val[0]);
        uint16x8_t vmul5_u16 = vmull_u8(v1_u8.val[1], v1_u8.val[1]);
        uint16x8_t vmul6_u16 = vmull_u8(v1_u8.val[2], v1_u8.val[2]);
        uint16x8_t vmul7_u16 = vmull_u8(v1_u8.val[3], v1_u8.val[3]);

        uint32x4_t vmul04_u32 = vaddq_u32(vaddl_u16(
                vget_low_u16(vmul0_u16), vget_high_u16(vmul0_u16)),
                vaddl_u16(vget_low_u16(vmul4_u16), vget_high_u16(vmul4_u16)));
        uint32x4_t vmul15_u32 = vaddq_u32(vaddl_u16(
                vget_low_u16(vmul1_u16), vget_high_u16(vmul1_u16)),
                vaddl_u16(vget_low_u16(vmul5_u16), vget_high_u16(vmul5_u16)));
        uint32x4_t vmul26_u32 = vaddq_u32(vaddl_u16(
                vget_low_u16(vmul2_u16), vget_high_u16(vmul2_u16)),
                vaddl_u16(vget_low_u16(vmul6_u16), vget_high_u16(vmul6_u16)));
        uint32x4_t vmul37_u32 = vaddq_u32(vaddl_u16(
                vget_low_u16(vmul3_u16), vget_high_u16(vmul3_u16)),
                vaddl_u16(vget_low_u16(vmul7_u16), vget_high_u16(vmul7_u16)));

        sq_sum0 = vaddq_u32(sq_sum0, vmul04_u32);
        sq_sum1 = vaddq_u32(sq_sum1, vmul15_u32);
        sq_sum2 = vaddq_u32(sq_sum2, vmul26_u32);
        sq_sum3 = vaddq_u32(sq_sum3, vmul37_u32);
    }

    //remain pixels process, 4 piexs of four channels each loop
    for (; i < len; i++, src_ptr += 4) {
        unsigned char sp0 = src_ptr[0];
        unsigned char sp1 = src_ptr[1];
        unsigned char sp2 = src_ptr[2];
        unsigned char sp3 = src_ptr[3];
        s0 += sp0;
        s1 += sp1;
        s2 += sp2;
        s3 += sp3;

        sq0 += sp0 * sp0;
        sq1 += sp1 * sp1;
        sq2 += sp2 * sp2;
        sq3 += sp3 * sp3;
    }

    sum[0] = s0 + v0_sum[0] + v0_sum[1] + v0_sum[2] + v0_sum[3];
    sum[1] = s1 + v1_sum[0] + v1_sum[1] + v1_sum[2] + v1_sum[3];
    sum[2] = s2 + v2_sum[0] + v2_sum[1] + v2_sum[2] + v2_sum[3];
    sum[3] = s3 + v3_sum[0] + v3_sum[1] + v3_sum[2] + v3_sum[3];

    square_sum[0] = sq0 + sq_sum0[0] + sq_sum0[1] + sq_sum0[2] + sq_sum0[3];
    square_sum[1] = sq1 + sq_sum1[0] + sq_sum1[1] + sq_sum1[2] + sq_sum1[3];
    square_sum[2] = sq2 + sq_sum2[0] + sq_sum2[1] + sq_sum2[2] + sq_sum2[3];
    square_sum[3] = sq3 + sq_sum3[0] + sq_sum3[1] + sq_sum3[2] + sq_sum3[3];

    return len;
}

static inline int sum_sqr_u8_neon(
        const unsigned char* src_ptr,
        double* sum,
        double* square_sum,
        int len,
        int cn) {
    int k = cn % 4;
    int block_size = len & (~15); // guarantee the loop size is the multiple of 16
    block_size = FCV_MIN(block_size, 1 << 15);

    int (*func)(const unsigned char*, double*, double*, int, int) = nullptr;

    if (k == 1) {
        func = sum_sqr_u8_c1_neon;
    } else if (k == 2) {
        func = sum_sqr_u8_c2_neon;
    } else if (k == 3) {
        func = sum_sqr_u8_c3_neon;
    } else {
        func = sum_sqr_u8_c4_neon;
    }

    return func(src_ptr, sum, square_sum, block_size, len);
}

static SumNeonFunc get_sum_neon_func(DataType type) {
    static std::map<DataType, SumNeonFunc> funcs = {
        {DataType::UINT8, sum_neon_u8},
        {DataType::UINT16, sum_neon_u16},
        {DataType::SINT32, sum_neon_s32},
        {DataType::F32, sum_neon_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

static SumMaskNeonFunc get_sum_mask_neon_func(DataType type) {
    static std::map<DataType, SumMaskNeonFunc> funcs = {
        {DataType::UINT8, sum_mask_neon_u8},
        {DataType::UINT16, sum_mask_neon_u16},
        {DataType::SINT32, sum_mask_neon_s32},
        {DataType::F32, sum_mask_neon_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

static SumRectNeonFunc get_sum_rect_neon_func(DataType type) {
    static std::map<DataType, SumRectNeonFunc> funcs = {
        {DataType::UINT8, sum_rect_neon_u8},
        {DataType::UINT16, sum_rect_neon_u16},
        {DataType::SINT32, sum_rect_neon_s32},
        {DataType::F32, sum_rect_neon_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

Scalar mean_neon(const Mat& src) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumNeonFunc sum_func = get_sum_neon_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();   //the channel of src
    int len = src.stride() * src.height() / (cn * src.type_byte_size());

    double* sum = (double*)malloc(cn * sizeof(double));
    memset(sum, 0, cn * sizeof(double));

    sum_func(src.data(), sum, len, cn);

    for (int i = 0; i < cn; i++) {
        res.set_val(i, sum[i] / len);
    }

    free(sum);
    return res;
}

Scalar mean_neon(const Mat& src, const Mat& mask) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumMaskNeonFunc sum_func = get_sum_mask_neon_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();   //the channel of src

    double* sum = (double*)malloc(cn * sizeof(double));
    memset(sum, 0, cn * sizeof(double));
    int len = src.stride() * src.height() / (cn * src.type_byte_size());

    unsigned char* mask_data = (unsigned char *)mask.data();

    //return the number of the total size of input
    int nz = sum_func(src.data(), mask_data, sum, len, cn);

    if (nz == 0) {
        free(sum);
        return res;
    }

    for (int i = 0; i < cn; i++) {
        res.set_val(i, sum[i] / nz);
    }

    free(sum);
    return res;
}

Scalar mean_neon(const Mat& src, const Rect& rect) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumRectNeonFunc sum_func = get_sum_rect_neon_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();
    double* sum = (double*)malloc(sizeof(double) * cn);
    memset(sum, 0, cn * sizeof(double));

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();
    int len = w * h;
    int stride = src.stride() / src.type_byte_size();

    sum_func(src.data(), stride, sum, x, y, w, h, cn);

    for (int i = 0; i < cn; i++) {
        res.set_val(i, sum[i] / len);
    }

    free(sum);
    return res;
}

void mean_stddev_neon(const Mat& src, Mat& mean, Mat& stddev) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return;
    }

    int cn = src.channels();   //the channel of src
    FCVImageType res_type;

    if (cn == 1) {
        res_type = FCVImageType::GRAY_F64;
    } else if (cn == 3) {
        res_type = FCVImageType::PKG_BGR_F64;
    } else if (cn == 4) {
        res_type = FCVImageType::PKG_BGRA_F64;
    } else {
        LOG_ERR("Unsupport mat channel, the channel should be 1, 3, 4!");
        return;
    }

    mean = Mat(1, 1, res_type);
    stddev = Mat(1, 1, res_type);

    double* sum = (double*)malloc((cn * sizeof(double)) << 1);
    memset(sum, 0, (cn * sizeof(double)) << 1);

    double* sum_sqr = sum + cn;
    int len = src.stride() * src.height() / (cn * src.type_byte_size());
    int nz = 0;

    switch (type_info.data_type) {
    case DataType::UINT8:
        nz = sum_sqr_u8_neon(reinterpret_cast<unsigned char*>(src.data()),
                sum, sum_sqr, len, cn);
        break;
    case DataType::UINT16:
        nz = sum_sqr_u16_common(src.data(), sum, sum_sqr, len, cn);
        break;
    case DataType::SINT32:
        nz = sum_sqr_s32_common(src.data(), sum, sum_sqr, len, cn);
        break;
    case DataType::F32:
        nz = sum_sqr_f32_common(src.data(), sum, sum_sqr, len, cn);
        break;
    default:
        break;
    };

    double* mean_data = (double*)mean.data();

    for (int i = 0; i < cn; i++) {
       mean_data[i] = sum[i] / nz;
    }

    double* stddev_data = (double*)stddev.data();

    for (int i = 0; i < cn; i++) {
       stddev_data[i] = std::sqrt(sum_sqr[i] / nz - mean_data[i] * mean_data[i]);
    }

    if (sum != nullptr) {
        free(sum);
        sum = nullptr;
    }
}

G_FCV_NAMESPACE1_END()
