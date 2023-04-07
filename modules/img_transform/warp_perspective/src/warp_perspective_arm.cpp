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

#include "modules/img_transform/warp_affine/include/warp_affine_arm.h"
#include "modules/img_transform/warp_affine/include/warp_affine_common.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/warp_perspective/include/warp_perspective_common.h"
#include "modules/img_transform/remap/include/remap_common.h"
#include "modules/img_transform/remap/include/remap_arm.h"
#include "modules/core/base/include/macro_utils.h"

#include <stdlib.h>
#include <thread>

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class WarpPerpectiveLinearU8ConstParallelTask : public ParallelTask {
public:
    WarpPerpectiveLinearU8ConstParallelTask(
            const Mat& src,
            Mat& dst,
            double* x_delta,
            double* y_delta,
            int block_width,
            int block_height,
            short* tab,
            const Scalar& border_value) :
            _src(src),
            _dst_data(reinterpret_cast<unsigned char*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride()),
            _x_delta(x_delta),
            _y_delta(y_delta),
            _block_width(block_width),
            _block_height(block_height),
            _tab(tab),
            _border_value(border_value),
            _coeffs_len(block_width * block_height * sizeof(short)),
            _src_xy_len(_coeffs_len << 1) {}

    void operator()(const Range& range) const override {
        short* src_xy = new short[_src_xy_len];
        short* coeffs = new short[_coeffs_len];

        double* x00 = _x_delta;
        double* x10 = _x_delta + _dst_width;
        double* x20 = _x_delta + _dst_width * 2;
        double* y01 = _y_delta;
        double* y11 = _y_delta + _dst_height;
        double* y21 = _y_delta + _dst_height * 2;

#ifdef __aarch64__
        float64x2_t v_zero = vdupq_n_f64(0);
        float64x2_t vpos = vdupq_n_f64(0.5);
        float64x2_t vneg = vdupq_n_f64(-0.5);
        int32x4_t v_mask = vdupq_n_s32(WARP_INTER_TAB_SIZE - 1);
#endif

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);

            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);

#ifdef __aarch64__
                int bw_align8 = bw & (~7);
