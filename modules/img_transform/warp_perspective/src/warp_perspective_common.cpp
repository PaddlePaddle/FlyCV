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

#include "modules/img_transform/warp_perspective/include/warp_perspective_common.h"

#include <cmath>
#include <cfloat>
#include <stdlib.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

typedef unsigned char uchar;

/****************************************************************************************\
*                          Inverse (or pseudo-inverse) of a matrix                       *
\****************************************************************************************/
const double EPSILON = 1e-9;
#define Sf( y, x ) ((float*)(srcdata + y*srcstep))[x]
#define Sd( y, x ) ((double*)(srcdata + y*srcstep))[x]
#define Df( y, x ) ((float*)(dstdata + y*dststep))[x]
#define Dd( y, x ) ((double*)(dstdata + y*dststep))[x]

#define det2(m)   ((double)m(0,0)*m(1,1) - (double)m(0,1)*m(1,0))
#define det3(m)   (m(0,0)*((double)m(1,1)*m(2,2) - (double)m(1,2)*m(2,1)) -  \
                   m(0,1)*((double)m(1,0)*m(2,2) - (double)m(1,2)*m(2,0)) +  \
                   m(0,2)*((double)m(1,0)*m(2,1) - (double)m(1,1)*m(2,0)))

bool invert_perspective_3x3_mat(Mat src, Mat dst) {
    FCVImageType type = src.type();
    if (type != FCVImageType::GRAY_F64 && type != FCVImageType::GRAY_F32) {
        return false;
    }

    int m = src.height();
    int n = src.width();

    if (m != n) {
        return false;
    }

    if (n <= 3) {
        const uchar* srcdata = (uchar*)src.data();
        uchar* dstdata = (uchar*)dst.data();
        size_t srcstep = src.stride();
        size_t dststep = dst.stride();

        if (n == 2) {
            if (type == FCVImageType::GRAY_F32) {
                double d = det2(Sf);
                if (std::fabs(d) >= EPSILON) {
                    d = 1./d;
                    double t0 = 0., t1 = 0.;
                    t0 = Sf(0,0)*d;
                    t1 = Sf(1,1)*d;
                    Df(1,1) = (float)t0;
                    Df(0,0) = (float)t1;
                    t0 = -Sf(0,1)*d;
                    t1 = -Sf(1,0)*d;
                    Df(0,1) = (float)t0;
                    Df(1,0) = (float)t1;
                }
            } else {
                double d = det2(Sd);
                if (std::fabs(d) >= EPSILON) {
                    d = 1./d;
                    double t0 = 0., t1 = 0.;
                    t0 = Sd(0,0)*d;
                    t1 = Sd(1,1)*d;
                    Dd(1,1) = t0;
                    Dd(0,0) = t1;
                    t0 = -Sd(0,1)*d;
                    t1 = -Sd(1,0)*d;
                    Dd(0,1) = t0;
                    Dd(1,0) = t1;
                }
            }
        } else if (n == 3) {
            if (type == FCVImageType::GRAY_F32) {
                double d = det3(Sf);
                if (std::fabs(d) >= EPSILON) {
                    double t[12];
                    d = 1./d;
                    t[0] = (((double)Sf(1,1) * Sf(2,2) - (double)Sf(1,2) * Sf(2,1)) * d);
                    t[1] = (((double)Sf(0,2) * Sf(2,1) - (double)Sf(0,1) * Sf(2,2)) * d);
                    t[2] = (((double)Sf(0,1) * Sf(1,2) - (double)Sf(0,2) * Sf(1,1)) * d);

                    t[3] = (((double)Sf(1,2) * Sf(2,0) - (double)Sf(1,0) * Sf(2,2)) * d);
                    t[4] = (((double)Sf(0,0) * Sf(2,2) - (double)Sf(0,2) * Sf(2,0)) * d);
                    t[5] = (((double)Sf(0,2) * Sf(1,0) - (double)Sf(0,0) * Sf(1,2)) * d);

                    t[6] = (((double)Sf(1,0) * Sf(2,1) - (double)Sf(1,1) * Sf(2,0)) * d);
                    t[7] = (((double)Sf(0,1) * Sf(2,0) - (double)Sf(0,0) * Sf(2,1)) * d);
                    t[8] = (((double)Sf(0,0) * Sf(1,1) - (double)Sf(0,1) * Sf(1,0)) * d);

                    Df(0,0) = (float)t[0]; Df(0,1) = (float)t[1]; Df(0,2) = (float)t[2];
                    Df(1,0) = (float)t[3]; Df(1,1) = (float)t[4]; Df(1,2) = (float)t[5];
                    Df(2,0) = (float)t[6]; Df(2,1) = (float)t[7]; Df(2,2) = (float)t[8];
                }
            } else {
                double d = det3(Sd);
                if (std::fabs(d) >= EPSILON) {
                    d = 1./d;
                    double t[9];

                    t[0] = (Sd(1,1) * Sd(2,2) - Sd(1,2) * Sd(2,1)) * d;
                    t[1] = (Sd(0,2) * Sd(2,1) - Sd(0,1) * Sd(2,2)) * d;
                    t[2] = (Sd(0,1) * Sd(1,2) - Sd(0,2) * Sd(1,1)) * d;

                    t[3] = (Sd(1,2) * Sd(2,0) - Sd(1,0) * Sd(2,2)) * d;
                    t[4] = (Sd(0,0) * Sd(2,2) - Sd(0,2) * Sd(2,0)) * d;
                    t[5] = (Sd(0,2) * Sd(1,0) - Sd(0,0) * Sd(1,2)) * d;

                    t[6] = (Sd(1,0) * Sd(2,1) - Sd(1,1) * Sd(2,0)) * d;
                    t[7] = (Sd(0,1) * Sd(2,0) - Sd(0,0) * Sd(2,1)) * d;
                    t[8] = (Sd(0,0) * Sd(1,1) - Sd(0,1) * Sd(1,0)) * d;

                    Dd(0,0) = t[0]; Dd(0,1) = t[1]; Dd(0,2) = t[2];
                    Dd(1,0) = t[3]; Dd(1,1) = t[4]; Dd(1,2) = t[5];
                    Dd(2,0) = t[6]; Dd(2,1) = t[7]; Dd(2,2) = t[8];
                }
            }
        } else {
            if (type == FCVImageType::GRAY_F32) {
                double d = Sf(0,0);
                if(std::fabs(d) >= EPSILON) {
                    Df(0,0) = (float)(1./d);
                }
            } else {
                double d = Sd(0,0);
                if(std::fabs(d) >= EPSILON) {
                    Dd(0,0) = 1./d;
                }
            }
        }
    } else {
        if (type == FCVImageType::GRAY_F32) {
            return false;
        } else if (type == FCVImageType::GRAY_F64) {
            return false;
        } else {
            return false;
        }
    }

    return true;
}

