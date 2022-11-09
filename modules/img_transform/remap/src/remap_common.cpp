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

#include <cfloat>
#include <cmath>
#include <stdlib.h>

#include "modules/img_transform/remap/include/remap_common.h"
#include "modules/core/base/interface/basic_types.h"
#include "modules/core/basic_math/interface/basic_math.h"
#include "modules/img_transform/rotation/interface/rotation.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int inverse_matrix_2x3_float(const double* src_mat, double* dst_mat) {
    if ((nullptr == src_mat) || (nullptr == dst_mat)) {
        LOG_ERR("Mat is empty!");
        return -1;
    }

    double d = src_mat[0] * src_mat[4] - src_mat[1] * src_mat[3];
    d = (fabs(d) < FCV_EPSILON) ? 0.0f : 1.0f / d;
    double a11 = src_mat[4] * d, a22 = src_mat[0] * d;
    dst_mat[0] = a11;
    dst_mat[1] = src_mat[1] * (-d);
    dst_mat[3] = src_mat[3] * (-d);
    dst_mat[4] = a22;
    double b1 = -dst_mat[0] * src_mat[2] - dst_mat[1] * src_mat[5];
    double b2 = -dst_mat[3] * src_mat[2] - dst_mat[4] * src_mat[5];
    dst_mat[2] = b1;
    dst_mat[5] = b2;

    return 0;
}

static inline bool get_coord_boundary(const int x, const int size) {
    return ((x >= 0) && (x < size));
}

// fixed-point for u8: calculate adjacent four points coefficient: x0 x1 y0 y1->x0*y0, x1*y0, x0*y1, x1*y1
void init_table_2d_coeff_u8(short *tab, int tabsz) {
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

            //the fixed-point scale is 2^15
            tab[0] = fcv_cast_s16(fcv_round(uy0 * vx0 * (1 << WARP_SCALE_REMAP_BITS)));
            tab[1] = fcv_cast_s16(fcv_round(uy0 * vx1 * (1 << WARP_SCALE_REMAP_BITS)));
            tab[2] = fcv_cast_s16(fcv_round(uy1 * vx0 * (1 << WARP_SCALE_REMAP_BITS)));
            tab[3] = fcv_cast_s16(fcv_round(uy1 * vx1 * (1 << WARP_SCALE_REMAP_BITS)));

            tab += 4;

        }
    }
}