#endif

                for (int y = 0; y < bh; y++) {
                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;
#ifdef __aarch64__
                    float64x2_t v_y01 = vdupq_n_f64(y01[y_]);
                    float64x2_t v_y11 = vdupq_n_f64(y11[y_]);
#endif

                    int x = 0;
#ifdef __aarch64__
                    for (x = 0; x < bw_align8; x += 8) {
                        int x_ = x + j;

                        double W[8] = {0};
                        for (int k = 0; k < 8; k++) {
                            double tmp_w = y21[y_] + x20[x_ + k];
                            tmp_w = tmp_w ? WARP_INTER_TAB_SIZE / tmp_w : 0;
                            W[k] = tmp_w;
                        }
                        float64x2_t W_vec0 = vld1q_f64(W);
                        float64x2_t W_vec1 = vld1q_f64(W + 2);
                        float64x2_t W_vec2 = vld1q_f64(W + 4);
                        float64x2_t W_vec3 = vld1q_f64(W + 6);

                        float64x2_t fX_vec0 = vmulq_f64(W_vec0, vaddq_f64(vld1q_f64(x00 + x_), v_y01));
                        float64x2_t fX_vec1 = vmulq_f64(W_vec1, vaddq_f64(vld1q_f64(x00 + x_ + 2), v_y01));
                        float64x2_t fX_vec2 = vmulq_f64(W_vec2, vaddq_f64(vld1q_f64(x00 + x_ + 4), v_y01));
                        float64x2_t fX_vec3 = vmulq_f64(W_vec3, vaddq_f64(vld1q_f64(x00 + x_ + 6), v_y01));

                        float64x2_t fY_vec0 = vmulq_f64(W_vec0, vaddq_f64(vld1q_f64(x10 + x_), v_y11));
                        float64x2_t fY_vec1 = vmulq_f64(W_vec1, vaddq_f64(vld1q_f64(x10 + x_ + 2), v_y11));
                        float64x2_t fY_vec2 = vmulq_f64(W_vec2, vaddq_f64(vld1q_f64(x10 + x_ + 4), v_y11));
                        float64x2_t fY_vec3 = vmulq_f64(W_vec3, vaddq_f64(vld1q_f64(x10 + x_ + 6), v_y11));          
                        
                        fX_vec0 = vbslq_f64(vcgeq_f64(fX_vec0, v_zero), vaddq_f64(fX_vec0, vpos), vaddq_f64(fX_vec0, vneg));
                        fX_vec1 = vbslq_f64(vcgeq_f64(fX_vec1, v_zero), vaddq_f64(fX_vec1, vpos), vaddq_f64(fX_vec1, vneg));
                        fX_vec2 = vbslq_f64(vcgeq_f64(fX_vec2, v_zero), vaddq_f64(fX_vec2, vpos), vaddq_f64(fX_vec2, vneg));
                        fX_vec3 = vbslq_f64(vcgeq_f64(fX_vec3, v_zero), vaddq_f64(fX_vec3, vpos), vaddq_f64(fX_vec3, vneg));

                        fY_vec0 = vbslq_f64(vcgeq_f64(fY_vec0, v_zero), vaddq_f64(fY_vec0, vpos), vaddq_f64(fY_vec0, vneg));
                        fY_vec1 = vbslq_f64(vcgeq_f64(fY_vec1, v_zero), vaddq_f64(fY_vec1, vpos), vaddq_f64(fY_vec1, vneg));
                        fY_vec2 = vbslq_f64(vcgeq_f64(fY_vec2, v_zero), vaddq_f64(fY_vec2, vpos), vaddq_f64(fY_vec2, vneg));
                        fY_vec3 = vbslq_f64(vcgeq_f64(fY_vec3, v_zero), vaddq_f64(fY_vec3, vpos), vaddq_f64(fY_vec3, vneg));
                        
                        int32x4_t X_vec0 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fX_vec0)), vqmovn_s64(vcvtq_s64_f64(fX_vec1)));
                        int32x4_t X_vec1 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fX_vec2)), vqmovn_s64(vcvtq_s64_f64(fX_vec3)));
                        int32x4_t Y_vec0 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fY_vec0)), vqmovn_s64(vcvtq_s64_f64(fY_vec1)));
                        int32x4_t Y_vec1 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fY_vec2)), vqmovn_s64(vcvtq_s64_f64(fY_vec3)));

                        //coefficient
                        int32x4_t v_coeff0 = vaddq_s32(vshlq_n_s32(vandq_s32(Y_vec0, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(X_vec0, v_mask));
                        int32x4_t v_coeff1 = vaddq_s32(vshlq_n_s32(vandq_s32(Y_vec1, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(X_vec1, v_mask));
                        int16x4_t v_coeff_n0 = vmovn_s32(v_coeff0);
                        int16x4_t v_coeff_n1 = vmovn_s32(v_coeff1);

                        vst1q_s16(coeffs_row + x, vcombine_s16(v_coeff_n0, v_coeff_n1));

                        int16x8x2_t v_xy;
                        v_xy.val[0] = vcombine_s16(vqshrn_n_s32(X_vec0, WARP_SCALE_BITS_HALF), \
                            vqshrn_n_s32(X_vec1, WARP_SCALE_BITS_HALF));
                        v_xy.val[1] = vcombine_s16(vqshrn_n_s32(Y_vec0, WARP_SCALE_BITS_HALF), \
                            vqshrn_n_s32(Y_vec1, WARP_SCALE_BITS_HALF));

                        vst2q_s16(map_row + (x << 1), v_xy);
                    }

                    map_row = map_row + bw_align8 * 2;
#endif
                    //the rest part
                    for (; x < bw; x++) {
                        int x_ = x + j;

                        double W = y21[y_] + x20[x_];
                        W = W ? WARP_INTER_TAB_SIZE / W : 0;

                        double fX = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (y01[y_] + x00[x_]) * W));
                        double fY = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (y11[y_] + x10[x_]) * W));

                        int X = fcv_round(fX);
                        int Y = fcv_round(fY);

                        //cal the offset in block, per block width is 32
                        coeffs_row[x] = (short)((Y & (WARP_INTER_TAB_SIZE - 1)) *
                                WARP_INTER_TAB_SIZE + (X & (WARP_INTER_TAB_SIZE - 1)));

                        //in order to reduce if branch, get the correspondong src
                        //coordinate (x x+1 y y+1), stored the four data in map
                        short src0_x = (short)(X >> WARP_SCALE_BITS_HALF);
                        short src0_y = (short)(Y >> WARP_SCALE_BITS_HALF);

                        *(map_row++) = src0_x;
                        *(map_row++) = src0_y;
                    }
                }

                unsigned char* dst_ptr = _dst_data + _dst_stride * i + j * _src.channels();
                remap_linear_const_u8_neon(0, bh, bw, _src, dst_ptr,
                        _dst_stride, src_xy, coeffs, _tab, _border_value);
            }
        }

        delete[] src_xy;
        delete[] coeffs;
    }

