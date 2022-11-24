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

#include <stdlib.h>
#include <cfloat>
#include <cmath>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)
/*
    Perform an affine transform on an input image

    {m[0] m[1] m[2]
     m[3] m[4] m[5]}

    src_x = dst_x * m[0] + dst_y * m[1] + m[2]
    src_y = dst_x * m[3] + dst_y * m[4] + m[5]
*/
static int warp_affine_linear_u8_const(
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
    //get inverse matrix m_ivt
    if ((nullptr == src_data) || (nullptr == dst_data) || (nullptr == m_data)) {
        LOG_ERR("Mat is empty!");
        return -1;
    }

    // double m_ivt[6];
    // inverse_matrix_2x3_float(m_data, m_ivt);

    int round_delta = 1 << (WARP_SCALE_BITS_HALF - 1);

    int bh0 = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_h);
    int bw0 = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / bh0, dst_w);

    int *x_deta = (int *)malloc((dst_w << 1) * sizeof(int));
    int *y_deta = (int *)malloc((dst_h << 1) * sizeof(int));
    short *tab = (short *)malloc((AREA_SZ << 2) * sizeof(short));
    short *src_xy = (short *)malloc((bh0 * bw0 * 2) * sizeof(short));
    short *coeffs = (short *)malloc((AREA_SZ) * sizeof(short));

    init_table_2d_coeff_u8(tab, WARP_INTER_TAB_SIZE);

    int* x00 = (int*)x_deta;
    int* x10 = (int*)(x_deta + dst_w);
    int* y01 = (int*)y_deta;
    int* y11 = (int*)(y_deta + dst_h);

    // fixed-point calculate for x/y coordinates
    int i = 0, j = 0;
    for (i = 0; i < dst_w; i++) {
        x00[i] = fcv_round(m_data[0] * i * AB_SCALE);
        x10[i] = fcv_round(m_data[3] * i * AB_SCALE);
    }

    for (j = 0; j < dst_h; j++) {
        y01[j] = fcv_round((m_data[1] * j + m_data[2]) * AB_SCALE) + round_delta;
        y11[j] = fcv_round((m_data[4] * j + m_data[5]) * AB_SCALE) + round_delta;
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

                    int X = (y01[y_] + x00[x_]) >> WARP_SCALE_BITS_HALF;
                    int Y = (y11[y_] + x10[x_]) >> WARP_SCALE_BITS_HALF;

                    //cal the offset in block, per block width is 32
                    // Y & (WARP_INTER_TAB_SIZE - 1) means calculate the remainder
                    // of (Y / WARP_INTER_TAB_SIZE), X is the same as Y
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

            remap_linear_u8_const(bh, bw, src_data, src_w, src_h, src_stride, dst_ptr,
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

static int warp_affine_linear_f32_const(
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
    int round_delta = 1 << (WARP_SCALE_BITS_HALF - 1);

    int bh0 = FCV_MIN(BLOCK_SIZE_HEIGHT, dst_h);
    int bw0 = FCV_MIN(BLOCK_SIZE_WIDTH * BLOCK_SIZE_HEIGHT / bh0, dst_w);

    int* x_deta = (int *)malloc((dst_w << 1) * sizeof(int));
    int* y_deta = (int *)malloc((dst_h << 1) * sizeof(int));
    float* tab = (float *)malloc((AREA_SZ << 2) * sizeof(float)); //store the coefficient four points coordinata
    short* src_xy = (short *)malloc((bh0 * bw0 * 2) * sizeof(short));
    short* coeffs = (short *)malloc((AREA_SZ) * sizeof(short));

    //dividing the peocessing width by 32
    init_table_2d_coeff_f32(tab, WARP_INTER_TAB_SIZE);

    int* x00 = (int*)x_deta;
    int* x10 = (int*)(x_deta + dst_w);
    int* y01 = (int*)y_deta;
    int* y11 = (int*)(y_deta + dst_h);

    int i = 0, j = 0;
    for (; i < dst_w; i++) {
        x00[i] = fcv_round(m_data[0] * i * AB_SCALE);
        x10[i] = fcv_round(m_data[3] * i * AB_SCALE);
    }

    for (; j < dst_h; j++) {
        y01[j] = fcv_round((j * m_data[1] + m_data[2]) * AB_SCALE) + round_delta;
        y11[j] = fcv_round((j * m_data[4] + m_data[5]) * AB_SCALE) + round_delta;
    }

    for (i = 0; i < dst_h; i += bh0) {
        int bh = FCV_MIN(bh0, dst_h - i);
        float* ptr_dst = (float *)(dst_data + dst_stride * i);
        for (j = 0; j < dst_w; j += bw0) {
            int bw = FCV_MIN(bw0, dst_w - j);
            for (int y = 0; y < bh; y++) {
                short* map_row = (short*)(src_xy + (bw * (y << 1)));
                short* coeffs_row = (short*)(coeffs + (bw * y));
                int y_ = y + i;

                for (int x = 0 ; x < bw; x++) {
                    int x_ = x + j;

                    // the grid coordinate
                    int X = (y01[y_] + x00[x_]) >> WARP_SCALE_BITS_HALF;
                    int Y = (y11[y_] + x10[x_]) >> WARP_SCALE_BITS_HALF;

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

static void warp_affine_linear_const_common(
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
        warp_affine_linear_u8_const((unsigned char*)src_data, (unsigned char*)dst_data,
                src_h, src_w, src_stride, dst_h, dst_w, dst_stride, sc, m_data, border_value);
        break;
    case FCVImageType::GRAY_F32:
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGB_F32:
        warp_affine_linear_f32_const((float*)src_data, (float*)dst_data, src_h,
                src_w, src_stride / sizeof(float), dst_h, dst_w, dst_stride / sizeof(float),
                sc, m_data, border_value);
        break;
    default:
        LOG_ERR("warp_affine data type not support yet!");
        break;
    };
}

static int warp_affine_linear_commmon(
        const Mat& src,
        Mat& dst,
        const double *m_data,
        BorderType border_method,
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

    if (sc > 0) {
        switch (border_method) {
        case BorderType::BORDER_CONSTANT:
            warp_affine_linear_const_common(src_data, dst_data, src_h, src_w,
                    src_stride, dst_h, dst_w, dst_stride, sc, m_data, src.type(), border_value);
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

int warp_affine_commmon(
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
        double m_ivt[6];
        inverse_matrix_2x3_float(m_data, m_ivt);
        warp_affine_linear_commmon(src, dst, m_ivt, border_method, border_value);
        break;
    case InterpolationType::WARP_INVERSE_MAP:
        warp_affine_linear_commmon(src, dst, m_data, border_method, border_value);
        break;
    default:
        LOG_ERR("warp_affine interpolation type not support yet!");
        break;
    };

    return 0;
}

Mat get_rotation_matrix_2D(Point2f center, double angle, double scale) {
    angle *= FCV_PI / 180;
    double alpha = cos(angle) * scale;
    double beta  = sin(angle) * scale;

    Mat M(3, 2, FCVImageType::GRAY_F64);
    double* m = (double*)M.data();

    m[0] = alpha;
    m[1] = beta;
    m[2] = (1 - alpha) * center.x() - beta * center.y();
    m[3] = -beta;
    m[4] = alpha;
    m[5] = beta * center.x() + (1 - alpha) * center.y();

    return M;
}

template<typename _Tp> static int
LUImpl(_Tp* A,
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

int LU64f(double* A,
        size_t astep,
        int m,
        double* b,
        size_t bstep,
        int n) {
    int res = -1;
    res = LUImpl(A, astep, m, b, bstep, n, (double)(FLT_EPSILON * 100));
    return res;
}

void solve(Mat& a, Mat& b, Mat& dst) {
    double *buffer = (double *)malloc(sizeof(double) * 36);
    Mat a0(a.width(), a.height(), a.type(), buffer);
    a.copy_to(a0);
    b.copy_to(dst);

    LU64f((double*)a0.data(), a0.stride(), a0.width(),
            (double*)dst.data(), dst.stride(), dst.width());

    free(buffer);
}

/* Calculates coefficients of affine transformation
 * which maps (xi,yi) to (ui,vi), (i=1,2,3):
 *
 * ui = c00*xi + c01*yi + c02
 *
 * vi = c10*xi + c11*yi + c12
 *
 * Coefficients are calculated by solving linear system:
 * / x0 y0  1  0  0  0 \ /c00\ /u0\
 * | 0  0  0  x0 y0  1 | |c01| |u1|
 * | x1 y1  1  0  0  0 | |c02| |u2|
 * |  0  0  0  x1 y1  1| |c10| |v0|
 * | x2 y2  1  0  0  0 | |c11| |v1|
 * \  0  0  0 x2 y2  1 / |c12| |v2|
 *
 * where:
 *   cij - matrix coefficients
 */

Mat get_affine_transform(const Point2f src[], const Point2f dst[]) {
    Mat M(3, 2, FCVImageType::GRAY_F64);
    Mat X(1, 6, FCVImageType::GRAY_F64, M.data());
    double a[6 * 6];
    double b[6];
    Mat A(6, 6, FCVImageType::GRAY_F64, a);
    Mat B(1, 6, FCVImageType::GRAY_F64, b);

    for(int i = 0; i < 3; i++) {
        int j = i * 12;
        int k = i * 12 + 6;
        a[j    ] = a[k + 3] = src[i].x();
        a[j + 1] = a[k + 4] = src[i].y();
        a[j + 2] = a[k + 5] = 1;
        a[j + 3] = a[j + 4] = a[j + 5] = 0;
        a[k    ] = a[k + 1] = a[k + 2] = 0;
        b[i * 2] = dst[i].x();
        b[i*2+1] = dst[i].y();
    }

    solve(A, B, X);
    return M;
}

G_FCV_NAMESPACE1_END()
