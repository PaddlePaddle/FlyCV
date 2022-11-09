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

#include "modules/img_transform/remap/include/remap_common.h"
#include "modules/core/base/interface/basic_types.h"
#include "modules/core/basic_math/interface/basic_math.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/utils.h"

#include <stdlib.h>

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void init_table_2d_coeff_u8_neon(short *tab, int tabsz) {
    if (nullptr == tab) {
        LOG_ERR("Mat is empty!");
        return;
    }

    float coeffs[tabsz * 2];
    float scale = 1.0f / tabsz;

    for (int i = 0; i < tabsz; i += 2) {
        float val0 = scale * i;
        float val1 = scale + val0;
        float32x4_t v_tab = {1.f - val0, val0, 1.f - val1, val1};
        vst1q_f32(&coeffs[i << 1], v_tab);
    }

    float32x4_t v_scale_remap = vdupq_n_f32(1 << WARP_SCALE_REMAP_BITS);

    for (int j = 0; j < tabsz; j++) {
        float32x4_t v_uy0 = vdupq_n_f32(coeffs[(j << 1) + 0]);
        float32x4_t v_uy1 = vdupq_n_f32(coeffs[(j << 1) + 1]);

        v_uy0 = vmulq_f32(v_uy0, v_scale_remap);
        v_uy1 = vmulq_f32(v_uy1, v_scale_remap);

        for (int k = 0; k < tabsz; k += 4, tab += 16) {
            float32x4x2_t v_tab_k = vld2q_f32(&coeffs[k << 1]);

            float32x4_t v_tab0_f = vmulq_f32(v_uy0, v_tab_k.val[0]);
            float32x4_t v_tab1_f = vmulq_f32(v_uy0, v_tab_k.val[1]);
            float32x4_t v_tab2_f = vmulq_f32(v_uy1, v_tab_k.val[0]);
            float32x4_t v_tab3_f = vmulq_f32(v_uy1, v_tab_k.val[1]);

            int32x4_t v_tab0_l = VCVTAQ_S32_F32(v_tab0_f);
            int32x4_t v_tab1_l = VCVTAQ_S32_F32(v_tab1_f);
            int32x4_t v_tab2_l = VCVTAQ_S32_F32(v_tab2_f);
            int32x4_t v_tab3_l = VCVTAQ_S32_F32(v_tab3_f);

            int16x4x4_t v_tab;
            v_tab.val[0] = vqmovn_s32(v_tab0_l);
            v_tab.val[1] = vqmovn_s32(v_tab1_l);
            v_tab.val[2] = vqmovn_s32(v_tab2_l);
            v_tab.val[3] = vqmovn_s32(v_tab3_l);

            vst4_s16(tab, v_tab);
        }
    }
}

void init_table_2d_coeff_f32_neon(float *tab, int tabsz) {
    if (nullptr == tab) {
        LOG_ERR("Mat is empty!");
        return;
    }

    float coeffs[WARP_INTER_TAB_SIZE * 2];
    float scale = 1.0f / tabsz;

    for (int i = 0; i < tabsz; i++) {
        coeffs[(i << 1) + 0] = 1.0f - scale * i;
        coeffs[(i << 1) + 1] = scale * i;
    }

    for (int j = 0; j < tabsz; j++) {
        float uy0 = coeffs[(j << 1) + 0];
        float uy1 = coeffs[(j << 1) + 1];
        for (int k = 0; k < tabsz; k++) {
            float vx0 = coeffs[(k << 1) + 0];
            float vx1 = coeffs[(k << 1) + 1];

            tab[0] = uy0 * vx0;
            tab[1] = uy0 * vx1;
            tab[2] = uy1 * vx0;
            tab[3] = uy1 * vx1;

            tab += 4;
        }
    }
}

static inline int16x8_t set_lane_s16x8x1(
        const unsigned char *src,
        const int32x4_t offset0,
        const int32x4_t offset1) {
    uint8x8_t dst = uint8x8_t();

    dst = vld1_lane_u8(&src[offset0[0]], dst, 0);
    dst = vld1_lane_u8(&src[offset0[1]], dst, 1);
    dst = vld1_lane_u8(&src[offset0[2]], dst, 2);
    dst = vld1_lane_u8(&src[offset0[3]], dst, 3);
    dst = vld1_lane_u8(&src[offset1[0]], dst, 4);
    dst = vld1_lane_u8(&src[offset1[1]], dst, 5);
    dst = vld1_lane_u8(&src[offset1[2]], dst, 6);
    dst = vld1_lane_u8(&src[offset1[3]], dst, 7);

    int16x8_t res;
    res = vreinterpretq_s16_u16(vmovl_u8(dst));

    return res;
}

//static inline int16x8x2_t set_lane_s16x8x2(
//        const unsigned char *src,
//        const int32x4_t offset0,
//        const int32x4_t offset1) {
//    uint8x8x2_t dst = uint8x8x2_t();
//
//    dst = vld2_lane_u8(src + offset0[0], dst, 0);
//    dst = vld2_lane_u8(src + offset0[1], dst, 1);
//    dst = vld2_lane_u8(src + offset0[2], dst, 2);
//    dst = vld2_lane_u8(src + offset0[3], dst, 3);
//    dst = vld2_lane_u8(src + offset1[0], dst, 4);
//    dst = vld2_lane_u8(src + offset1[1], dst, 5);
//    dst = vld2_lane_u8(src + offset1[2], dst, 6);
//    dst = vld2_lane_u8(src + offset1[3], dst, 7);
//
//    int16x8x2_t res;
//    res.val[0] = vreinterpretq_s16_u16(vmovl_u8(dst.val[0]));
//    res.val[1] = vreinterpretq_s16_u16(vmovl_u8(dst.val[1]));
//
//    return res;
//}

static inline int16x8x3_t set_lane_s16x8x3(
        const unsigned char *src,
        const int32x4_t offset0,
        const int32x4_t offset1) {
    uint8x8x3_t dst = uint8x8x3_t();

    dst = vld3_lane_u8(src + offset0[0], dst, 0);
    dst = vld3_lane_u8(src + offset0[1], dst, 1);
    dst = vld3_lane_u8(src + offset0[2], dst, 2);
    dst = vld3_lane_u8(src + offset0[3], dst, 3);
    dst = vld3_lane_u8(src + offset1[0], dst, 4);
    dst = vld3_lane_u8(src + offset1[1], dst, 5);
    dst = vld3_lane_u8(src + offset1[2], dst, 6);
    dst = vld3_lane_u8(src + offset1[3], dst, 7);

    int16x8x3_t res;
    res.val[0] = vreinterpretq_s16_u16(vmovl_u8(dst.val[0]));
    res.val[1] = vreinterpretq_s16_u16(vmovl_u8(dst.val[1]));
    res.val[2] = vreinterpretq_s16_u16(vmovl_u8(dst.val[2]));

    return res;
}

static inline int16x4x4_t get_coeffs_s16x4(
        short *src,
        int table[]) {
    int16x4x4_t dst = int16x4x4_t();

    dst = vld4_lane_s16(src + table[0], dst, 0);
    dst = vld4_lane_s16(src + table[1], dst, 1);
    dst = vld4_lane_s16(src + table[2], dst, 2);
    dst = vld4_lane_s16(src + table[3], dst, 3);

    return dst;
}

static inline float32x4x2_t set_lane_f32x4x2(
        const float *src,
        const int32x4_t offset) {
    float32x4x2_t dst = float32x4x2_t();

    dst = vld2q_lane_f32(&src[offset[0]], dst, 0);
    dst = vld2q_lane_f32(&src[offset[1]], dst, 1);
    dst = vld2q_lane_f32(&src[offset[2]], dst, 2);
    dst = vld2q_lane_f32(&src[offset[3]], dst, 3);

    return dst;
}

static inline float32x4x3_t set_lane_f32x4x3(
       const float *src,
       const int32x4_t offset) {
   float32x4x3_t dst = float32x4x3_t();

   dst = vld3q_lane_f32(&src[offset[0]], dst, 0);
   dst = vld3q_lane_f32(&src[offset[1]], dst, 1);
   dst = vld3q_lane_f32(&src[offset[2]], dst, 2);
   dst = vld3q_lane_f32(&src[offset[3]], dst, 3);

   return dst;
}

static inline float32x4x4_t get_coeffs_f32x4x4(
        float *src,
        int table[]) {
    float32x4x4_t dst = float32x4x4_t();

    dst = vld4q_lane_f32(&src[table[0]], dst, 0);
    dst = vld4q_lane_f32(&src[table[1]], dst, 1);
    dst = vld4q_lane_f32(&src[table[2]], dst, 2);
    dst = vld4q_lane_f32(&src[table[3]], dst, 3);

    return dst;
}