// non-fixed-point for f32: calculate adjacent four points coefficient: x0 x1 y0 y1->x0*y0, x1*y0, x0*y1, x1*y1
void init_table_2d_coeff_f32(float *tab, int tabsz) {
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
        const Scalar border_value) {
    int width1 = src_width - 2;
    int height1 = src_height - 2;

    unsigned char cval[4];

    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<unsigned char>(border_value.val()[k]);
    }

    for (int y = 0; y < height; y++) {
        bool pre_inter = false;
        int offset = width * y;
        short * __restrict src_xy_row = (short *)(src_xy + (offset << 1));
        short * __restrict coeffs_row = (short *)(coeffs + offset);

        unsigned char*  dst_row = (unsigned char *)(dst + dst_stride * y);
        int x0 = 0;

        for (int x = 0; x <= width; x++) {
            /* compute the x & y coordinates which are always in the area of [0 0 width height],
            in the circumstances of value of src_xy_row is negative, convert it to unsigned short, 
            the value will the curInlier will set to 0, and skip out*/
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
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x * 2];
                    short srcy0 = src_xy_row[x * 2 + 1];

                    const unsigned char* s0 = src + srcy0 * src_stride + srcx0 * channel;
                    const unsigned char* s1 = s0 + src_stride;

                    //because the tab size is 32x32(2^10), by calculating the remainder of dividing by 2^10 to get the coeff value,
                    // multiply by 4 is because there are 4 coefficients, (∆x * ∆y * WARP_SCALE_REMAP_BITS, ∆x * (1-∆y) * WARP_SCALE_REMAP_BITS,
                    // (1-∆x) * ∆y * WARP_SCALE_REMAP_BITS, (1-∆x) * (1-∆y) * WARP_SCALE_REMAP_BITS
                    int tab_offset = ((coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2);
                    short* tab_ptr = (short *)(tab + tab_offset);
                    short coeffs00 = *(tab_ptr++);
                    short coeffs01 = *(tab_ptr++);
                    short coeffs10 = *(tab_ptr++);
                    short coeffs11 = *(tab_ptr++);

                    for (int ch = 0; ch < channel; ch++) {
                        unsigned char src00 = s0[ch];
                        unsigned char src01 = s0[channel + ch];
                        unsigned char src10 = s1[ch];
                        unsigned char src11 = s1[channel + ch];

                        dst_row[x * channel + ch] = fcv_cast_u8((src00 * coeffs00 +
                                src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                                (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    }
                }
            } else {
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x*2];
                    short srcx1 = srcx0 + 1;
                    short srcy0 = src_xy_row[x*2+1];
                    short srcy1 = srcy0 + 1;

                    int tab_offset = ((coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2);

                    short *tab_ptr = (short *)(tab + tab_offset);
                    short coeffs00 = *(tab_ptr++);
                    short coeffs01 = *(tab_ptr++);
                    short coeffs10 = *(tab_ptr++);
                    short coeffs11 = *(tab_ptr++);

                    bool flag_x0 = get_coord_boundary(srcx0, src_width);
                    bool flag_x1 = get_coord_boundary(srcx1, src_width);
                    bool flag_y0 = get_coord_boundary(srcy0, src_height);
                    bool flag_y1 = get_coord_boundary(srcy1, src_height);

                    const unsigned char* p_src00 = (flag_x0 && flag_y0) ?
                            &src[srcy0 * src_stride + srcx0 * channel] : &cval[0];
                    const unsigned char* p_src01 = (flag_x1 && flag_y0) ?
                            &src[srcy0 * src_stride + srcx1 * channel] : &cval[0];
                    const unsigned char* p_src10 = (flag_x0 && flag_y1) ?
                            &src[srcy1 * src_stride + srcx0 * channel] : &cval[0];
                    const unsigned char* p_src11 = (flag_x1 && flag_y1) ?
                            &src[srcy1 * src_stride + srcx1 * channel] : &cval[0];

                    for (int ch = 0; ch < channel; ch++) {
                        const unsigned char src00 = p_src00[ch];
                        const unsigned char src01 = p_src01[ch];
                        const unsigned char src10 = p_src10[ch];
                        const unsigned char src11 = p_src11[ch];

                        dst_row[x * channel + ch] = fcv_cast_u8((src00 * coeffs00 +
                                src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11 +
                                (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    }
                }
            }
        }
    }
}

void remap_linear_planar_u8_const(
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
        const Scalar border_value) {
    int width1 = src_width - 2;
    int height1 = src_height - 2;

    unsigned char cval[4];
    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<unsigned char>(border_value.val()[k]);
    }

    unsigned char* dst_b = (unsigned char *)dst;
    unsigned char* dst_g = (unsigned char *)dst_b + height * width;
    unsigned char* dst_r = (unsigned char *)dst_g + height * width;

    unsigned char* src_b = (unsigned char *)src;
    unsigned char* src_g = (unsigned char *)src_b + src_height * src_width;
    unsigned char* src_r = (unsigned char *)src_g + src_height * src_width;

    for (int y = 0; y < height; y++) {
        bool pre_inter = false;
        int offset = width * y;
        short* __restrict src_xy_row = (short *)(src_xy + (offset << 1));
        short* __restrict coeffs_row = (short *)(coeffs + offset);

        unsigned char* dstb_row = (unsigned char *)(dst_b + dst_stride * y);
        unsigned char* dstg_row = (unsigned char *)(dst_g + dst_stride * y);
        unsigned char* dstr_row = (unsigned char *)(dst_r + dst_stride * y);
        int x0 = 0;

        for (int x = 0; x <= width; x++) {
            /* compute the x & y coordinates which are always in the area of [0 0 width height],
            in the circumstances of value of src_xy_row is negative, convert it to unsigned short,
            the value will the curInlier will set to 0, and skip out*/
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
                for(; x < x1; x++) {
                    short srcx0 = src_xy_row[x * 2];
                    short srcy0 = src_xy_row[x * 2 + 1];

                    const unsigned char* s_b0 = src_b + srcy0 * src_stride + srcx0;
                    const unsigned char* s_b1 = s_b0 + src_stride;

                    const unsigned char* s_g0 = src_g + srcy0 * src_stride + srcx0;
                    const unsigned char* s_g1 = s_g0 + src_stride;

                    const unsigned char* s_r0 = src_r + srcy0 * src_stride + srcx0;
                    const unsigned char* s_r1 = s_r0 + src_stride;

                    //because the tab size is 32x32(2^10), by calculating the remainder of dividing by 2^10 to get the coeff value,
                    // multiply by 4 is because there are 4 coefficients, (∆x * ∆y * WARP_SCALE_REMAP_BITS, ∆x * (1-∆y) * WARP_SCALE_REMAP_BITS,
                    // (1-∆x) * ∆y * WARP_SCALE_REMAP_BITS, (1-∆x) * (1-∆y) * WARP_SCALE_REMAP_BITS
                    int tab_offset = ((coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2);
                    short * tab_ptr = (short *)(tab + tab_offset);
                    short coeffs00 = *(tab_ptr++);
                    short coeffs01 = *(tab_ptr++);
                    short coeffs10 = *(tab_ptr++);
                    short coeffs11 = *(tab_ptr++);

                    dstb_row[x] = fcv_cast_u8((s_b0[0] * coeffs00 +
                            s_b1[0] * coeffs10 + s_b0[1] * coeffs01 + s_b1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    dstg_row[x] = fcv_cast_u8((s_g0[0] * coeffs00 +
                            s_g1[0] * coeffs10 + s_g0[1] * coeffs01 + s_g1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                    dstr_row[x] = fcv_cast_u8((s_r0[0] * coeffs00 +
                            s_r1[0] * coeffs10 + s_r0[1] * coeffs01 + s_r1[1] * coeffs11 +
                            (1 << (WARP_SCALE_REMAP_BITS - 1))) >> (WARP_SCALE_REMAP_BITS));
                }
            } else {
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x*2];
                    short srcx1 = srcx0 + 1;
                    short srcy0 = src_xy_row[x*2+1];
                    short srcy1 = srcy0 + 1;

                    int tab_offset = ((coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2);

                    short *tab_ptr = (short *)(tab + tab_offset);
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
        const Scalar border_value) {
    int width1 = src_width - 2;
    int height1 = src_height - 2;

    float cval[4];
    for (int k = 0; k < channel; k++) {
        cval[k] = static_cast<float>(border_value.val()[k]);
    }

    for (int y = 0; y < height; y++) {
        bool pre_inter = false;
        int offset = width * y;
        short* __restrict src_xy_row = (short *)(src_xy + (offset << 1));
        short* __restrict coeffs_row = (short *)(coeffs + offset);
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
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x * 2];
                    short srcy0 = src_xy_row[x * 2 + 1];

                    const float* s0 = src + srcy0 * src_stride + srcx0 * channel;
                    const float* s1 = s0 + src_stride;

                    //becase the coordinates is fixed with the scale 2^10, so there should shift left by 10
                    int tab_offset = (coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2;
                    float * tab_ptr = (float *)(tab + tab_offset);
                    float coeffs00 = *(tab_ptr++);
                    float coeffs01 = *(tab_ptr++);
                    float coeffs10 = *(tab_ptr++);
                    float coeffs11 = *(tab_ptr++);

                    for (int ch = 0; ch < channel; ch++) {
                        float src00 = s0[ch];
                        float src01 = s0[channel + ch];
                        float src10 = s1[ch];
                        float src11 = s1[channel + ch];

                        dst_row[x * channel + ch] = (float)((src00 * coeffs00 +
                                src10 * coeffs10 + src01 * coeffs01 + src11 * coeffs11));
                    }
                }
            } else {
                for (; x < x1; x++) {
                    short srcx0 = src_xy_row[x * 2];
                    short srcy0 = src_xy_row[x * 2 + 1];
                    short srcx1 = srcx0 + 1;
                    short srcy1 = srcy0 + 1;

                    int tab_offset = (coeffs_row[x] & ((1 << WARP_SCALE_BITS) - 1)) << 2;

                    float* tab_ptr = (float *)(tab + tab_offset);
                    float coeffs00 = *(tab_ptr++);
                    float coeffs01 = *(tab_ptr++);
                    float coeffs10 = *(tab_ptr++);
                    float coeffs11 = *(tab_ptr++);

                    bool flag_x0 = get_coord_boundary(srcx0, src_width);
                    bool flag_x1 = get_coord_boundary(srcx1, src_width);
                    bool flag_y0 = get_coord_boundary(srcy0, src_height);
                    bool flag_y1 = get_coord_boundary(srcy1, src_height);

                    const float* p_src00 = (flag_x0 && flag_y0) ?
                            &src[srcy0 * src_stride + srcx0 * channel] : &cval[0];
                    const float* p_src01 = (flag_x1 && flag_y0) ?
                            &src[srcy0 * src_stride + srcx1 * channel] : &cval[0];
                    const float* p_src10 = (flag_x0 && flag_y1) ?
                            &src[srcy1 * src_stride + srcx0 * channel] : &cval[0];
                    const float* p_src11 = (flag_x1 && flag_y1) ?
                            &src[srcy1 * src_stride + srcx1 * channel] : &cval[0];

                    for (int ch = 0; ch < channel; ch++) {
                        const float src00 = p_src00[ch];
                        const float src01 = p_src01[ch];
                        const float src10 = p_src10[ch];
                        const float src11 = p_src11[ch];

                        dst_row[x * channel + ch] = (float)(src00 * coeffs00 + src10 * coeffs10 +
                                src01 * coeffs01 + src11 * coeffs11);
                    }
                }
            }
        }
    }
}

static void remap_linear_x_const_f32_common(
        const float *src,
        const int src_width,
        const int src_height,
        const int src_stride,
        float *dst,
        const int dst_width,
        const int dst_height,
        const int dst_stride,
        const float *sx,
        const float *sy,
        int sxy_stride,
        const int channel,
        const Scalar border_value) {
    int bh0 = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_height);
    int bw0 = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / bh0, dst_width);

    int tab_len = (AREA_SZ << 2) * sizeof(float);
    int src_xy_len = (bh0 * bw0 * 2) * sizeof(short);
    int coeffs_len = (AREA_SZ) * sizeof(short);

    float *tab = (float *)malloc(tab_len); //store the coefficient four points coordinata
    short *src_xy = (short *)malloc(src_xy_len);
    short *coeffs = (short *)malloc(coeffs_len);

    memset(tab, 0, tab_len);
    memset(src_xy, 0, src_xy_len);
    memset(coeffs, 0, coeffs_len);

    init_table_2d_coeff_f32(tab, WARP_INTER_TAB_SIZE);

    int i = 0, j = 0;
    for (i = 0; i < dst_height; i += bh0) {
        int bh = FCV_MIN(bh0, dst_height - i);
        float* ptr_dst = (float *)(dst + dst_stride * i);

        for (j = 0; j < dst_width; j += bw0) {
            int bw = FCV_MIN(bw0, dst_width - j);
            float* dst_ptr = (float *)(ptr_dst + j * channel);

            for (int y = 0; y < bh; y++) {
                short* map_row = (short *)(src_xy + (bw * (y << 1)));
                short* coeffs_row = (short *)(coeffs + (bw * y));
                int y_ = y + i;

                const float* sx_row = sx + y_ * sxy_stride;
                const float* sy_row = sy + y_ * sxy_stride;
                for (int x = 0 ; x < bw; x++ ) {
                    int x_ = x + j;
                    int ssx = fcv_round(sx_row[x_] * WARP_INTER_TAB_SIZE);
                    int ssy = fcv_round(sy_row[x_] * WARP_INTER_TAB_SIZE);

                    int alpha = (int)((ssy & (WARP_INTER_TAB_SIZE - 1)) *
                            WARP_INTER_TAB_SIZE + (ssx & (WARP_INTER_TAB_SIZE - 1)));
                    map_row[x * 2] = fcv_cast_s16(ssx >> WARP_SCALE_BITS_HALF);
                    map_row[x * 2 + 1] = fcv_cast_s16(ssy >> WARP_SCALE_BITS_HALF);
                    coeffs_row[x] = (short)alpha;
                }
            }

            remap_linear_f32_const(bh, bw, src, src_width, src_height, src_stride,
                    dst_ptr, dst_stride, src_xy, coeffs, channel, tab, border_value);
        }
    }

    //free
    free(tab);
    free(src_xy);
    free(coeffs);
}

static void remap_linear_x_const_u8_common(
        const unsigned char *src,
        const int src_width,
        const int src_height,
        const int src_stride,
        unsigned char *dst,
        const int dst_width,
        const int dst_height,
        const int dst_stride,
        const float *sx,
        const float *sy,
        int sxy_stride,
        const int channel,
        const Scalar border_value,
        int is_planar) {
    int bh0 = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_height);
    int bw0 = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / bh0, dst_width);

    int tab_len = (AREA_SZ << 2) * sizeof(short);
    int src_xy_len = bh0 * bw0  * 2 * sizeof(short);
    int coeffs_len = AREA_SZ * sizeof(short);

    short *tab = (short *)malloc(tab_len); //store the coefficient four points coordinata
    short *src_xy = (short *)malloc(src_xy_len);
    short *coeffs = (short *)malloc(coeffs_len);

    memset(tab, 0, tab_len);
    memset(src_xy, 0, src_xy_len);
    memset(coeffs, 0, coeffs_len);

    init_table_2d_coeff_u8(tab, WARP_INTER_TAB_SIZE);

    int i = 0, j = 0;
    for (i = 0; i < dst_height; i += bh0) {
        int bh = FCV_MIN(bh0, dst_height - i);
        unsigned char* ptr_dst = (unsigned char *)(dst + dst_stride * i);

        for (j = 0; j < dst_width; j += bw0) {
            int bw = FCV_MIN(bw0, dst_width - j);
            unsigned char* dst_ptr = (unsigned char *)(ptr_dst + j * channel);

            for (int y = 0; y < bh; y++) {
                short* map_row    = (short *)(src_xy + (bw * (y << 1)));
                short* coeffs_row = (short *)(coeffs + (bw * y));
                int y_ = y + i;

                const float* sx_row = sx + y_ * sxy_stride;
                const float* sy_row = sy + y_ * sxy_stride;
                for (int x = 0 ; x < bw; x++) {
                    int x_ = x + j;
                    int ssx = fcv_round(sx_row[x_] * WARP_INTER_TAB_SIZE);
                    int ssy = fcv_round(sy_row[x_] * WARP_INTER_TAB_SIZE);

                    int alpha = (int)((ssy & (WARP_INTER_TAB_SIZE - 1)) *
                            WARP_INTER_TAB_SIZE + (ssx & (WARP_INTER_TAB_SIZE - 1)));
                    map_row[x * 2] = fcv_cast_s16(ssx >> WARP_SCALE_BITS_HALF);
                    map_row[x * 2 + 1] = fcv_cast_s16(ssy >> WARP_SCALE_BITS_HALF);
                    coeffs_row[x] = (short)alpha;
                }
            }

            if (is_planar) {
                remap_linear_planar_u8_const(bh, bw, src, src_width,
                        src_height, src_stride, dst_ptr, dst_stride,
                        src_xy, coeffs, channel, tab, border_value);
            } else {
                remap_linear_u8_const(bh, bw, src, src_width, src_height,
                        src_stride, dst_ptr, dst_stride, src_xy, coeffs,
                        channel, tab, border_value);
            }
        }
    }

    //free
    free(tab);
    free(src_xy);
    free(coeffs);
}

static void remap_linear_x_const_common(
        const Mat& src,
        Mat& dst,
        const Mat& map0,
        const Mat& map1,
        const Scalar border_value) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    void *src_data = (void *)src.data();
    void *dst_data = (void *)dst.data();
    float *map_x = (float *)map0.data();
    float *map_y = (float *)map1.data();

    int sc = src.channels();
    int src_stride = src.stride();
    int dst_stride = dst.stride();
    int map_stride = map0.stride() >> 2;

    FCVImageType type = src.type();

    if (type == FCVImageType::GRAY_U8
            || type == FCVImageType::PKG_BGR_U8
            || type == FCVImageType::PKG_RGB_U8) {
        remap_linear_x_const_u8_common((unsigned char*)src_data, src_w,
                src_h, src_stride, (unsigned char*)dst_data, dst_w, dst_h, dst_stride,
                map_x, map_y, map_stride, sc, border_value, 0);

    } else if (type == FCVImageType::GRAY_F32
            || type == FCVImageType::PKG_BGR_F32
            || type == FCVImageType::PKG_RGB_F32) {
        remap_linear_x_const_f32_common((float*)src_data, src_w,
                src_h, src_stride / sizeof(float), (float*)dst_data, dst_w, dst_h,
                dst_stride / sizeof(float), map_x, map_y, map_stride, sc, border_value);
    } else if (type == FCVImageType::PLA_BGR_U8
            || type == FCVImageType::PLA_RGB_U8) {
        remap_linear_x_const_u8_common((unsigned char*)src_data, src_w, src_h,
                src_stride, (unsigned char*)dst_data, dst_w, dst_h, dst_stride,
                map_x, map_y, map_stride, sc, border_value, 1);
    } else {
        LOG_ERR("remap data type not support yet!");
    }
}

int remap_linear_common(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        BorderTypes border_method,
        const Scalar border_value) {
    if ((map1.type() == FCVImageType::GRAY_F32)
            && (map2.type() == FCVImageType::GRAY_F32)) {
        if (dst.empty()) {
            dst = Mat(map1.width(), map1.height(), src.type());
        }
        if (dst.width() != map1.width() || dst.height() != map1.height() || dst.type() != src.type()) {
            LOG_ERR("illegal size or type of dst mat to remap_linear!");
            return -1;
        }

        switch (border_method) {
        case BorderTypes::BORDER_CONSTANT:
            remap_linear_x_const_common(src, dst, map1, map2, border_value);
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

int remap_common(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        InterpolationType inter_type,
        BorderTypes border_method,
        const Scalar border_value) {
    switch (inter_type) {
    case InterpolationType::INTER_LINEAR:
        remap_linear_common(src, dst, map1, map2, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_affine interpolation type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