private:
    const Mat& _src;
    unsigned char* _dst_data;
    int _dst_width;
    int _dst_height;
    int _dst_stride;
    double* _x_delta;
    double* _y_delta;
    int _block_width;
    int _block_height;
    short* _tab;
    const Scalar& _border_value;
    int _coeffs_len;
    int _src_xy_len;
};

int warp_perspective_linear_u8_const_neon(
        const Mat& src,
        Mat& dst,
        const double* m,
        const Scalar border_value) {
    const int dst_width = dst.width();
    const int dst_height = dst.height();
    const int BLOCK_SZ = 64;

    int block_height = FCV_MIN(BLOCK_SZ / 2, dst_height);
    int block_width = FCV_MIN(BLOCK_SZ * BLOCK_SZ / block_height, dst_width);
    block_height = FCV_MIN(BLOCK_SZ * BLOCK_SZ / block_width, dst_height);

    double* x_deta = (double *)malloc(((dst_width << 1) + dst_width) * sizeof(double));
    double* y_deta = (double *)malloc(((dst_height << 1) + dst_height) * sizeof(double));

    //init table 2D for bilinear interploration
    short* tab = new short[(AREA_SZ << 2)];
    init_table_2d_coeff_u8_neon(tab, WARP_INTER_TAB_SIZE);

    double* x00 = x_deta;
    double* x10 = x_deta + dst_width;
    double* x20 = x_deta + dst_width * 2;
    double* y01 = y_deta;
    double* y11 = y_deta + dst_height;
    double* y21 = y_deta + dst_height * 2;

    // fixed-point calculate for x/y coordinates
    const double *m_data = m;
    int i = 0, j = 0;
    int dst_width_align4 = dst_width & (~3);
    for (i = 0; i < dst_width_align4; i+=4) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
        x00[i + 1] = m_data[0] * (i + 1);
        x10[i + 1] = m_data[3] * (i + 1);
        x20[i + 1] = m_data[6] * (i + 1);
        x00[i + 2] = m_data[0] * (i + 2);
        x10[i + 2] = m_data[3] * (i + 2);
        x20[i + 2] = m_data[6] * (i + 2);
        x00[i + 3] = m_data[0] * (i + 3);
        x10[i + 3] = m_data[3] * (i + 3);
        x20[i + 3] = m_data[6] * (i + 3);
    }

    for (i = dst_width_align4; i < dst_width; i++) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
    }

    int dst_height_align4 = dst_height & (~3);
    for (j = 0; j < dst_height_align4; j+=4) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
        y01[j + 1] = m_data[1] * (j + 1) + m_data[2];
        y11[j + 1] = m_data[4] * (j + 1) + m_data[5];
        y21[j + 1] = m_data[7] * (j + 1) + m_data[8];
        y01[j + 2] = m_data[1] * (j + 2) + m_data[2];
        y11[j + 2] = m_data[4] * (j + 2) + m_data[5];
        y21[j + 2] = m_data[7] * (j + 2) + m_data[8];
        y01[j + 3] = m_data[1] * (j + 3) + m_data[2];
        y11[j + 3] = m_data[4] * (j + 3) + m_data[5];
        y21[j + 3] = m_data[7] * (j + 3) + m_data[8];
    }

    for (j = dst_height_align4; j < dst_height; j++) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
    }

    WarpPerpectiveLinearU8ConstParallelTask task(src, dst, x_deta, y_deta,
            block_width, block_height, tab, border_value);

    if (dst_width > 128 && dst_height > 128) {
        parallel_run(Range(0, dst_height), task);
    } else {
        parallel_run(Range(0, dst_height), task, 1);
    }

    //free
    delete[] tab;

    return 0;
}