static inline bool get_coord_boundary(const int x, const int size) {
    return ((x >= 0) && (x < size));
}

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
        const Scalar& border_value) {
    const int src_width = src.width();
    const int src_height = src.height();
    int src_stride = src.stride();
    const int channel = src.channels();
    const unsigned char* src_data = (const unsigned char*)src.data();

    int width1 = src_width - 2;
    int height1 = src_height - 2;
    int delta = 1 << (WARP_SCALE_REMAP_BITS - 1);
    int16x4_t v_src_stride = vdup_n_s16(src_stride);
    int32x4_t v_offset  = vdupq_n_s32(1 << (WARP_SCALE_REMAP_BITS - 1));
    int tab_scale = (1 << WARP_SCALE_BITS) - 1;

    unsigned char cval[4];
    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<unsigned char>(border_value.val()[k]);
    }

    if (1 == channel) {
        for (int y = start_height; y < end_height; y++) {
            bool pre_inter = false;
            int offset = width * y;
            const short* src_xy_row = (const short *)(src_xy + (offset << 1));
            const short* coeffs_row = (const short *)(coeffs + offset);
            unsigned char* dst_row = (unsigned char *)(dst + dst_stride * y);

            const unsigned char* src_data0 = src_data;
            const unsigned char* src_data1 = src_data + src_stride;

            int x0 = 0;
            int x = 0;
            for (; x <= width; x++) {
                int idx = x << 1;
                bool cur_inter = x < width ? (unsigned short)src_xy_row[idx] < width1
                        && (unsigned short)src_xy_row[idx + 1] < height1 : !pre_inter;

                // get parallelable x value
                if (cur_inter == pre_inter) {
                    continue;
                }

                int x1 = x;
                x = x0;
                x0 = x1;
                pre_inter = cur_inter;

                if (!cur_inter) {
                    int dt = x1 - x;

                    int d_align8 = dt & (~7);
                    const short* xy_row = src_xy_row + (x << 1);
                    const short* co_row = coeffs_row + x;
                    unsigned char* cur_dst = dst_row + x;
                    int d = 0;

                    for (d = 0; d < d_align8; d += 8) {
                        const short* cur_coeff = co_row + d;
                        int16x8x2_t xy0 = vld2q_s16(xy_row + d * 2);

                        int32x4_t voffset0 = vmovl_s16(vget_low_s16(xy0.val[0]));
                        int32x4_t voffset1 = vmovl_s16(vget_high_s16(xy0.val[0]));

                        voffset0 = vmlal_s16(voffset0, vget_low_s16(xy0.val[1]), v_src_stride);
                        voffset1 = vmlal_s16(voffset1, vget_high_s16(xy0.val[1]), v_src_stride);

                        int tab_index0[] = {(cur_coeff[0] & tab_scale) << 2,
                                (cur_coeff[1] & tab_scale) << 2,
                                (cur_coeff[2] & tab_scale) << 2,
                                (cur_coeff[3] & tab_scale) << 2};

                        int tab_index1[] = {(cur_coeff[4] & tab_scale) << 2,
                                (cur_coeff[5] & tab_scale) << 2,
                                (cur_coeff[6] & tab_scale) << 2,
                                (cur_coeff[7] & tab_scale) << 2};

                        int of0 = voffset0[0];
                        int of1 = voffset0[1];
                        int of2 = voffset0[2];
                        int of3 = voffset0[3];
                        int of4 = voffset1[0];
                        int of5 = voffset1[1];
                        int of6 = voffset1[2];
                        int of7 = voffset1[3];

                        uint8x8x2_t dst = uint8x8x2_t();
                        dst = vld2_lane_u8(src_data0 + of0, dst, 0);
                        dst = vld2_lane_u8(src_data0 + of1, dst, 1);
                        dst = vld2_lane_u8(src_data0 + of2, dst, 2);
                        dst = vld2_lane_u8(src_data0 + of3, dst, 3);
                        dst = vld2_lane_u8(src_data0 + of4, dst, 4);
                        dst = vld2_lane_u8(src_data0 + of5, dst, 5);
                        dst = vld2_lane_u8(src_data0 + of6, dst, 6);
                        dst = vld2_lane_u8(src_data0 + of7, dst, 7);

                        int16x8x2_t src_l0, src_l1;
                        src_l0.val[0] = vreinterpretq_s16_u16(vmovl_u8(dst.val[0]));
                        src_l0.val[1] = vreinterpretq_s16_u16(vmovl_u8(dst.val[1]));

                        dst = vld2_lane_u8(src_data1 + of0, dst, 0);
                        dst = vld2_lane_u8(src_data1 + of1, dst, 1);
                        dst = vld2_lane_u8(src_data1 + of2, dst, 2);
                        dst = vld2_lane_u8(src_data1 + of3, dst, 3);
                        dst = vld2_lane_u8(src_data1 + of4, dst, 4);
                        dst = vld2_lane_u8(src_data1 + of5, dst, 5);
                        dst = vld2_lane_u8(src_data1 + of6, dst, 6);
                        dst = vld2_lane_u8(src_data1 + of7, dst, 7);

                        src_l1.val[0] = vreinterpretq_s16_u16(vmovl_u8(dst.val[0]));
                        src_l1.val[1] = vreinterpretq_s16_u16(vmovl_u8(dst.val[1]));

                        int16x4x4_t coeff0 = get_coeffs_s16x4(tab, tab_index0);
                        int16x4x4_t coeff1 = get_coeffs_s16x4(tab, tab_index1);

                        int32x4_t vsum0 = v_offset;
                        int32x4_t vsum1 = v_offset;

                        vsum0 = vmlal_s16(vsum0, vget_low_s16(src_l0.val[0]), coeff0.val[0]);
                        vsum1 = vmlal_s16(vsum1, vget_high_s16(src_l0.val[0]), coeff1.val[0]);

                        vsum0 = vmlal_s16(vsum0, vget_low_s16(src_l0.val[1]), coeff0.val[1]);
                        vsum1 = vmlal_s16(vsum1, vget_high_s16(src_l0.val[1]), coeff1.val[1]);

                        vsum0 = vmlal_s16(vsum0, vget_low_s16(src_l1.val[0]), coeff0.val[2]);
                        vsum1 = vmlal_s16(vsum1, vget_high_s16(src_l1.val[0]), coeff1.val[2]);

                        vsum0 = vmlal_s16(vsum0, vget_low_s16(src_l1.val[1]), coeff0.val[3]);
                        vsum1 = vmlal_s16(vsum1, vget_high_s16(src_l1.val[1]), coeff1.val[3]);

                        //merge the first and second parts
                        vst1_u8(cur_dst + d, vqmovn_u16(vcombine_u16(
                                vshrn_n_s32(vsum0, 15), vshrn_n_s32(vsum1, 15))));
                    }

                    //the rest part
                    for (; d < dt; d++) {
                        int d2 = d << 1;
                        short srcx0 = xy_row[d2];
                        short srcy0 = xy_row[d2 + 1];

                        const unsigned char* s0 = src_data + srcy0 * src_stride + srcx0;
                        const unsigned char* s1 = s0 + src_stride;

                        //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                        int tab_offset = (co_row[d] & tab_scale) << 2;
                        short * tab_ptr = (short *)(tab + tab_offset);
                        short coeffs00 = *(tab_ptr++);
                        short coeffs01 = *(tab_ptr++);
                        short coeffs10 = *(tab_ptr++);
                        short coeffs11 = *(tab_ptr++);

                        unsigned char src00 = s0[0];
                        unsigned char src01 = s0[1];
                        unsigned char src10 = s1[0];
                        unsigned char src11 = s1[1];

                        cur_dst[d] = (unsigned char)((src00 * coeffs00 + src10 * coeffs10 +
                                src01 * coeffs01 + src11 * coeffs11 +
                                (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    }
                    x = x1; 
                } else {
                    for (; x < x1; x++) {
                        short srcx0 = src_xy_row[x * 2];
                        short srcy0 = src_xy_row[x * 2 + 1];
                        short srcx1 = srcx0 + 1;
                        short srcy1 = srcy0 + 1;
                        const unsigned char* psrc0 = src_data + srcy0 * src_stride + srcx0;
                        const unsigned char* psrc1 = psrc0 + src_stride;

                        int tab_offset = (coeffs_row[x] & tab_scale) << 2;

                        short* tab_ptr = (short *)(tab + tab_offset);
                        short coeffs00 = *(tab_ptr++);
                        short coeffs01 = *(tab_ptr++);
                        short coeffs10 = *(tab_ptr++);
                        short coeffs11 = *(tab_ptr++);

                        bool flag_x0 = get_coord_boundary(srcx0, src_width);
                        bool flag_x1 = get_coord_boundary(srcx1, src_width);
                        bool flag_y0 = get_coord_boundary(srcy0, src_height);
                        bool flag_y1 = get_coord_boundary(srcy1, src_height);

                        bool mask0 = flag_x0 && flag_y0;
                        bool mask1 = flag_x1 && flag_y0;
                        bool mask2 = flag_x0 && flag_y1;
                        bool mask3 = flag_x1 && flag_y1;

                        int sum = 0;
                        sum += mask0 ? (psrc0[0] * coeffs00) : (cval[0] * coeffs00);
                        sum += mask1 ? (psrc0[1] * coeffs01) : (cval[0] * coeffs01);
                        sum += mask2 ? (psrc1[0] * coeffs10) : (cval[0] * coeffs10);
                        sum += mask3 ? (psrc1[1] * coeffs11) : (cval[0] * coeffs11);

                        dst_row[x] = (unsigned char)((sum + delta) >> (WARP_SCALE_REMAP_BITS));
                    }
                }
            }
        }
    } else if (3 == channel) {
        int32x4_t v_channel = vdupq_n_s32(channel);
        int32x4x3_t v3_offset;
        v3_offset.val[0] = v_offset;
        v3_offset.val[1] = v_offset;
        v3_offset.val[2] = v_offset;

        for (int y = start_height; y < end_height; y++) {
            bool pre_inter = false;
            int offset = width * y;
            short* src_xy_row = (short *)(src_xy + (offset << 1));
            short* coeffs_row = (short *)(coeffs + offset);
            unsigned char* dst_row = (unsigned char *)(dst + dst_stride * y);

            int x = 0;
            int x0 = 0;

            for (x = 0; x <= width; x++) {
                bool cur_inter = x < width ? (unsigned short)src_xy_row[x * 2] < width1
                        && (unsigned short)src_xy_row[x * 2 + 1] < height1 : !pre_inter;

                if (cur_inter == pre_inter) {
                    continue;
                }

                int x1 = x;
                x = x0;
                x0 = x1;
                pre_inter = cur_inter;

                if (!cur_inter) {
                    int dt = x1 - x;
                    int d_align8 = dt & (~7);
                    const short* xy_row = src_xy_row + (x << 1);
                    const short* co_row = coeffs_row + x;
                    unsigned char* cur_dst = dst_row + x * channel;
                    int d = 0;

                    for (d = 0; d < d_align8; d += 8) {
                        int d2 = d << 1;
                        const short* cur_coeff = co_row + d;
                        int16x8x2_t xy0 = vld2q_s16(xy_row + d2);

                        int16x8_t vx0 = vmulq_n_s16(xy0.val[0], channel);

                        int tab_index0[] = {(cur_coeff[0] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[1] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[2] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[3] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                        int32x4_t voffset0 = vmovl_s16(vget_low_s16(vx0));
                        int32x4_t voffset1 = vmovl_s16(vget_high_s16(vx0));

                        voffset0 = vmlal_s16(voffset0, vget_low_s16(xy0.val[1]), v_src_stride);
                        voffset1 = vmlal_s16(voffset1, vget_high_s16(xy0.val[1]), v_src_stride);

                        // int32x4_t tab_index0 = vmovl_s16(vld1_s16(cur_coeff));
                        // tab_index0 = vshlq_n_s32(vandq_s32(tab_index0, v_scale_1), 2);

                        int tab_index1[] = {(cur_coeff[4] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[5] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[6] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[7] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                        int32x4_t voffset2 = vaddq_s32(voffset0, v_channel);
                        int32x4_t voffset3 = vaddq_s32(voffset1, v_channel);

                        int16x8x3_t src_l00 = set_lane_s16x8x3(src_data, voffset0, voffset1);
                        int16x8x3_t src_l10 = set_lane_s16x8x3(src_data + src_stride, voffset0, voffset1);

                        int16x8x3_t src_l01 = set_lane_s16x8x3(src_data, voffset2, voffset3);
                        int16x8x3_t src_l11 = set_lane_s16x8x3(src_data + src_stride, voffset2, voffset3);

                        int16x4x4_t coeff = get_coeffs_s16x4(tab, tab_index0);

                        int32x4x3_t vsum0 = v3_offset;
                        vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l00.val[0]), coeff.val[0]);
                        vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l00.val[1]), coeff.val[0]);
                        vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l00.val[2]), coeff.val[0]);

                        vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l01.val[0]), coeff.val[1]);
                        vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l01.val[1]), coeff.val[1]);
                        vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l01.val[2]), coeff.val[1]);

                        vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l10.val[0]), coeff.val[2]);
                        vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l10.val[1]), coeff.val[2]);
                        vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l10.val[2]), coeff.val[2]);

                        vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l11.val[0]), coeff.val[3]);
                        vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l11.val[1]), coeff.val[3]);
                        vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l11.val[2]), coeff.val[3]);

                        coeff = get_coeffs_s16x4(tab, tab_index1);

                        int32x4x3_t vsum1 = v3_offset;
                        vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l00.val[0]), coeff.val[0]);
                        vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l00.val[1]), coeff.val[0]);
                        vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l00.val[2]), coeff.val[0]);

                        vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l01.val[0]), coeff.val[1]);
                        vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l01.val[1]), coeff.val[1]);
                        vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l01.val[2]), coeff.val[1]);

                        vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l10.val[0]), coeff.val[2]);
                        vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l10.val[1]), coeff.val[2]);
                        vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l10.val[2]), coeff.val[2]);

                        vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l11.val[0]), coeff.val[3]);
                        vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l11.val[1]), coeff.val[3]);
                        vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l11.val[2]), coeff.val[3]);

                        //merge the first and second parts
                        uint8x8x3_t res;
                        res.val[0] = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[0], 15),
                                vshrn_n_s32(vsum1.val[0], 15)));
                        res.val[1] = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[1], 15),
                                vshrn_n_s32(vsum1.val[1], 15)));
                        res.val[2] = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[2], 15),
                                vshrn_n_s32(vsum1.val[2], 15)));

                        vst3_u8(cur_dst + (d2 + d), res);
                    }

                    //the rest part
                    for (; d < dt; d++) {
                        int d2 = d << 1;
                        short srcx0 = xy_row[d2];
                        short srcy0 = xy_row[d2 + 1];

                        const unsigned char* s0 = src_data + srcy0 * src_stride + srcx0 * channel;
                        const unsigned char* s1 = s0 + src_stride;

                        //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                        int tab_offset = (co_row[d] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                        short * tab_ptr = (short *)(tab + tab_offset);
                        short coeffs00 = *(tab_ptr++);
                        short coeffs01 = *(tab_ptr++);
                        short coeffs10 = *(tab_ptr++);
                        short coeffs11 = *(tab_ptr++);

                        for (int ch = 0; ch < channel; ch++) {
                            unsigned char src00 = s0[ch];
                            unsigned char src01 = s0[channel + ch];
                            unsigned char src10 = s1[ch];
                            unsigned char src11 = s1[channel + ch];

                            cur_dst[d * channel + ch] = (unsigned char)((src00 * coeffs00 +
                                    src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                                    (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                        }
                    }
                    x = x1;
                } else {
                    for (; x < x1; x++) {
                        short srcx0 = src_xy_row[x * 2];
                        short srcx1 = srcx0 + 1;
                        short srcy0 = src_xy_row[x * 2 + 1];
                        short srcy1 = srcy0 + 1;

                        int tab_offset = (coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                        short * tab_ptr = (short *)(tab + tab_offset);

                        short coeffs00 = *(tab_ptr++);
                        short coeffs01 = *(tab_ptr++);
                        short coeffs10 = *(tab_ptr++);
                        short coeffs11 = *(tab_ptr++);

                        bool flag_x0 = get_coord_boundary(srcx0, src_width);
                        bool flag_x1 = get_coord_boundary(srcx1, src_width);
                        bool flag_y0 = get_coord_boundary(srcy0, src_height);
                        bool flag_y1 = get_coord_boundary(srcy1, src_height);

                        const unsigned char* p_src00 = (flag_x0 && flag_y0) ?
                                &src_data[srcy0 * src_stride + srcx0 * channel] : &cval[0];
                        const unsigned char* p_src01 = (flag_x1 && flag_y0) ?
                                &src_data[srcy0 * src_stride + srcx1 * channel] : &cval[0];
                        const unsigned char* p_src10 = (flag_x0 && flag_y1) ?
                                &src_data[srcy1 * src_stride + srcx0 * channel] : &cval[0];
                        const unsigned char* p_src11 = (flag_x1 && flag_y1) ?
                                &src_data[srcy1 * src_stride + srcx1 * channel] : &cval[0];

                        for (int ch = 0; ch < channel; ch++) {
                            const unsigned char src00 = p_src00[ch];
                            const unsigned char src01 = p_src01[ch];
                            const unsigned char src10 = p_src10[ch];
                            const unsigned char src11 = p_src11[ch];

                            dst_row[x * channel + ch] = (unsigned char)((src00 * coeffs00 +
                                    src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                                    (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                        }
                    }
                }
            }
        }
    }
}

void remap_linear_const_planar_u8_neon(
        int start_height,
        int end_height,
        const Mat& src,
        unsigned char* dst,
        const int dst_width,
        const int dst_height,
        const int dst_stride,
        const short *src_xy,
        const short *coeffs,
        short *tab,
        const Scalar& border_value) {
    const int src_width = src.width();
    const int src_height = src.height();
    int src_stride = src.stride();
    const int channel = src.channels();

    const unsigned char* src_data = (const unsigned char*)src.data();

    int width1 = src_width - 2;
    int height1 = src_height - 2;
    int16x4_t v_src_stride = vdup_n_s16(src_stride);
    int32x4_t v_offset  = vdupq_n_s32(1 << (WARP_SCALE_REMAP_BITS - 1));

    unsigned char cval[4];
    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<unsigned char>(border_value.val()[k]);
    }

    int src_step = src_height * src_width;
    int dst_step = dst_height * dst_width;

    unsigned char* dst_b = (unsigned char *)dst;
    unsigned char* dst_g = (unsigned char *)dst_b + dst_step;
    unsigned char* dst_r = (unsigned char *)dst_g + dst_step;

    unsigned char* src_b = (unsigned char *)src_data;
    unsigned char* src_g = (unsigned char *)src_b + src_step;
    unsigned char* src_r = (unsigned char *)src_g + src_step;

    int32x4_t v_channel  = vdupq_n_s32(1);

    for (int y = start_height; y < end_height; y++) {
        bool pre_inter = false;
        int offset = dst_width * y;
        short* src_xy_row = (short *)(src_xy + (offset << 1));
        short* coeffs_row = (short *)(coeffs + offset);

        unsigned char* dstb_row = (unsigned char *)(dst_b + dst_stride * y);
        unsigned char* dstg_row = (unsigned char *)(dst_g + dst_stride * y);
        unsigned char* dstr_row = (unsigned char *)(dst_r + dst_stride * y);

        int x = 0;
        int x0 = 0;

        for (x = 0; x <= dst_width; x++) {
            bool cur_inter = x < dst_width ? (unsigned short)src_xy_row[x * 2] < width1
                    && (unsigned short)src_xy_row[x * 2 + 1] < height1 : !pre_inter;

            if (cur_inter == pre_inter) {
                continue;
            }

            int x1 = x;
            x = x0;
            x0 = x1;
            pre_inter = cur_inter;

            if (!cur_inter) {
                int dt = x1 - x;
                int d_align8 = dt & (~7);
                const short* xy_row = src_xy_row + (x << 1);
                const short* co_row = coeffs_row + x;
                unsigned char* curb_dst = dstb_row + x;
                unsigned char* curg_dst = dstg_row + x;
                unsigned char* curr_dst = dstr_row + x;
                int d = 0;

                for (d = 0; d < d_align8; d += 8) {
                    int d2 = d << 1;
                    const short* cur_coeff = co_row + d;
                    int16x8x2_t xy0 = vld2q_s16(xy_row + d2);

                    int16x8_t vx0 = xy0.val[0];

                    int32x4_t voffset0 = vmull_s16(vget_low_s16(xy0.val[1]), v_src_stride);
                    voffset0 = vaddq_s32(voffset0, vmovl_s16(vget_low_s16(vx0)));

                    int32x4_t voffset1 = vmull_s16(vget_high_s16(xy0.val[1]), v_src_stride);
                    voffset1 = vaddq_s32(voffset1, vmovl_s16(vget_high_s16(vx0)));

                    int tab_index0[] = {(cur_coeff[0] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[1] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[2] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[3] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                     int tab_index1[] = {(cur_coeff[4] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[5] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[6] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                            (cur_coeff[7] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                    int16x8x3_t src_l00, src_l10, src_l01, src_l11;
                    src_l00.val[0] = set_lane_s16x8x1(src_b, voffset0, voffset1);
                    src_l10.val[0] = set_lane_s16x8x1(src_b + src_stride, voffset0, voffset1);

                    src_l00.val[1] = set_lane_s16x8x1(src_g, voffset0, voffset1);
                    src_l10.val[1] = set_lane_s16x8x1(src_g + src_stride, voffset0, voffset1);

                    src_l00.val[2] = set_lane_s16x8x1(src_r, voffset0, voffset1);
                    src_l10.val[2] = set_lane_s16x8x1(src_r + src_stride, voffset0, voffset1);

                    int32x4_t voffset2 = vaddq_s32(voffset0, v_channel);
                    int32x4_t voffset3 = vaddq_s32(voffset1, v_channel);

                    src_l01.val[0] = set_lane_s16x8x1(src_b, voffset2, voffset3);
                    src_l11.val[0] = set_lane_s16x8x1(src_b + src_stride, voffset2, voffset3);

                    src_l01.val[1] = set_lane_s16x8x1(src_g, voffset2, voffset3);
                    src_l11.val[1] = set_lane_s16x8x1(src_g + src_stride, voffset2, voffset3);

                    src_l01.val[2] = set_lane_s16x8x1(src_r, voffset2, voffset3);
                    src_l11.val[2] = set_lane_s16x8x1(src_r + src_stride, voffset2, voffset3);

                    int16x4x4_t coeff = get_coeffs_s16x4(tab, tab_index0);

                    int32x4x3_t vsum0;
                    vsum0.val[0] = vmull_s16(vget_low_s16(src_l00.val[0]), coeff.val[0]);
                    vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l01.val[0]), coeff.val[1]);
                    vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l10.val[0]), coeff.val[2]);
                    vsum0.val[0] = vmlal_s16(vsum0.val[0], vget_low_s16(src_l11.val[0]), coeff.val[3]);

                    vsum0.val[1] = vmull_s16(vget_low_s16(src_l00.val[1]), coeff.val[0]);
                    vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l01.val[1]), coeff.val[1]);
                    vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l10.val[1]), coeff.val[2]);
                    vsum0.val[1] = vmlal_s16(vsum0.val[1], vget_low_s16(src_l11.val[1]), coeff.val[3]);

                    vsum0.val[2] = vmull_s16(vget_low_s16(src_l00.val[2]), coeff.val[0]);
                    vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l01.val[2]), coeff.val[1]);
                    vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l10.val[2]), coeff.val[2]);
                    vsum0.val[2] = vmlal_s16(vsum0.val[2], vget_low_s16(src_l11.val[2]), coeff.val[3]);

                    vsum0.val[0] = vaddq_s32(vsum0.val[0], v_offset);
                    vsum0.val[1] = vaddq_s32(vsum0.val[1], v_offset);
                    vsum0.val[2] = vaddq_s32(vsum0.val[2], v_offset);

                    coeff = get_coeffs_s16x4(tab, tab_index1);

                    int32x4x3_t vsum1;
                    vsum1.val[0] = vmull_s16(vget_high_s16(src_l00.val[0]), coeff.val[0]);
                    vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l01.val[0]), coeff.val[1]);
                    vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l10.val[0]), coeff.val[2]);
                    vsum1.val[0] = vmlal_s16(vsum1.val[0], vget_high_s16(src_l11.val[0]), coeff.val[3]);

                    vsum1.val[1] = vmull_s16(vget_high_s16(src_l00.val[1]), coeff.val[0]);
                    vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l01.val[1]), coeff.val[1]);
                    vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l10.val[1]), coeff.val[2]);
                    vsum1.val[1] = vmlal_s16(vsum1.val[1], vget_high_s16(src_l11.val[1]), coeff.val[3]);

                    vsum1.val[2] = vmull_s16(vget_high_s16(src_l00.val[2]), coeff.val[0]);
                    vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l01.val[2]), coeff.val[1]);
                    vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l10.val[2]), coeff.val[2]);
                    vsum1.val[2] = vmlal_s16(vsum1.val[2], vget_high_s16(src_l11.val[2]), coeff.val[3]);

                    vsum1.val[0] = vaddq_s32(vsum1.val[0], v_offset);
                    vsum1.val[1] = vaddq_s32(vsum1.val[1], v_offset);
                    vsum1.val[2] = vaddq_s32(vsum1.val[2], v_offset);

                    //merge the first and second parts
                    uint8x8_t res_b, res_g, res_r;
                    res_b = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[0], 15),
                            vshrn_n_s32(vsum1.val[0], 15)));
                    res_g = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[1], 15),
                            vshrn_n_s32(vsum1.val[1], 15)));
                    res_r = vqmovn_u16(vcombine_u16(vshrn_n_s32(vsum0.val[2], 15),
                            vshrn_n_s32(vsum1.val[2], 15)));

                    vst1_u8(curb_dst + d, res_b);
                    vst1_u8(curg_dst + d, res_g);
                    vst1_u8(curr_dst + d, res_r);
                }

                //the rest part
                for (; d < dt; d++) {
                    int d2 = d << 1;
                    short srcx0 = src_xy_row[d2];
                    short srcy0 = src_xy_row[d2 + 1];

                    const unsigned char* s_b0 = src_b + srcy0 * src_stride + srcx0;
                    const unsigned char* s_b1 = s_b0 + src_stride;

                    const unsigned char* s_g0 = src_g + srcy0 * src_stride + srcx0;
                    const unsigned char* s_g1 = s_g0 + src_stride;

                    const unsigned char* s_r0 = src_r + srcy0 * src_stride + srcx0;
                    const unsigned char* s_r1 = s_r0 + src_stride;

                    //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                    int tab_offset = ((coeffs_row[d] & ((1 << WARP_SCALE_BITS) - 1)) << 2);
                    //int tab_offset = coeffs_row[x] << 2;
                    short * tab_ptr = (short *)(tab + tab_offset);
                    short coeffs00 = *(tab_ptr++);
                    short coeffs01 = *(tab_ptr++);
                    short coeffs10 = *(tab_ptr++);
                    short coeffs11 = *(tab_ptr++);

                    dstb_row[d] = fcv_cast_u8((s_b0[0] * coeffs00 +
                            s_b1[0] * coeffs10 + s_b0[1] * coeffs01 + s_b1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    dstg_row[d] = fcv_cast_u8((s_g0[0] * coeffs00 +
                            s_g1[0] * coeffs10 + s_g0[1] * coeffs01 + s_g1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    dstr_row[d] = fcv_cast_u8((s_r0[0] * coeffs00 +
                            s_r1[0] * coeffs10 + s_r0[1] * coeffs01 + s_r1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));

                }
                x = x1;
            } else {
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x * 2];
                    short srcx1 = srcx0 + 1;
                    short srcy0 = src_xy_row[x * 2 + 1];
                    short srcy1 = srcy0 + 1;

                    int tab_offset = (coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                    short* tab_ptr = (short *)(tab + tab_offset);

                    short coeffs00 = *(tab_ptr++);
                    short coeffs01 = *(tab_ptr++);
                    short coeffs10 = *(tab_ptr++);
                    short coeffs11 = *(tab_ptr++);

                    bool flag_x0 = get_coord_boundary(srcx0, src_width);
                    bool flag_x1 = get_coord_boundary(srcx1, src_width);
                    bool flag_y0 = get_coord_boundary(srcy0, src_height);
                    bool flag_y1 = get_coord_boundary(srcy1, src_height);

                    unsigned char src00 = (flag_x0 && flag_y0) ? src_b[srcy0 * src_stride + srcx0] : cval[0];
                    unsigned char src01 = (flag_x1 && flag_y0) ? src_b[srcy0 * src_stride + srcx1] : cval[0];
                    unsigned char src10 = (flag_x0 && flag_y1) ? src_b[srcy1 * src_stride + srcx0] : cval[0];
                    unsigned char src11 = (flag_x1 && flag_y1) ? src_b[srcy1 * src_stride + srcx1] : cval[0];

                    dstb_row[x] = fcv_cast_u8((src00 * coeffs00 +
                            src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));

                    src00 = (flag_x0 && flag_y0) ? src_g[srcy0 * src_stride + srcx0] : cval[0];
                    src01 = (flag_x1 && flag_y0) ? src_g[srcy0 * src_stride + srcx1] : cval[0];
                    src10 = (flag_x0 && flag_y1) ? src_g[srcy1 * src_stride + srcx0] : cval[0];
                    src11 = (flag_x1 && flag_y1) ? src_g[srcy1 * src_stride + srcx1] : cval[0];

                    dstg_row[x] = fcv_cast_u8((src00 * coeffs00 +
                            src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));

                    src00 = (flag_x0 && flag_y0) ? src_r[srcy0 * src_stride + srcx0] : cval[0];
                    src01 = (flag_x1 && flag_y0) ? src_r[srcy0 * src_stride + srcx1] : cval[0];
                    src10 = (flag_x0 && flag_y1) ? src_r[srcy1 * src_stride + srcx0] : cval[0];
                    src11 = (flag_x1 && flag_y1) ? src_r[srcy1 * src_stride + srcx1] : cval[0];

                    dstr_row[x] = fcv_cast_u8((src00 * coeffs00 +
                            src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                }
            }
        }
    } 
}

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
        const Scalar& border_value) {
    const int src_width = src.width();
    const int src_height = src.height();
    const int channel = src.channels();
    int src_stride = src.stride() / sizeof(float);
    const float* src_data = (const float*)src.data();

    int width1 = src_width - 2;
    int height1 = src_height - 2;
    int16x4_t v_src_stride = vdup_n_s16(src_stride);

    float cval[4];
    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<float>(border_value.val()[k]);
    }

    const float* src0 = src_data;
    const float* src1 = src_data + src_stride;

    if (1 == channel) {
        for (int y = start_height; y < end_height; y++) {
            bool pre_inter = false;
            int offset = width * y;
            const short*  src_xy_row = (const short *)(src_xy + (offset << 1));
            const short*  coeffs_row = (const short *)(coeffs + offset);

            float* dst_row = (float *)(dst + dst_stride * y);

            int x0 = 0;
            for (int x = 0; x <= width; x++) {
                bool cur_inter = x < width ? (unsigned short)src_xy_row[x * 2] < width1
                        && (unsigned short)src_xy_row[x * 2 + 1] < height1 : !pre_inter;
                if (cur_inter == pre_inter) {
                    continue;
                }

                int x1 = x;
                x = x0;
                x0 = x1;
                pre_inter = cur_inter;

                if (!cur_inter) {
                    int dt = x1 - x;
                    int d_align4 = dt & (~7);
                    const short* xy_row = src_xy_row + (x << 1);
                    const short* co_row = coeffs_row + x;
                    float* cur_dst = dst_row + x;
                    int d = 0;
                    for (d = 0; d < d_align4; d += 8) {
                        const short* cur_coeff = co_row + d;
                        int16x8x2_t xy0 = vld2q_s16(xy_row + d * 2);

                        int32x4_t voffset0 = vmovl_s16(vget_low_s16(xy0.val[0]));
                        int32x4_t voffset1 = vmovl_s16(vget_high_s16(xy0.val[0]));

                        int tab_index0[] = {(cur_coeff[0] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[1] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[2] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[3] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                        int tab_index1[] = {(cur_coeff[4] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[5] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[6] & ((1 << WARP_SCALE_BITS) - 1)) << 2,
                                (cur_coeff[7] & ((1 << WARP_SCALE_BITS) - 1)) << 2};

                        voffset0 = vmlal_s16(voffset0, vget_low_s16(xy0.val[1]), v_src_stride);
                        voffset1 = vmlal_s16(voffset1, vget_high_s16(xy0.val[1]), v_src_stride);

                        float32x4x4_t coeff0 = get_coeffs_f32x4x4(tab, tab_index0);
                        float32x4x4_t coeff1 = get_coeffs_f32x4x4(tab, tab_index1);

                        float32x4x2_t src_l00 = set_lane_f32x4x2(src0, voffset0);
                        float32x4x2_t src_l10 = set_lane_f32x4x2(src1, voffset0);

                        float32x4x2_t src_l01 = set_lane_f32x4x2(src0, voffset1);
                        float32x4x2_t src_l11 = set_lane_f32x4x2(src1, voffset1);

                        float32x4_t vsum0 = vmulq_f32(src_l00.val[0], coeff0.val[0]);
                        float32x4_t vsum1 = vmulq_f32(src_l01.val[0], coeff1.val[0]);

                        vsum0 = vmlaq_f32(vsum0, src_l00.val[1], coeff0.val[1]);
                        vsum1 = vmlaq_f32(vsum1, src_l01.val[1], coeff1.val[1]);

                        vsum0 = vmlaq_f32(vsum0, src_l10.val[0], coeff0.val[2]);
                        vsum1 = vmlaq_f32(vsum1, src_l11.val[0], coeff1.val[2]);

                        vsum0 = vmlaq_f32(vsum0, src_l10.val[1], coeff0.val[3]);
                        vsum1 = vmlaq_f32(vsum1, src_l11.val[1], coeff1.val[3]);

                        //store
                        vst1q_f32(cur_dst + d, vsum0);
                        vst1q_f32(cur_dst + d + 4, vsum1);
                    }

                    //the rest part
                    for (; d < dt; d++) {
                        int d2 = d << 1;
                        short srcx0 = xy_row[d2];
                        short srcy0 = xy_row[d2 + 1];

                        const float* s0 = src_data + srcy0 * src_stride + srcx0;
                        const float* s1 = s0 + src_stride;

                        //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                        int tab_offset = ((co_row[d] & ((1 << WARP_SCALE_BITS) - 1)) << 2);
                        float* tab_ptr = (float *)(tab + tab_offset);
                        float coeffs00 = tab_ptr[0];
                        float coeffs01 = tab_ptr[1];
                        float coeffs10 = tab_ptr[2];
                        float coeffs11 = tab_ptr[3];

                        float src00 = s0[0];
                        float src01 = s0[1];
                        float src10 = s1[0];
                        float src11 = s1[1];

                        cur_dst[d] = (float)(src00 * coeffs00 + src10 * coeffs10
                                + src01 * coeffs01 + src11 * coeffs11);
                    }
                    x = x1;
                } else {
                    for (; x < x1; x++) {
                        int x2 = x << 1;
                        short srcx0 = src_xy_row[x2];
                        short srcy0 = src_xy_row[x2 + 1];
                        short srcx1 = srcx0 + 1;
                        short srcy1 = srcy0 + 1;
                        const float* psrc0 = src_data + srcy0 * src_stride + srcx0;
                        const float* psrc1 = psrc0 + src_stride;

                        int tab_offset = ((coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2);

                        float * tab_ptr = (float *)(tab + tab_offset);
                        float coeffs00 = *(tab_ptr++);
                        float coeffs01 = *(tab_ptr++);
                        float coeffs10 = *(tab_ptr++);
                        float coeffs11 = *(tab_ptr++);

                        bool flag_x0 = get_coord_boundary(srcx0, src_width);
                        bool flag_x1 = get_coord_boundary(srcx1, src_width);
                        bool flag_y0 = get_coord_boundary(srcy0, src_height);
                        bool flag_y1 = get_coord_boundary(srcy1, src_height);

                        bool mask0 = flag_x0 && flag_y0;
                        bool mask1 = flag_x1 && flag_y0;
                        bool mask2 = flag_x0 && flag_y1;
                        bool mask3 = flag_x1 && flag_y1;

                        float sum = 0.f;
                        sum += mask0 ? (psrc0[0] * coeffs00) : (cval[0] * coeffs00);
                        sum += mask1 ? (psrc0[1] * coeffs01) : (cval[0] * coeffs01);
                        sum += mask2 ? (psrc1[0] * coeffs10) : (cval[0] * coeffs10);
                        sum += mask3 ? (psrc1[1] * coeffs11) : (cval[0] * coeffs11);

                        dst_row[x] = sum;

                        // bool flag_x0 = get_coord_boundary(srcx0, src_width);
                        // bool flag_x1 = get_coord_boundary(srcx1, src_width);
                        // bool flag_y0 = get_coord_boundary(srcy0, src_height);
                        // bool flag_y1 = get_coord_boundary(srcy1, src_height);

                        // const float* p_src00 = (flag_x0 && flag_y0) ? &psrc0[0] : &cval[0];
                        // const float* p_src01 = (flag_x1 && flag_y0) ? &psrc0[1] : &cval[0];
                        // const float* p_src10 = (flag_x0 && flag_y1) ? &psrc1[0] : &cval[0];
                        // const float* p_src11 = (flag_x1 && flag_y1) ? &psrc1[1] : &cval[0];

                        // float src00 = p_src00[0];
                        // float src01 = p_src01[0];
                        // float src10 = p_src10[0];
                        // float src11 = p_src11[0];

                        // dst_row[x] = (float)(src00 * coeffs00 + src10 * coeffs10
                        //         + src01 * coeffs01 + src11 * coeffs11);
                    }
                }
            }
        }
    } else if (3 == channel) {
        int32x4_t v_channel  = vdupq_n_s32(channel);
        for (int y = start_height; y < end_height; y++) {
            bool pre_inter = false;
            int offset = width * y;
            short *src_xy_row = (short *)(src_xy + (offset << 1));
            short *coeffs_row = (short *)(coeffs + offset);
            float* dst_row = (float *)(dst + dst_stride * y);

            int x = 0;
            int x0 = 0;
            for (x = 0; x <= width; x++) {
                bool cur_inter = x < width ? (unsigned short)src_xy_row[x * 2] < width1
                        && (unsigned short)src_xy_row[x * 2 + 1] < height1 : !pre_inter;
                if (cur_inter == pre_inter) {
                    continue;
                }

                int x1 = x;
                x = x0;
                x0 = x1;
                pre_inter = cur_inter;

                if (!cur_inter) {
                    int dt = x1 - x;
                    int d_align4 = dt & (~3);
                    const short* xy_row = src_xy_row + (x << 1);
                    const short* co_row = coeffs_row + x;
                    float* cur_dst = dst_row + x * channel;
                    int d = 0;
                    for (d = 0; d < d_align4; d += 4) {
                        int d2 = d << 1;
                        const short* cur_coeff = co_row + d;
                        int16x4x2_t xy0 = vld2_s16(xy_row + d2);
                        int16x4_t vx0 = vmul_n_s16(xy0.val[0], channel);

                        int tab_index[] = {cur_coeff[0] << 2, cur_coeff[1] << 2,
                                cur_coeff[2] << 2, cur_coeff[3] << 2};
                        int32x4_t voffset0 = vmovl_s16(vx0);
                        voffset0 = vmlal_s16(voffset0, xy0.val[1], v_src_stride);
                        float32x4x4_t coeff = get_coeffs_f32x4x4(tab, tab_index);

                        float32x4x3_t src_l00 = set_lane_f32x4x3(src0, voffset0);
                        float32x4x3_t src_l10 = set_lane_f32x4x3(src1, voffset0);

                        int32x4_t voffset1 = vaddq_s32(voffset0, v_channel);
                        float32x4x3_t src_l01 = set_lane_f32x4x3(src0, voffset1);
                        float32x4x3_t src_l11 = set_lane_f32x4x3(src1, voffset1);

                        float32x4x3_t vsum0;
                        vsum0.val[0] = vmulq_f32(src_l00.val[0], coeff.val[0]);
                        vsum0.val[1] = vmulq_f32(src_l00.val[1], coeff.val[0]);
                        vsum0.val[2] = vmulq_f32(src_l00.val[2], coeff.val[0]);

                        vsum0.val[0] = vmlaq_f32(vsum0.val[0], src_l01.val[0], coeff.val[1]);
                        vsum0.val[1] = vmlaq_f32(vsum0.val[1], src_l01.val[1], coeff.val[1]);
                        vsum0.val[2] = vmlaq_f32(vsum0.val[2], src_l01.val[2], coeff.val[1]);

                        vsum0.val[0] = vmlaq_f32(vsum0.val[0], src_l10.val[0], coeff.val[2]);
                        vsum0.val[1] = vmlaq_f32(vsum0.val[1], src_l10.val[1], coeff.val[2]);
                        vsum0.val[2] = vmlaq_f32(vsum0.val[2], src_l10.val[2], coeff.val[2]);

                        vsum0.val[0] = vmlaq_f32(vsum0.val[0], src_l11.val[0], coeff.val[3]);
                        vsum0.val[1] = vmlaq_f32(vsum0.val[1], src_l11.val[1], coeff.val[3]);
                        vsum0.val[2] = vmlaq_f32(vsum0.val[2], src_l11.val[2], coeff.val[3]);

                        vst3q_f32(cur_dst + d2 + d, vsum0);
                    }

                    //the rest part
                    for (; d < dt; d++) {
                        int d2 = d << 1;
                        short srcx0 = xy_row[d2];
                        short srcy0 = xy_row[d2 + 1];

                        const float* s0 = src_data + srcy0 * src_stride + srcx0 * channel;
                        const float* s1 = s0 + src_stride;

                        //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                        int tab_offset = (co_row[d] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                        float* tab_ptr = (float *)(tab + tab_offset);
                        float coeffs00 = *(tab_ptr++);
                        float coeffs01 = *(tab_ptr++);
                        float coeffs10 = *(tab_ptr++);
                        float coeffs11 = *(tab_ptr++);

                        for (int ch = 0; ch < channel; ch++) {
                            float src00 = s0[ch];
                            float src01 = s0[channel + ch];
                            float src10 = s1[ch];
                            float src11 = s1[channel + ch];

                            cur_dst[d * channel + ch] = (float)(src00 * coeffs00
                                    + src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11);
                        }
                    }
                    x = x1;
                } else {
                    for (; x < x1; x++) {
                        short srcx0 = src_xy_row[x * 2];
                        short srcx1 = srcx0 + 1;
                        short srcy0 = src_xy_row[x * 2 + 1];
                        short srcy1 = srcy0 + 1;

                        int tab_offset = (coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                        float * tab_ptr = (float *)(tab + tab_offset);

                        float coeffs00 = *(tab_ptr++);
                        float coeffs01 = *(tab_ptr++);
                        float coeffs10 = *(tab_ptr++);
                        float coeffs11 = *(tab_ptr++);

                        bool flag_x0 = get_coord_boundary(srcx0, src_width);
                        bool flag_x1 = get_coord_boundary(srcx1, src_width);
                        bool flag_y0 = get_coord_boundary(srcy0, src_height);
                        bool flag_y1 = get_coord_boundary(srcy1, src_height);

                        const float* p_src00 = (flag_x0 && flag_y0) ?
                                &src_data[srcy0 * src_stride + srcx0 * channel] : &cval[0];
                        const float* p_src01 = (flag_x1 && flag_y0) ?
                                &src_data[srcy0 * src_stride + srcx1 * channel] : &cval[0];
                        const float* p_src10 = (flag_x0 && flag_y1) ?
                                &src_data[srcy1 * src_stride + srcx0 * channel] : &cval[0];
                        const float* p_src11 = (flag_x1 && flag_y1) ?
                                &src_data[srcy1 * src_stride + srcx1 * channel] : &cval[0];

                        for (int ch = 0; ch < channel; ch++) {
                            const float src00 = p_src00[ch];
                            const float src01 = p_src01[ch];
                            const float src10 = p_src10[ch];
                            const float src11 = p_src11[ch];

                            dst_row[x * channel + ch] = (float)(src00 * coeffs00 +
                                    src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11);
                        }
                    }
                }
            }
        }
    }
}

class RemapLinearXConstF32ParallelTask : public ParallelTask {
public:
    RemapLinearXConstF32ParallelTask(
            const Mat& src,
            Mat& dst,
            int block_width,
            int block_height,
            const float* map_x,
            const float* map_y,
            int map_stride,
            float* tab, 
            const Scalar& border_value) :
            _src(src),
            _dst_data(reinterpret_cast<float*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride() / sizeof(float)),
            _block_width(block_width),
            _block_height(block_height),
            _map_x(map_x),
            _map_y(map_y),
            _map_stride(map_stride),
            _tab(tab),
            _border_value(border_value),
            _coeffs_len(block_width * block_height * sizeof(short)),
            _src_xy_len(_coeffs_len << 1) {}

    void operator()(const Range& range) const {
        //creat array map and coeffs to store the remap coordinate matrix and the remap coefficient matrix
        short* src_xy = new short[_src_xy_len];
        short* coeffs = new short[_coeffs_len];

        float32x4_t v_32 = vdupq_n_f32(32.f);
        int32x4_t v_mask = vdupq_n_s32(WARP_INTER_TAB_SIZE - 1);
    
        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);
    
            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);
                int bw_align8 = bw & (~7);
    
                for (int y = 0; y < bh; y++) {
                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;
                    int map_step = y_ * _map_stride;
    
                    const float* sx_row = _map_x + map_step;
                    const float* sy_row = _map_y + map_step;
    
                    int x = 0;
                    for (x = 0; x < bw_align8; x += 8) {
                        int x_ = x + j;
                        int16x4x2_t v_xy0, v_xy1;
                        float32x4_t v_x00 = vld1q_f32(sx_row + x_);
                        float32x4_t v_y00 = vld1q_f32(sy_row + x_);
    
                        int32x4_t v_x = VCVTAQ_S32_F32(vmulq_f32(v_x00, v_32));
                        int32x4_t v_y = VCVTAQ_S32_F32(vmulq_f32(v_y00, v_32));
    
                        //corordiate
                        v_xy0.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy0.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);
    
                        //coefficient
                        int32x4_t v_coeff = vaddq_s32(vshlq_n_s32(vandq_s32(v_y, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(v_x, v_mask));
                        int16x4_t v_coeff_n0 = vmovn_s32(v_coeff);
    
                        v_x00 = vld1q_f32(sx_row + x_ + 4);
                        v_y00 = vld1q_f32(sy_row + x_ + 4);
    
                        v_x = VCVTAQ_S32_F32(vmulq_f32(v_x00, v_32));
                        v_y = VCVTAQ_S32_F32(vmulq_f32(v_y00, v_32));
    
                        //corordiate
                        v_xy1.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy1.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);
    
                        //coefficient
                        v_coeff = vaddq_s32(vshlq_n_s32(vandq_s32(v_y, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(v_x, v_mask));
                        int16x4_t v_coeff_n1 = vmovn_s32(v_coeff);
    
                        //store
                        int16x8x2_t v_xy;
                        v_xy.val[0] = vcombine_s16(v_xy0.val[0], v_xy1.val[0]);
                        v_xy.val[1] = vcombine_s16(v_xy0.val[1], v_xy1.val[1]);
                        vst2q_s16(map_row + (x << 1), v_xy);
                        vst1q_s16(coeffs_row + x, vcombine_s16(v_coeff_n0, v_coeff_n1));
                    }

                    //the rest part
                    for (; x < bw; x++) {
                        int x_ = x + j;
                        int sx = fcv_round(sx_row[x_] * WARP_INTER_TAB_SIZE);
                        int sy = fcv_round(sy_row[x_] * WARP_INTER_TAB_SIZE);
                        short alpha = (short)((sy & (WARP_INTER_TAB_SIZE - 1)) *
                                    WARP_INTER_TAB_SIZE + (sx & (WARP_INTER_TAB_SIZE - 1)));
                        map_row[x * 2] = fcv_cast_s16(sx >> WARP_SCALE_BITS_HALF);
                        map_row[x * 2 + 1] = fcv_cast_s16(sy >> WARP_SCALE_BITS_HALF);
                        coeffs_row[x] = alpha;
                    }
                }
    
                float* dst_ptr = _dst_data + _dst_stride * i + j * _src.channels();
                remap_linear_const_f32_neon(0, bh, bw, _src, dst_ptr, _dst_stride,
                        src_xy, coeffs, _tab, _border_value);
            }
        }

        delete[] src_xy;
        delete[] coeffs;
    }

private:
    const Mat& _src;
    float* _dst_data;
    int _dst_width;
    int _dst_height;
    int _dst_stride;
    int _block_width;
    int _block_height;
    const float* _map_x;
    const float* _map_y;
    int _map_stride;
    float* _tab;
    const Scalar& _border_value;
    int _coeffs_len;
    int _src_xy_len;
};

static void remap_linear_const_f32_neon(
        const Mat& src,
        Mat& dst,
        const float* map_x,
        const float* map_y,
        int map_stride,
        const Scalar border_value) {
    int dst_width = dst.width();
    int dst_height = dst.height();
    int block_height = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_height);
    int block_width = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / block_height, dst_width);

    int tab_len = (AREA_SZ << 2) * sizeof(float);
    float* tab = new float[tab_len];

    //init table 2D for bilinear interploration
    init_table_2d_coeff_f32_neon(tab, WARP_INTER_TAB_SIZE);

    RemapLinearXConstF32ParallelTask task(src, dst, block_width, block_height,
            map_x, map_y, map_stride, tab, border_value);

    if (dst_width > 128 && dst_height > 128) {
        parallel_run(Range(0, dst_height), task);
    } else {
        parallel_run(Range(0, dst_height), task, 1);
    }

    delete[] tab;
}

