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

#include "modules/core/mat/interface/mat.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/remap/include/remap_arm.h"
#include "modules/img_transform/remap/include/remap_common.h"

#include <stdlib.h>
#include <thread>

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class WarpAffineLinearParallelTask : public ParallelTask {
public:
    WarpAffineLinearParallelTask(
            const Mat& src,
            Mat& dst,
            int* x_delta,
            int* y_delta,
            int block_height,
            int block_width,
            void* tab,
            const Scalar& border_value) :
            _src(src),
            _dst_data(dst.data()),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride()),
            _x_delta(x_delta),
            _y_delta(y_delta),
            _block_height(block_height),
            _block_width(block_width),
            _tab(tab),
            _border_value(border_value) {}

    void operator()(const Range& range) const override {
        int coeffs_len = _block_width * _block_height * sizeof(short);
        int src_xy_len = coeffs_len * 2;

        short* src_xy = new short[src_xy_len];
        short* coeffs = new short[coeffs_len];

        int32x4_t v_mask = vdupq_n_s32(WARP_INTER_TAB_SIZE - 1);

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);

            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);
                int bw_align8 = bw & (~7);

                for (int y = 0; y < bh; y++) {
                    int y_ = (y + i) << 1;
                    int y01 = _y_delta[y_];
                    int y11 = _y_delta[y_ + 1];

                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));

                    int32x4_t v_y01 = vdupq_n_s32(y01);
                    int32x4_t v_y11 = vdupq_n_s32(y11);

                    int x = 0;
                    for (x = 0; x < bw_align8; x += 8) {
                        int x_id = (x + j) << 1;
                        int16x4x2_t v_xy0, v_xy1;
                        int32x4x2_t v_x00 = vld2q_s32(_x_delta + x_id);

                        //round
                        int32x4_t v_x = vshrq_n_s32(vaddq_s32(v_y01, v_x00.val[0]), WARP_SCALE_BITS_HALF);
                        int32x4_t v_y = vshrq_n_s32(vaddq_s32(v_y11, v_x00.val[1]), WARP_SCALE_BITS_HALF);

                        //coefficient
                        int32x4_t v_coeff = vaddq_s32(vshlq_n_s32(vandq_s32(v_y, v_mask),
                                WARP_SCALE_BITS_HALF), vandq_s32(v_x, v_mask));
                        int16x4_t v_coeff_n0 = vmovn_s32(v_coeff);

                        //corordiate
                        v_xy0.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy0.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);

                        v_x00 = vld2q_s32(_x_delta + x_id + 8);

                        v_x = vshrq_n_s32(vaddq_s32(v_y01, v_x00.val[0]), WARP_SCALE_BITS_HALF);
                        v_y = vshrq_n_s32(vaddq_s32(v_y11, v_x00.val[1]), WARP_SCALE_BITS_HALF);

                        v_xy1.val[0] = vqshrn_n_s32(v_x, WARP_SCALE_BITS_HALF);
                        v_xy1.val[1] = vqshrn_n_s32(v_y, WARP_SCALE_BITS_HALF);

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
                        int x_ = (x + j) << 1;

                        int X = (y01 + _x_delta[x_]) >> WARP_SCALE_BITS_HALF;
                        int Y = (y11 + _x_delta[x_ + 1]) >> WARP_SCALE_BITS_HALF;

                        //cal the offset in block, per block width is 32
                        coeffs_row[x] = (short)((Y & (WARP_INTER_TAB_SIZE - 1)) *
                                WARP_INTER_TAB_SIZE + (X & (WARP_INTER_TAB_SIZE - 1)));

                        //in order to reduce if branch, get the correspondong src
                        //coordinate (x x+1 y y+1), stored the four data in map
                        short src0_x = (short)(X >> WARP_SCALE_BITS_HALF);
                        short src0_y = (short)(Y >> WARP_SCALE_BITS_HALF);

                        map_row[(x << 1) + 0] = src0_x;
                        map_row[(x << 1) + 1] = src0_y;
                    }
                }

                if (_src.type() == FCVImageType::GRAY_U8 ||
                    _src.type() == FCVImageType::PKG_BGR_U8 ||
                    _src.type() == FCVImageType::PKG_RGB_U8) {
                    unsigned char* dst_start = (unsigned char *)((unsigned char *)(_dst_data) +
                            _dst_stride * i + j * _src.channels());
                    remap_linear_const_u8_neon(0, bh, bw, _src, dst_start,
                            _dst_stride, src_xy, coeffs, (short*)_tab, _border_value);
                } else {
                    int stride = _dst_stride / sizeof(float);
                    float* dst_start = (float*)((float*)(_dst_data) +
                            stride * i + j * _src.channels());
                    remap_linear_const_f32_neon(0, bh, bw, _src, dst_start,
                            stride, src_xy, coeffs, (float*)_tab, _border_value);
                }
            }
        }

        delete[] src_xy;
        delete[] coeffs;
    }