class WarpPerpectiveLinearF32ConstParallelTask : public ParallelTask {
public:
    WarpPerpectiveLinearF32ConstParallelTask(
            const Mat& src,
            Mat& dst,
            double* x_delta,
            double* y_delta,
            int block_width,
            int block_height,
            float* tab,
            const Scalar& border_value) :
            _src(src),
            _dst_data(reinterpret_cast<float*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride() / sizeof(float)),
            _x_delta(x_delta),
            _y_delta(y_delta),
            _block_width(block_width),
            _block_height(block_height),
            _tab(tab),
            _border_value(border_value),
            _coeffs_len(block_width * block_height),
            _src_xy_len(_coeffs_len << 1) {}

    void operator()(const Range& range) const override {
        short* src_xy = new short[_src_xy_len];
        short* coeffs = new short[_coeffs_len];

        double* x00 = _x_delta;
        double* x10 = _x_delta + _dst_width;
        double* x20 = _x_delta + _dst_width * 2;
        double* y01 = _y_delta;
        double* y11 = _y_delta + _dst_height;
        double* y21 = _y_delta + _dst_height * 2;

#ifdef __aarch64__
        float64x2_t v_zero = vdupq_n_f64(0);
        float64x2_t vpos = vdupq_n_f64(0.5);
        float64x2_t vneg = vdupq_n_f64(-0.5);
        int32x4_t v_mask = vdupq_n_s32(WARP_INTER_TAB_SIZE - 1);
#endif

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);

            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);
                int bw_align8 = bw & (~7);

                for (int y = 0; y < bh; y++) {
                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;
#ifdef __aarch64__                    
                    float64x2_t v_y01 = vdupq_n_f64(y01[y_]);
                    float64x2_t v_y11 = vdupq_n_f64(y11[y_]);
#endif
                    int x = 0;
#ifdef __aarch64__
                    for (x = 0; x < bw_align8; x += 8) {
                        int x_ = x + j;

                        double W[8] = {0};
                        for (int k = 0; k < 8; k++) {
                            double tmp_w = y21[y_] + x20[x_ + k];
                            tmp_w = tmp_w ? WARP_INTER_TAB_SIZE / tmp_w : 0;
                            W[k] = tmp_w;
                        }
                        float64x2_t W_vec0 = vld1q_f64(W);
                        float64x2_t W_vec1 = vld1q_f64(W + 2);
                        float64x2_t W_vec2 = vld1q_f64(W + 4);
                        float64x2_t W_vec3 = vld1q_f64(W + 6);

                        float64x2_t fX_vec0 = vmulq_f64(W_vec0, vaddq_f64(vld1q_f64(x00 + x_), v_y01));
                        float64x2_t fX_vec1 = vmulq_f64(W_vec1, vaddq_f64(vld1q_f64(x00 + x_ + 2), v_y01));
                        float64x2_t fX_vec2 = vmulq_f64(W_vec2, vaddq_f64(vld1q_f64(x00 + x_ + 4), v_y01));
                        float64x2_t fX_vec3 = vmulq_f64(W_vec3, vaddq_f64(vld1q_f64(x00 + x_ + 6), v_y01));

                        float64x2_t fY_vec0 = vmulq_f64(W_vec0, vaddq_f64(vld1q_f64(x10 + x_), v_y11));
                        float64x2_t fY_vec1 = vmulq_f64(W_vec1, vaddq_f64(vld1q_f64(x10 + x_ + 2), v_y11));
                        float64x2_t fY_vec2 = vmulq_f64(W_vec2, vaddq_f64(vld1q_f64(x10 + x_ + 4), v_y11));
                        float64x2_t fY_vec3 = vmulq_f64(W_vec3, vaddq_f64(vld1q_f64(x10 + x_ + 6), v_y11));          
                        
                        fX_vec0 = vbslq_f64(vcgeq_f64(fX_vec0, v_zero), vaddq_f64(fX_vec0, vpos), vaddq_f64(fX_vec0, vneg));
                        fX_vec1 = vbslq_f64(vcgeq_f64(fX_vec1, v_zero), vaddq_f64(fX_vec1, vpos), vaddq_f64(fX_vec1, vneg));
                        fX_vec2 = vbslq_f64(vcgeq_f64(fX_vec2, v_zero), vaddq_f64(fX_vec2, vpos), vaddq_f64(fX_vec2, vneg));
                        fX_vec3 = vbslq_f64(vcgeq_f64(fX_vec3, v_zero), vaddq_f64(fX_vec3, vpos), vaddq_f64(fX_vec3, vneg));

                        fY_vec0 = vbslq_f64(vcgeq_f64(fY_vec0, v_zero), vaddq_f64(fY_vec0, vpos), vaddq_f64(fY_vec0, vneg));
                        fY_vec1 = vbslq_f64(vcgeq_f64(fY_vec1, v_zero), vaddq_f64(fY_vec1, vpos), vaddq_f64(fY_vec1, vneg));
                        fY_vec2 = vbslq_f64(vcgeq_f64(fY_vec2, v_zero), vaddq_f64(fY_vec2, vpos), vaddq_f64(fY_vec2, vneg));
                        fY_vec3 = vbslq_f64(vcgeq_f64(fY_vec3, v_zero), vaddq_f64(fY_vec3, vpos), vaddq_f64(fY_vec3, vneg));
                        
                        int32x4_t X_vec0 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fX_vec0)), vqmovn_s64(vcvtq_s64_f64(fX_vec1)));
                        int32x4_t X_vec1 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fX_vec2)), vqmovn_s64(vcvtq_s64_f64(fX_vec3)));
                        int32x4_t Y_vec0 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fY_vec0)), vqmovn_s64(vcvtq_s64_f64(fY_vec1)));
                        int32x4_t Y_vec1 = vcombine_s32(vqmovn_s64(vcvtq_s64_f64(fY_vec2)), vqmovn_s64(vcvtq_s64_f64(fY_vec3)));

                        //coefficient
                        int32x4_t v_coeff0 = vaddq_s32(vshlq_n_s32(vandq_s32(Y_vec0, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(X_vec0, v_mask));
                        int32x4_t v_coeff1 = vaddq_s32(vshlq_n_s32(vandq_s32(Y_vec1, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(X_vec1, v_mask));
                        int16x4_t v_coeff_n0 = vmovn_s32(v_coeff0);
                        int16x4_t v_coeff_n1 = vmovn_s32(v_coeff1);

                        vst1q_s16(coeffs_row + x, vcombine_s16(v_coeff_n0, v_coeff_n1));

                        int16x8x2_t v_xy;
                        v_xy.val[0] = vcombine_s16(vqshrn_n_s32(X_vec0, WARP_SCALE_BITS_HALF), \
                            vqshrn_n_s32(X_vec1, WARP_SCALE_BITS_HALF));
                        v_xy.val[1] = vcombine_s16(vqshrn_n_s32(Y_vec0, WARP_SCALE_BITS_HALF), \
                            vqshrn_n_s32(Y_vec1, WARP_SCALE_BITS_HALF));

                        vst2q_s16(map_row + (x << 1), v_xy);
                    }
                    map_row = map_row + bw_align8 * 2;
#endif
                    //the rest part
                    for (; x < bw; x++) {
                        int x_ = x + j;

                        double W = y21[y_] + x20[x_];
                        W = W ? WARP_INTER_TAB_SIZE / W : 0;

                        double fX = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (y01[y_] + x00[x_]) * W));
                        double fY = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (y11[y_] + x10[x_]) * W));

                        int X = fcv_round(fX);
                        int Y = fcv_round(fY);

                        //cal the offset in block, per block width is 32
                        coeffs_row[x] = (short)((Y & (WARP_INTER_TAB_SIZE - 1)) *
                                WARP_INTER_TAB_SIZE + (X & (WARP_INTER_TAB_SIZE - 1)));

                        //in order to reduce if branch, get the correspondong src
                        //coordinate (x x+1 y y+1), stored the four data in map
                        short src0_x = (short)(X >> WARP_SCALE_BITS_HALF);
                        short src0_y = (short)(Y >> WARP_SCALE_BITS_HALF);

                        *(map_row++) = src0_x;
                        *(map_row++) = src0_y;
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
    double* _x_delta;
    double* _y_delta;
    int _block_width;
    int _block_height;
    float* _tab;
    const Scalar& _border_value;
    int _coeffs_len;
    int _src_xy_len;
};

