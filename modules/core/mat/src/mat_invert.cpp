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
#include <cmath>

#ifdef HAVE_NEON
#include <arm_neon.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<class T, class D>
static inline double det2(T* data, D step) {
    T m00 = data[0];
    T m11 = (data + step)[1];
    T m01 = (data + step)[0];
    T m10 = data[1];

    return (double)m00 * m11 - (double)m10 * m01;
}

template<class T, class D>
static inline double det3(T* data, D step) {
    T m00 = data[0];
    T m11 = (data + step)[1];
    T m22 = (data + 2 * step)[2];
    T m12 = (data + step)[2];
    T m21 = (data + 2 * step)[1];
    T m01 = (data + step)[0];
    T m10 = data[1];
    T m20 = (data + 2 * step)[0];
    T m02 = data[2];

    return (m00 * ((double)m11 * m22 - (double)m12 * m21) -
            m01 * ((double)m10 * m22 - (double)m12 * m20) +
            m02 * ((double)m10 * m21 - (double)m11 * m20));
}

bool Mat::invert(Mat& dst) const {
    if (_type != FCVImageType::GRAY_F64 && _type != FCVImageType::GRAY_F32) {
        LOG_ERR("The type not support yet");
        return false;
    }

    int m = _height;
    int n = _width;

    if (m != n) {
        LOG_ERR("Mat with unequal width and height is not supported yet!");
        return false;
    }

    if (n <= 3) {
        if (n == 2) {
            if (_type == FCVImageType::GRAY_F32) {
                float* src_data = (float*)_data;
                float* dst_data = (float*)dst.data();
                int src_step = _stride / sizeof(float);
                int dst_step = dst.stride() / sizeof(float);

                double d = det2(src_data, src_step);
                float s00 = src_data[0];
                float s11 = (src_data + src_step)[1];
                float s01 = src_data[1];
                float s10 = (src_data + src_step)[0];

                if (std::fabs(d) >= FCV_DBL_EPSILON) {
                    d = 1. / d;
                    double t0 = s00 * d;
                    double t1 = s11 * d;
                    (dst_data + dst_step)[1] = (float)t0;
                    dst_data[0] = (float)t1;
                    t0 = -s01 * d;
                    t1 = -s10 * d;
                    (dst_data)[1] = (float)t0;
                    (dst_data + dst_step)[0] = (float)t1;
                }
            } else {
                double* src_data = (double*)_data;
                double* dst_data = (double*)dst.data();
                int src_step = _stride / sizeof(double);
                int dst_step = dst.stride() / sizeof(double);

                double d = det2(src_data, src_step);
                double s00 = src_data[0];
                double s11 = (src_data + src_step)[1];
                double s01 = src_data[1];
                double s10 = (src_data + src_step)[0];

                if (std::fabs(d) >= FCV_DBL_EPSILON) {
                    d = 1. / d;
                    double t0 = s00 * d;
                    double t1 = s11 * d;
                    (dst_data + dst_step)[1] = t0;
                    dst_data[0] = t1;
                    t0 = -s01 * d;
                    t1 = -s10 * d;
                    (dst_data)[1] = t0;
                    (dst_data + dst_step)[0] = t1;
                }
            }
        } else if (n == 3) {
            if (_type == FCVImageType::GRAY_F32) {
                float* src_data = (float*)_data;
                float* dst_data = (float*)dst.data();
                int src_step = _stride / sizeof(float);
                int dst_step = dst.stride() / sizeof(float);
                double d = det3(src_data, src_step);

                float s11 = (src_data + src_step)[1];
                float s22 = (src_data + 2 * src_step)[2];
                float s12 = (src_data + src_step)[2];
                float s21 = (src_data + 2 * src_step)[1];
                float s02 = src_data[2];
                float s01 = src_data[1];
                float s20 = (src_data + 2 * src_step)[0];
                float s10 = (src_data + src_step)[0];
                float s00 = src_data[0];

                if (std::fabs(d) >= FCV_DBL_EPSILON) {
                    double t[12];
                    d = 1. / d;
                    t[0] = ((double)s11 * s22 - (double)s12 * s21) * d;
                    t[1] = ((double)s02 * s21 - (double)s01 * s22) * d;
                    t[2] = ((double)s01 * s12 - (double)s02 * s11) * d;

                    t[3] = ((double)s12 * s20 - (double)s10 * s22) * d;
                    t[4] = ((double)s00 * s22 - (double)s02 * s20) * d;
                    t[5] = ((double)s02 * s10 - (double)s00 * s12) * d;

                    t[6] = ((double)s10 * s21 - (double)s11 * s20) * d;
                    t[7] = ((double)s01 * s20 - (double)s00 * s21) * d;
                    t[8] = ((double)s00 * s11 - (double)s01 * s10) * d;

                    dst_data[0] = (float)t[0];
                    dst_data[1] = (float)t[1];
                    dst_data[2] = (float)t[2];
                    (dst_data + dst_step)[0] = (float)t[3];
                    (dst_data + dst_step)[1] = (float)t[4];
                    (dst_data + dst_step)[2] = (float)t[5];
                    (dst_data + 2 * dst_step)[0] = (float)t[6];
                    (dst_data + 2 * dst_step)[1] = (float)t[7];
                    (dst_data + 2 * dst_step)[2] = (float)t[8];
                }
            } else {
                double* src_data = (double*)_data;
                double* dst_data = (double*)dst.data();
                int src_step = _stride / sizeof(double);
                int dst_step = dst.stride() / sizeof(double);
                double d = det3(src_data, src_step);

                double s11 = (src_data + src_step)[1];
                double s22 = (src_data + 2 * src_step)[2];
                double s12 = (src_data + src_step)[2];
                double s21 = (src_data + 2 * src_step)[1];
                double s02 = src_data[2];
                double s01 = src_data[1];
                double s20 = (src_data + 2 * src_step)[0];
                double s10 = (src_data + src_step)[0];
                double s00 = src_data[0];

                if (std::fabs(d) >= FCV_DBL_EPSILON) {
                    d = 1. / d;
                    double t[9];

                    t[0] = (s11 * s22 - s12 * s21) * d;
                    t[1] = (s02 * s21 - s01 * s22) * d;
                    t[2] = (s01 * s12 - s02 * s11) * d;

                    t[3] = (s12 * s20 - s10 * s22) * d;
                    t[4] = (s00 * s22 - s02 * s20) * d;
                    t[5] = (s02 * s10 - s00 * s12) * d;

                    t[6] = (s10 * s21 - s11 * s20) * d;
                    t[7] = (s01 * s20 - s00 * s21) * d;
                    t[8] = (s00 * s11 - s01 * s10) * d;

                    dst_data[0] = t[0];
                    dst_data[1] = t[1];
                    dst_data[2] = t[2];
                    (dst_data + dst_step)[0] = t[3];
                    (dst_data + dst_step)[1] = t[4];
                    (dst_data + dst_step)[2] = t[5];
                    (dst_data + 2 * dst_step)[0] = t[6];
                    (dst_data + 2 * dst_step)[1] = t[7];
                    (dst_data + 2 * dst_step)[2] = t[8];
                }
            }
        } else {
            if (_type == FCVImageType::GRAY_F32) {
                float* src_data = (float*)_data;
                float* dst_data = (float*)dst.data();
                double d = (double)src_data[0];

                if(std::fabs(d) >= FCV_DBL_EPSILON) {
                    dst_data[0] = (float)(1. / d);
                }
            } else {
                double* src_data = (double*)_data;
                double* dst_data = (double*)dst.data();
                double d = (double)src_data[0];

                if(std::fabs(d) >= FCV_DBL_EPSILON) {
                    dst_data[0]= 1. / d;
                }
            }
        }
    } else {
        if (_type == FCVImageType::GRAY_F32) {
            LOG_ERR("Mat height or width > 3 not support yet!");
            return false;
        } else if (_type == FCVImageType::GRAY_F64) {
            LOG_ERR("Mat height or width > 3 not support yet!");
            return false;
        } else {
            LOG_ERR("Mat height or width > 3 not support yet!");
            return false;
        }
    }
    return true;
}

G_FCV_NAMESPACE1_END()