private:
    const Mat& _src;
    void* _dst_data;
    int _dst_width;
    int _dst_height;
    int _dst_stride;
    int* _x_delta;
    int* _y_delta;
    int _block_height;
    int _block_width;
    void* _tab;
    const Scalar& _border_value;
};

static void warp_affine_linear_border_const_neon(
        const Mat& src,
        Mat& dst,
        const double *m_data,
        const Scalar border_value) {
    // creat array map and coeffs to store the remap coordinate matrix
    // and the remap coefficient matrix
    int tab_len = (AREA_SZ << 2) * sizeof(float);
    void* tab = nullptr;

    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGB_U8:
        tab = malloc(tab_len);
        memset(tab, 0, tab_len);
        init_table_2d_coeff_u8_neon((short*)tab, WARP_INTER_TAB_SIZE);
        break;
    case FCVImageType::GRAY_F32:
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        tab = malloc(tab_len);
        memset(tab, 0, tab_len);
        init_table_2d_coeff_f32_neon((float*)tab, WARP_INTER_TAB_SIZE);
        break;
    default:
        LOG_ERR("warp_affine data type not support yet!");
        return;
    }

    int dst_width = dst.width();
    int dst_height = dst.height();
    int block_height = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_height);
    int block_width = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / block_height, dst_width);
    int round_delta = 1 << (WARP_SCALE_BITS_HALF - 1);

    int x_delta_len = (dst_width << 1) * sizeof(int);
    int *x_delta   = (int *)malloc(x_delta_len);
    memset(x_delta, 0, x_delta_len);

    int y_delta_len = (dst_height << 1) * sizeof(int);
    int *y_delta   = (int *)malloc(y_delta_len);
    memset(y_delta, 0, y_delta_len);

    for (int i = 0; i < dst_width; i++) {
        int index = i << 1;
        x_delta[index] = fcv_round(m_data[0] * i * AB_SCALE);
        x_delta[index + 1] = fcv_round(m_data[3] * i * AB_SCALE);
    }

    for (int i = 0; i < dst_height; i++) {
        int index = i << 1;
        y_delta[index] = fcv_round((i * m_data[1] + m_data[2]) * AB_SCALE) + round_delta;
        y_delta[index + 1] = fcv_round((i * m_data[4] + m_data[5]) * AB_SCALE) + round_delta;
    }

    WarpAffineLinearParallelTask task(src, dst, x_delta, y_delta,
            block_height, block_width, tab, border_value);

    parallel_run(Range(0, dst_height), task);

    free(tab);
    free(x_delta);
    free(y_delta);
}

static int warp_affine_linear_neon(
        const Mat& src,
        Mat& dst,
        const double *m_data,
        BorderType border_method,
        const Scalar border_value) {

    int sc = src.channels();
    if (sc > 0) {
        switch (border_method) {
        case BorderType::BORDER_CONSTANT:
            warp_affine_linear_border_const_neon(src, dst, m_data, border_value);
            break;
        default:
            LOG_ERR("warp_affine interpolation type not support yet!");
            break;
        };
    } else {
        LOG_ERR("warp_affine data type not support yet");
    }

    return 0;
}

int warp_affine_neon(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag,
        BorderType border_method,
        const Scalar border_value) {
    double m_data[6];
    if (m.type() == FCVImageType::GRAY_F32) {
        float* m0 = (float*)m.data();
        m_data[0] = m0[0];
        m_data[1] = m0[1];
        m_data[2] = m0[2];
        m_data[3] = m0[3];
        m_data[4] = m0[4];
        m_data[5] = m0[5];
    } else if (m.type() == FCVImageType::GRAY_F64){
        double* m0 = (double*)m.data();
        m_data[0] = m0[0];
        m_data[1] = m0[1];
        m_data[2] = m0[2];
        m_data[3] = m0[3];
        m_data[4] = m0[4];
        m_data[5] = m0[5];
    } else {
        LOG_ERR("warp_affine m data type not support yet!");
        return -1;
    }

    switch (flag) {
    case InterpolationType::INTER_LINEAR:
        //get inverse affine matrix m_ivt
        double m_ivt[6];
        inverse_matrix_2x3_float(m_data, m_ivt);
        warp_affine_linear_neon(src, dst, m_ivt, border_method, border_value);
        break;
    case InterpolationType::WARP_INVERSE_MAP:
        warp_affine_linear_neon(src, dst, m_data, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_affine interpolation type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
