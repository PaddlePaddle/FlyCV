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
            const double* trans_m,
            int block_width,
            int block_height,
            short* tab,
            const Scalar& border_value) :
            _src(src),
            _dst_data(reinterpret_cast<unsigned char*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride()),
            _trans_m(trans_m),
            _block_width(block_width),
            _block_height(block_height),
            _tab(tab),
            _border_value(border_value),
            _coeffs_len(block_width * block_height * sizeof(short)),
            _src_xy_len(_coeffs_len << 1) {}

    void operator()(const Range& range) const override {
        short* src_xy = new short[_src_xy_len];
        short* coeffs = new short[_coeffs_len];

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);

            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);

                for (int y = 0; y < bh; y++) {
                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;

                    double X0 = _trans_m[0] * j + _trans_m[1] * y_ + _trans_m[2];
                    double Y0 = _trans_m[3] * j + _trans_m[4] * y_ + _trans_m[5];
                    double W0 = _trans_m[6] * j + _trans_m[7] * y_ + _trans_m[8];

                    //the rest part
                    for (int x = 0; x < bw; x++) {
                        double W = W0 + _trans_m[6] * x;
                        W = W ? WARP_INTER_TAB_SIZE / W : 0;
                        double fX = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (X0 + _trans_m[0] * x) * W));
                        double fY = FCV_MAX((double)INT_MIN, FCV_MIN((double)INT_MAX,
                                    (Y0 + _trans_m[3] * x) * W));

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
    const double* _trans_m;
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

    //init table 2D for bilinear interploration
    short* tab = new short[(AREA_SZ << 2)];
    init_table_2d_coeff_u8_neon(tab, WARP_INTER_TAB_SIZE);

    WarpPerpectiveLinearU8ConstParallelTask task(src, dst, m,
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
            const double* trans_m,
            int block_width,
            int block_height,
            float* tab,
            const Scalar& border_value) :
            _src(src),
            _dst_data(reinterpret_cast<float*>(dst.data())),
            _dst_width(dst.width()),
            _dst_height(dst.height()),
            _dst_stride(dst.stride() / sizeof(float)),
            _trans_m(trans_m),
            _block_width(block_width),
            _block_height(block_height),
            _tab(tab),
            _border_value(border_value),
            _coeffs_len(block_width * block_height),
            _src_xy_len(_coeffs_len << 1) {}

    void operator()(const Range& range) const override {
        short* src_xy = new short[_src_xy_len];
        short* coeffs = new short[_coeffs_len];

        for (int i = range.start(); i < range.end(); i += _block_height) {
            int bh = FCV_MIN(_block_height, range.end() - i);

            for (int j = 0; j < _dst_width; j += _block_width) {
                int bw = FCV_MIN(_block_width, _dst_width - j);

                for (int y = 0; y < bh; y++) {
                    short* map_row = (short *)(src_xy + (bw * (y << 1)));
                    short* coeffs_row = (short *)(coeffs + (bw * y));
                    int y_ = y + i;

                    double X0 = _trans_m[0] * j + _trans_m[1] * y_ + _trans_m[2];
                    double Y0 = _trans_m[3] * j + _trans_m[4] * y_ + _trans_m[5];
                    double W0 = _trans_m[6] * j + _trans_m[7] * y_ + _trans_m[8];

                    for (int x = 0 ; x < bw; x++) {
                        double W = W0 + _trans_m[6] * x;
                        W = W ? WARP_INTER_TAB_SIZE / W : 0;
                        double fX = FCV_MAX((double)INT_MIN,
                                FCV_MIN((double)INT_MAX, (X0 + _trans_m[0] * x) * W));
                        double fY = FCV_MAX((double)INT_MIN,
                                FCV_MIN((double)INT_MAX, (Y0 + _trans_m[3] * x) * W));

                        int X = fcv_round(fX);
                        int Y = fcv_round(fY);

                        //cal the offset in block, per block width is 32
                        coeffs_row[x] = (short)((Y & (WARP_INTER_TAB_SIZE - 1)) *
                                WARP_INTER_TAB_SIZE + (X & (WARP_INTER_TAB_SIZE - 1)));

                        //in order to reduce if branch, get the correspondong
                        //src coordinate (x x+1 y y+1), stored the four data in map
                        short src0_x = (short)(X >> WARP_SCALE_BITS_HALF);
                        short src0_y = (short)(Y >> WARP_SCALE_BITS_HALF);

                        map_row[(x << 1)] = src0_x;
                        map_row[(x << 1) + 1] = src0_y;
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
    const double* _trans_m;
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

    //init table 2D for bilinear interploration
    init_table_2d_coeff_f32_neon(tab, WARP_INTER_TAB_SIZE);

    WarpPerpectiveLinearF32ConstParallelTask task(src, dst,
            m, block_width, block_height, tab, border_value);

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