int warp_perspective_linear_f32_const_neon(
        const Mat& src,
        Mat& dst,
        const double* m,
        const Scalar border_value) {
    const int BLOCK_SZ = 64;
    int dst_width = dst.width();
    int dst_height = dst.height();

    int block_height = FCV_MIN(BLOCK_SZ / 2, dst_height);
    int block_width = FCV_MIN(BLOCK_SZ * BLOCK_SZ / block_height, dst_width);
    block_height = FCV_MIN(BLOCK_SZ * BLOCK_SZ / block_width, dst_height);

    //creat array map and coeffs to store the remap coordinate matrix and the remap coefficient matrix
    float* tab = new float[AREA_SZ << 2];
    double *x_deta = (double *)malloc(((dst_width << 1) + dst_width) * sizeof(double));
    double *y_deta = (double *)malloc(((dst_height << 1) + dst_height) * sizeof(double));
    short *coeffs = (short *)malloc((AREA_SZ) * sizeof(short));

    //init table 2D for bilinear interploration
    init_table_2d_coeff_f32_neon(tab, WARP_INTER_TAB_SIZE);

    double* x00 = x_deta;
    double* x10 = x_deta + dst_width;
    double* x20 = x_deta + dst_width * 2;
    double* y01 = y_deta;
    double* y11 = y_deta + dst_height;
    double* y21 = y_deta + dst_height * 2;

    // fixed-point calculate for x/y coordinates
    const double *m_data = m;
    int i = 0, j = 0;
    int dst_width_align4 = dst_width & (~3);
    for (i = 0; i < dst_width_align4; i+=4) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
        x00[i + 1] = m_data[0] * (i + 1);
        x10[i + 1] = m_data[3] * (i + 1);
        x20[i + 1] = m_data[6] * (i + 1);
        x00[i + 2] = m_data[0] * (i + 2);
        x10[i + 2] = m_data[3] * (i + 2);
        x20[i + 2] = m_data[6] * (i + 2);
        x00[i + 3] = m_data[0] * (i + 3);
        x10[i + 3] = m_data[3] * (i + 3);
        x20[i + 3] = m_data[6] * (i + 3);
    }

    for (i = dst_width_align4; i < dst_width; i++) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
    }

    int dst_height_align4 = dst_height & (~3);
    for (j = 0; j < dst_height_align4; j+=4) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
        y01[j + 1] = m_data[1] * (j + 1) + m_data[2];
        y11[j + 1] = m_data[4] * (j + 1) + m_data[5];
        y21[j + 1] = m_data[7] * (j + 1) + m_data[8];
        y01[j + 2] = m_data[1] * (j + 2) + m_data[2];
        y11[j + 2] = m_data[4] * (j + 2) + m_data[5];
        y21[j + 2] = m_data[7] * (j + 2) + m_data[8];
        y01[j + 3] = m_data[1] * (j + 3) + m_data[2];
        y11[j + 3] = m_data[4] * (j + 3) + m_data[5];
        y21[j + 3] = m_data[7] * (j + 3) + m_data[8];
    }

    for (j = dst_height_align4; j < dst_height; j++) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
    }

    WarpPerpectiveLinearF32ConstParallelTask task(src, dst,
            x_deta, y_deta, block_width, block_height, tab, border_value);

    if (dst_width > 128 && dst_height > 128) {
        parallel_run(Range(0, dst_height), task);
    } else {
        parallel_run(Range(0, dst_height), task, 1);
    }

    //free
    delete[] tab;

    return 0;
}