class RemapLinearXConstU8ParallelTask : public ParallelTask {
public:
    RemapLinearXConstU8ParallelTask(
            const Mat& src,
            Mat& dst,
            const float* sx,
            const float* sy,
            int map_stride,
            int block_width,
            int block_height,
            short* tab,
            const Scalar border_value,
            bool is_planar) :
            _src(src),
            _dst_data(reinterpret_cast<unsigned char*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride()),
            _sx(sx),
            _sy(sy),
            _map_stride(map_stride),
            _block_width(block_width),
            _block_height(block_height),
            _tab(tab),
            _border_value(border_value),
            _is_planar(is_planar),
            _coeffs_len(block_width * block_height * sizeof(short)),
            _src_xy_len(2 * _coeffs_len) {}

    void operator()(const Range& range) const {
        short* coeffs = (short *)malloc(_coeffs_len);
        short* src_xy = (short *)malloc(_src_xy_len);

        float32x4_t v_32 = vdupq_n_f32(32.f);
        int32x4_t v_mask = vdupq_n_s32(WARP_INTER_TAB_SIZE - 1);

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);
    
            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);
                int bw_align8 = bw & (~7);
    
                for (int y = 0; y < bh; y++) {
                    short* map_row    = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;
                    int map_step = y_ * _map_stride;
    
                    const float* sx_row = _sx + map_step;
                    const float* sy_row = _sy + map_step;
    
                    int x = 0;
                    for (x = 0; x < bw_align8; x += 8) {
                        int x_ = x + j;
                        int16x4x2_t v_xy0, v_xy1;
                        float32x4_t v_x00 = vld1q_f32(sx_row + x_);
                        float32x4_t v_y00 = vld1q_f32(sy_row + x_);
    
                        int32x4_t v_x = VCVTAQ_S32_F32(vmulq_f32(v_x00, v_32));
                        int32x4_t v_y = VCVTAQ_S32_F32(vmulq_f32(v_y00, v_32));
    
                        //corordiate
                        v_xy0.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy0.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);

                        //coefficient
                        int32x4_t v_coeff = vaddq_s32(vshlq_n_s32(vandq_s32(v_y, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(v_x, v_mask));
                        int16x4_t v_coeff_n0 = vmovn_s32(v_coeff);
    
                        v_x00 = vld1q_f32(sx_row + x_ + 4);
                        v_y00 = vld1q_f32(sy_row + x_ + 4);
    
                        v_x = VCVTAQ_S32_F32(vmulq_f32(v_x00, v_32));
                        v_y = VCVTAQ_S32_F32(vmulq_f32(v_y00, v_32));
    
                        //corordiate
                        v_xy1.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy1.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);
    
                        //coefficient
                        v_coeff = vaddq_s32(vshlq_n_s32(vandq_s32(v_y, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(v_x, v_mask));
                        int16x4_t v_coeff_n1 = vmovn_s32(v_coeff);
    
                        //store
                        int16x8x2_t v_xy;
                        v_xy.val[0] = vcombine_s16(v_xy0.val[0], v_xy1.val[0]);
                        v_xy.val[1] = vcombine_s16(v_xy0.val[1], v_xy1.val[1]);
                        vst2q_s16(map_row + (x << 1), v_xy);
                        vst1q_s16(coeffs_row + x, vcombine_s16(v_coeff_n0, v_coeff_n1));
                    }
                    //the rest part
                    for (; x < bw; x++) {
                        int x_ = x + j;
                        int sx = fcv_round(sx_row[x_] * WARP_INTER_TAB_SIZE);
                        int sy = fcv_round(sy_row[x_] * WARP_INTER_TAB_SIZE);
                        short alpha = (short)((sy & (WARP_INTER_TAB_SIZE - 1)) *
                                    WARP_INTER_TAB_SIZE + (sx & (WARP_INTER_TAB_SIZE - 1)));
                        map_row[x * 2] = fcv_cast_s16(sx >> WARP_SCALE_BITS_HALF);
                        map_row[x * 2 + 1] = fcv_cast_s16(sy >> WARP_SCALE_BITS_HALF);
                        coeffs_row[x] = alpha;
                    }
                }
    
                unsigned char* dst_ptr = _dst_data + _dst_stride * i + j * _src.channels();

                if (_is_planar) {
                    remap_linear_const_planar_u8_neon(0, bh, _src, dst_ptr, _dst_width,
                            _dst_height, _dst_stride, src_xy, coeffs, _tab, _border_value);
                } else {
                    remap_linear_const_u8_neon(0, bh, bw, _src, dst_ptr,
                            _dst_stride, src_xy, coeffs, _tab, _border_value);
                }
            }
        }

        free(src_xy);
        free(coeffs);
    } 
    