/*
    Perform an affine transform on an input image

    {m[0] m[1] m[2]
     m[3] m[4] m[5]}

    src_x = dst_x * m[0] + dst_y * m[2] + m[4]
    src_y = dst_x * m[1] + dst_y * m[3] + m[5]
*/
static int warp_perspective_linear_u8_const(
        const unsigned char *src_data,
        unsigned char *dst_data, 
        int src_h,
        int src_w,
        int src_stride,
        int dst_h,
        int dst_w,
        int dst_stride,
        int src_channel, 
        const double *m_data, 
        const Scalar border_value) {
    if ((nullptr == src_data)
            || (nullptr == dst_data)
            || (nullptr == m_data)) {
        LOG_ERR("Mat is empty!");
        return -1;
    }
    const int BLOCK_SZ = 64;

    int bh0 = FCV_MIN(BLOCK_SZ / 2, dst_h);
    int bw0 = FCV_MIN(BLOCK_SZ * BLOCK_SZ / bh0, dst_w);
    // bh0 = FCV_MIN(BLOCK_SZ * BLOCK_SZ / bw0, dst_h);

    double* x_deta = (double *)malloc(((dst_w << 1) + dst_w) * sizeof(double));
    double* y_deta = (double *)malloc(((dst_h << 1) + dst_h) * sizeof(double));
    short* tab = (short *)malloc((AREA_SZ << 2) * sizeof(short));
    short* src_xy = (short *)malloc((bh0 * bw0 * 2) * sizeof(short));
    short* coeffs = (short *)malloc((AREA_SZ) * sizeof(short));

    init_table_2d_coeff_u8(tab, WARP_INTER_TAB_SIZE);

    double* x00 = x_deta;
    double* x10 = x_deta + dst_w;
    double* x20 = x_deta + dst_w * 2;
    double* y01 = y_deta;
    double* y11 = y_deta + dst_h;
    double* y21 = y_deta + dst_h * 2;

    // fixed-point calculate for x/y coordinates
    int i = 0, j = 0;
    int dst_width_align4 = dst_w & (~3);
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

    for (i = dst_width_align4; i < dst_w; i++) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
    }

    int dst_height_align4 = dst_h & (~3);
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

    for (j = dst_height_align4; j < dst_h; j++) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
    }

    for (i = 0; i < dst_h; i += bh0) {
        int bh = FCV_MIN(bh0, dst_h - i);
        unsigned char* ptr_dst = (unsigned char *)(dst_data + dst_stride * i);
        for (j = 0; j < dst_w; j += bw0) {
            int bw = FCV_MIN(bw0, dst_w - j);
            unsigned char* dst_ptr = (unsigned char *)(ptr_dst + j * src_channel);
            for (int y = 0; y < bh; y++) {
                short* map_row    = (short *)(src_xy + (bw * (y << 1)));
                short* coeffs_row = (short *)(coeffs + (bw * y));
                int y_ = y + i;

                for (int x = 0 ; x < bw; x++ ) {
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

            remap_linear_u8_const(bh, bw, src_data, src_w, src_h, src_stride,
                    dst_ptr, dst_stride, src_xy, coeffs, src_channel, tab, border_value);

        }
    }

    free(x_deta);
    free(y_deta);
    free(tab);
    free(src_xy);
    free(coeffs);
    return 0;
}

static int warp_perspective_linear_f32_const(
        const float *src_data,
        float *dst_data,
        int src_h,
        int src_w,
        int src_stride,
        int dst_h,
        int dst_w,
        int dst_stride,
        int src_channel,
        const double *m_data,
        const Scalar border_value) {
    const int BLOCK_SZ = 64;

    int bh0 = FCV_MIN(BLOCK_SZ / 2, dst_h);
    int bw0 = FCV_MIN(BLOCK_SZ * BLOCK_SZ / bh0, dst_w);
    bh0 = FCV_MIN(BLOCK_SZ * BLOCK_SZ / bw0, dst_h);

    double *x_deta = (double *)malloc(((dst_w << 1) + dst_w) * sizeof(double));
    double *y_deta = (double *)malloc(((dst_h << 1) + dst_h) * sizeof(double));
    float *tab = (float *)malloc((AREA_SZ << 2) * sizeof(float)); //store the coefficient four points coordinata
    short *src_xy = (short *)malloc((bh0 * bw0 * 2) * sizeof(short));
    short *coeffs = (short *)malloc((AREA_SZ) * sizeof(short));

    //dividing the peocessing width by 32
    init_table_2d_coeff_f32(tab, WARP_INTER_TAB_SIZE);

    double* x00 = x_deta;
    double* x10 = x_deta + dst_w;
    double* x20 = x_deta + dst_w * 2;
    double* y01 = y_deta;
    double* y11 = y_deta + dst_h;
    double* y21 = y_deta + dst_h * 2;

    // fixed-point calculate for x/y coordinates
    int i = 0, j = 0;
    int dst_width_align4 = dst_w & (~3);
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

    for (i = dst_width_align4; i < dst_w; i++) {
        x00[i] = m_data[0] * i;
        x10[i] = m_data[3] * i;
        x20[i] = m_data[6] * i;
    }

    int dst_height_align4 = dst_h & (~3);
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

    for (j = dst_height_align4; j < dst_h; j++) {
        y01[j] = m_data[1] * j + m_data[2];
        y11[j] = m_data[4] * j + m_data[5];
        y21[j] = m_data[7] * j + m_data[8];
    }

    for (i = 0; i < dst_h; i += bh0) {
        int bh = FCV_MIN(bh0, dst_h - i);
        float* ptr_dst = (float *)(dst_data + dst_stride * i);
        for (j = 0; j < dst_w; j += bw0) {
            int bw = FCV_MIN(bw0, dst_w - j);
            for (int y = 0; y < bh; y++) {
                short* map_row    = (short*)(src_xy + (bw * (y << 1)));
                short* coeffs_row = (short*)(coeffs + (bw * y));
                int y_ = y + i;

                for (int x = 0 ; x < bw; x++) {
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

                    map_row[(x << 1) + 0] = src0_x;
                    map_row[(x << 1) + 1] = src0_y;
                }
            }

            float* dst_ptr = (float *)(ptr_dst + j * src_channel);
            remap_linear_f32_const(bh, bw, src_data, src_w, src_h, src_stride, dst_ptr,
                    dst_stride, src_xy, coeffs, src_channel, tab, border_value);
        }
    }

    free(x_deta);
    free(y_deta);
    free(tab);
    free(src_xy);
    free(coeffs);

    return 0;
}

static void warp_perspective_linear_const_common(
        const void *src_data,
        void *dst_data,
        int src_h,
        int src_w,
        int src_stride,
        int dst_h,
        int dst_w,
        int dst_stride,
        const int sc,
        const double *m_data,
        FCVImageType data_type,
        const Scalar border_value) {
    switch (data_type) {
        case FCVImageType::GRAY_U8:
        case FCVImageType::PKG_BGR_U8: 
        case FCVImageType::PKG_RGB_U8:
            warp_perspective_linear_u8_const((unsigned char*)src_data,
                    (unsigned char*)dst_data, src_h, src_w, src_stride,
                    dst_h, dst_w, dst_stride, sc, m_data, border_value);
            break;
        case FCVImageType::GRAY_F32:
        case FCVImageType::PKG_BGR_F32:
        case FCVImageType::PKG_RGB_F32:
            warp_perspective_linear_f32_const((float*)src_data,
                    (float*)dst_data, src_h, src_w, src_stride / sizeof(float),
                    dst_h, dst_w, dst_stride / sizeof(float), sc, m_data, border_value);
            break;
        default:
        LOG_ERR("warp_perspective data type not support yet!");
            break;
    };
}

static int warp_perspective_linear_comm(
        const Mat& src,
        Mat& dst,
        Mat& m,
        BorderTypes border_method,
        const Scalar border_value) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    void *src_data = (void *)src.data();
    void *dst_data = (void *)dst.data();

    int sc = src.channels();
    int src_stride = src.stride();
    int dst_stride = dst.stride();

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
    } else if (m.type() == FCVImageType::GRAY_F64){
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

    if (sc > 0) {
        switch (border_method) {
        case BorderTypes::BORDER_CONSTANT:
            warp_perspective_linear_const_common(src_data, dst_data,
                    src_h, src_w, src_stride, dst_h, dst_w, dst_stride,
                    sc, m_data, src.type(), border_value);
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

int warp_perspective_common(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag,
        BorderTypes border_method,
        const Scalar border_value) {
    switch (flag) {
    case InterpolationType::INTER_LINEAR:
        warp_perspective_linear_comm(src, dst, m, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_perspective interpolation type not support yet!");
        break;
    };

    return 0;
}

template<typename _Tp> static int
perspective_LUImpl(_Tp* A, 
        size_t astep,
        int m,
        _Tp* b,
        size_t bstep,
        int n,
        _Tp eps) {
    int i = 0;
    int j = 0;
    int k = 0;
    astep /= sizeof(A[0]);
    bstep /= sizeof(b[0]);

    // find principal element
    for (i = 0; i < m; i++) {
        k = i;
        for (j = i + 1; j < m; j++) {
            // A[j][[i] > A[i][i]
            if(std::abs(A[j * astep + i]) > std::abs(A[k * astep + i])) {
                k = j;
            }
        }
        // principal element is zero so A is Not full rank matrix and has no inverse matrix
        if(std::abs(A[k * astep + i]) < eps) {
            return 0;
        }

        // if principal element is not in row i，switch row i and row k
        if (k != i) {
            for (j = i; j < m; j++) {
                std::swap(A[i * astep + j], A[k * astep + j]);
            }
            if (b) {
                for(j = 0; j < n; j++) {
                    std::swap(b[i * bstep + j], b[k * bstep + j]);
                }
            }
        }

        _Tp d = -1/A[i * astep + i];

        for (j = i + 1; j < m; j++) {
            _Tp alpha = A[j * astep + i] * d;

            for (k = i + 1; k < m; k++) {
                A[j * astep + k] += alpha * A[i * astep + k];
            }

            if (b) {
                for (k = 0; k < n; k++) {
                    b[j * bstep + k] += alpha * b[i * bstep + k];
                }
            }
        }
    }

    if (b) {
        for (i = m - 1; i >= 0; i--) {
            for (j = 0; j < n; j++) {
                _Tp s = b[i * bstep + j];
                for (k = i + 1; k < m; k++) {
                    s -= A[i * astep + k] * b[k * bstep + j];
                }
                b[i * bstep + j] = s / A[i * astep + i];
            }
        }
    }

    return 0;
}

int perspective_LU64f(double* A,
        size_t astep,
        int m,
        double* b,
        size_t bstep,
        int n) {
    int res = -1;
    res = perspective_LUImpl(A, astep, m, b, bstep, n, (double)(FLT_EPSILON * 100));
    return res;
}

void perspective_solve(Mat& a, Mat& b, Mat& dst) {
    double *buffer = (double *)malloc(sizeof(double) * 64);
    Mat a0(a.width(), a.height(), a.type(), buffer);
    a.copy_to(a0);
    b.copy_to(dst);

    perspective_LU64f((double*)a0.data(), a0.stride(), a0.width(),
            (double*)dst.data(), dst.stride(), dst.width());

    free(buffer);
}

/* Calculates coefficients of perspective transformation
 * which maps (xi,yi) to (ui,vi), (i=1,2,3,4):
 *
 *      c00*xi + c01*yi + c02
 * ui = ---------------------
 *      c20*xi + c21*yi + c22
 *
 *      c10*xi + c11*yi + c12
 * vi = ---------------------
 *      c20*xi + c21*yi + c22
 *
 * Coefficients are calculated by solving linear system:
 * / x0 y0  1  0  0  0 -x0*u0 -y0*u0 \ /c00\ /u0\
 * | x1 y1  1  0  0  0 -x1*u1 -y1*u1 | |c01| |u1|
 * | x2 y2  1  0  0  0 -x2*u2 -y2*u2 | |c02| |u2|
 * | x3 y3  1  0  0  0 -x3*u3 -y3*u3 |.|c10|=|u3|,
 * |  0  0  0 x0 y0  1 -x0*v0 -y0*v0 | |c11| |v0|
 * |  0  0  0 x1 y1  1 -x1*v1 -y1*v1 | |c12| |v1|
 * |  0  0  0 x2 y2  1 -x2*v2 -y2*v2 | |c20| |v2|
 * \  0  0  0 x3 y3  1 -x3*v3 -y3*v3 / \c21/ \v3/
 *
 * where:
 *   cij - matrix coefficients, c22 = 1
 */
Mat calculate_perspective_transform(const Point2f src[], const Point2f dst[]) {
    Mat M(3, 3, FCVImageType::GRAY_F64);
    Mat X(1, 8, FCVImageType::GRAY_F64, M.data());
    double a[8][8];
    double b[8];
    Mat A(8, 8, FCVImageType::GRAY_F64, a);
    Mat B(1, 8, FCVImageType::GRAY_F64, b);

    for(int i = 0; i < 4; i++) {
        a[i][0] = a[i + 4][3] = src[i].x();
        a[i][1] = a[i + 4][4] = src[i].y();
        a[i][2] = a[i + 4][5] = 1;
        a[i][3] = a[i][4] = a[i][5] = a[i + 4][0] = a[i + 4][1] = a[i + 4][2] = 0;
        a[i][6] = -src[i].x() * dst[i].x();
        a[i][7] = -src[i].y() * dst[i].x();
        a[i + 4][6] = -src[i].x()* dst[i].y();
        a[i + 4][7] = -src[i].y() * dst[i].y();
        b[i] = dst[i].x();
        b[i + 4] = dst[i].y();
    }

    perspective_solve(A, B, X);
    M.ptr<double>(0, 0)[8] = 1.;
    return M;
}

G_FCV_NAMESPACE1_END()