static void warp_perspective_linear_const_neon(
        const Mat& src,
        Mat& dst,
        const double *m_data,
        const Scalar border_value) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_U8:
        warp_perspective_linear_u8_const_neon(src, dst, m_data, border_value);
        break;
    case FCVImageType::GRAY_F32:
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        warp_perspective_linear_f32_const_neon(src, dst, m_data, border_value);
        break;
    default:
        LOG_ERR("warp_perspective data type not support yet!");
        break;
    };
}

static int warp_perspective_linear_neon(
        const Mat& src,
        Mat& dst,
        Mat& m,
        BorderType border_method,
        const Scalar border_value) {
    Mat m_ivt(3, 3, FCVImageType::GRAY_F64);
    bool flag = invert_perspective_3x3_mat(m, m_ivt);

    if (!flag) {
        LOG_ERR("failed to invert matrix!\n");
        return -1;
    }

    double m_data[9];
    if (m.type() == FCVImageType::GRAY_F32) {
        float* m0 = (float*)m_ivt.data();
        m_data[0] = m0[0];
        m_data[1] = m0[1];
        m_data[2] = m0[2];
        m_data[3] = m0[3];
        m_data[4] = m0[4];
        m_data[5] = m0[5];
        m_data[6] = m0[6];
        m_data[7] = m0[7];
        m_data[8] = m0[8];
    } else if (m.type() == FCVImageType::GRAY_F64) {
        double* m0 = (double*)m_ivt.data();
        m_data[0] = m0[0];
        m_data[1] = m0[1];
        m_data[2] = m0[2];
        m_data[3] = m0[3];
        m_data[4] = m0[4];
        m_data[5] = m0[5];
        m_data[6] = m0[6];
        m_data[7] = m0[7];
        m_data[8] = m0[8];
    } else {
        LOG_ERR("warp_perspective m data type not support yet!");
        return -1;
    }

    int sc = src.channels();
    if (sc > 0) {
        switch (border_method) {
        case BorderType::BORDER_CONSTANT:
            warp_perspective_linear_const_neon(src, dst, m_data, border_value);
            break;
        default:
            LOG_ERR("warp_perspective interpolation type not support yet!");
            break;
        };
    } else {
        LOG_ERR("warp_perspective data type not support yet");
    }

    return 0;
}

int warp_perspective_neon(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag,
        BorderType border_method,
        const Scalar border_value) {
    switch (flag) {
    case InterpolationType::INTER_LINEAR:
        warp_perspective_linear_neon(src, dst, m, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_perspective interpolation type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