private:
    const Mat& _src;
    unsigned char* _dst_data;
    int _dst_width;
    int _dst_height;
    int _dst_stride;
    const float* _sx;
    const float* _sy;
    int _map_stride;
    int _block_width;
    int _block_height;
    short* _tab;
    const Scalar _border_value;
    bool _is_planar;
    int _coeffs_len;
    int _src_xy_len;
};

static void remap_linear_const_u8_neon(
        const Mat& src,
        Mat& dst,
        const float *sx,
        const float *sy,
        int map_stride,
        const Scalar border_value,
        int is_planar) {
    int dst_width = dst.width();
    int dst_height = dst.height();
    int block_height = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_height);
    int block_width = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / block_height, dst_width);

    int tab_len = (AREA_SZ << 2) * sizeof(short);
    short* tab = new short[tab_len];

    init_table_2d_coeff_u8_neon(tab, WARP_INTER_TAB_SIZE);

    RemapLinearXConstU8ParallelTask task(src, dst, sx, sy, map_stride,
            block_width, block_height, tab, border_value, is_planar);

    if (dst_width > 128 && dst_height > 128) {
        parallel_run(Range(0, dst.height()), task);
    } else {
        parallel_run(Range(0, dst.height()), task, 1);
    }

    delete[] tab;
}

static void remap_linear_const_neon(
        const Mat& src,
        Mat& dst,
        const Mat& map0,
        const Mat& map1,
        const Scalar border_value) {
    float *map_x = (float *)map0.data();
    float *map_y = (float *)map1.data();

    int map_stride = map0.stride() >> 2;

    FCVImageType type = src.type();

    if (type == FCVImageType::GRAY_U8
            || type == FCVImageType::PKG_BGR_U8
            || type == FCVImageType::PKG_RGB_U8) {
        remap_linear_const_u8_neon(src, dst, map_x, map_y, map_stride, border_value, 0);
    } else if (type == FCVImageType::GRAY_F32
            || type == FCVImageType::PKG_BGR_F32
            || type == FCVImageType::PKG_RGB_F32) {
        remap_linear_const_f32_neon(src, dst, map_x, map_y, map_stride, border_value);
    } else if (type == FCVImageType::PLA_BGR_U8
            || type == FCVImageType::PLA_RGB_U8) {
        remap_linear_const_u8_neon(src, dst, map_x, map_y, map_stride, border_value, 1);
    } else {
        LOG_ERR("remap data type not support yet!");
    }
}

int remap_linear_neon(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        BorderTypes border_method,
        const Scalar border_value) {
    if ((map1.type() == FCVImageType::GRAY_F32)
            && (map2.type() == FCVImageType::GRAY_F32)){
        if (dst.empty()) {
            dst = Mat(map1.width(), map1.height(), src.type());
        }
        if (dst.width() != map1.width() || dst.height() != map1.height() || dst.type() != src.type()) {
            LOG_ERR("illegal size or type of dst mat to remap_linear!");
            return -1;
        }

        switch (border_method) {
        case BorderTypes::BORDER_CONSTANT:
            remap_linear_const_neon(src, dst, map1, map2, border_value);
            break;
        default:
            LOG_ERR("remap interpolation type not support yet!");
        break;
        };
    } else {
        LOG_ERR("remap map1 and map2 type should be F32 or S16!");
        return -1;
    }

    return 0;
}

int remap_neon(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        InterpolationType inter_type,
        BorderTypes border_method,
        const Scalar border_value) {
    if ((src.empty()) || (map1.empty()) || (map2.empty())) {
        LOG_ERR("null ptr in remap!");
        return -1;
    }

    switch (inter_type) {
    case InterpolationType::INTER_LINEAR:
        remap_linear_neon(src, dst, map1, map2, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_affine interpolation type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
