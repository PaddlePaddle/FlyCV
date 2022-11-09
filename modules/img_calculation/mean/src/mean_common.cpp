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

#include "modules/img_calculation/mean/include/mean_common.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

typedef int (*SumCommonFunc)(const void*, double*, int, int);
typedef int (*SumMaskCommonFunc)(const void*,
        const unsigned char* mask, double*, int, int);
typedef int (*SumRectCommonFunc)(const void*, int, double*,
        int, int, int, int, int);
typedef int (*SumSqrCommonFunc)(const void*, double*, double*, int, int);

template<typename T>
static int sum_common(
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

static int sum_common_u8(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_common(static_cast<const unsigned char*>(src), dst, len, cn);
}

static int sum_common_u16(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_common(static_cast<const unsigned short*>(src), dst, len, cn);
}

static int sum_common_s32(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_common(static_cast<const int*>(src), dst, len, cn);
}

static int sum_common_f32(
        const void* src,
        double* dst,
        int len,
        int cn) {
    return sum_common(static_cast<const float*>(src), dst, len, cn);
}

template<typename T>
static int sum_mask(
        const T* src0,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    const T* src = src0;
    int nzm = 0;

    if (cn == 1) {
        double s = 0;
        for (int i = 0; i < len; i++) {
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
        for (int i = 0; i < len; i++, src += 3) {
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

static int sum_mask_u8(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask(static_cast<const unsigned char*>(src), mask, dst, len, cn);
}

static int sum_mask_u16(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask(static_cast<const unsigned short*>(src), mask, dst, len, cn);
}

static int sum_mask_s32(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask(static_cast<const int*>(src), mask, dst, len, cn);
}

static int sum_mask_f32(
        const void* src,
        const unsigned char* mask,
        double* dst,
        int len,
        int cn) {
    return sum_mask(static_cast<const float*>(src), mask, dst, len, cn);
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
                    s0 += src_start[0]
                    + src_start[cn]
                    + src_start[cn * 2]
                    + src_start[cn * 3];
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

static int sum_rect_u8(
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

static int sum_rect_u16(
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

static int sum_rect_s32(
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

static int sum_rect_f32(
        const void* src,
        int stride,
        double* dst,
        int x_start,
        int y_start,
        int width,
        int height,
        int cn) {
    return sum_rect(static_cast<const float*>(src),
            stride, dst, x_start, y_start, width, height, cn);
}

template<typename T>
static int sum_sqr_common(
        const T* src0,
        double* sum,
        double* suqare_sum,
        int len,
        int cn) {
    const T* src = src0;
    int k = cn % 4;
    if (k == 1) {
        double sum0 = 0;
        double sq_sum0 = 0;
        int i = 0;
        for (; i <= len - 4; i += 4, src += 4) {
            T s0 = src[0], s1 = src[1], s2 = src[2], s3 = src[3];
            sum0 += s0 + s1 + s2 + s3;
            sq_sum0 += (s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        }
        for (; i < len; i++, src += cn) {
            T s0 = src[0];
            sum0 += s0;
            sq_sum0 = s0 * s0;
        }
        sum[0] = sum0;
        suqare_sum[0] = sq_sum0;
    } else if (k == 2) {
        double sum0 = 0, sum1 = 0;
        double sq_sum0 = 0, sq_sum1 = 0;
        for (int i = 0; i < len; i++, src += cn) {
            T s0 = src[0], s1 = src[1];
            sum0 += s0;
            sum1 += s1;
            sq_sum0 += s0 * s0;
            sq_sum1 += s1 * s1;
        }
        sum[0] = sum0;
        sum[1] = sum1;
        suqare_sum[0] = sq_sum0;
        suqare_sum[1] = sq_sum1;
    } else if (k == 3) {
        double sum0 = 0, sum1 = 0, sum2 = 0;
        double sq_sum0 = 0, sq_sum1 = 0, sq_sum2 = 0;
        for (int i = 0 ; i < len; i++, src += cn) {
            T s0 = src[0], s1 = src[1], s2 = src[2];
            sum0 += s0;
            sum1 += s1;
            sum2 += s2;
            sq_sum0 += s0 * s0;
            sq_sum1 += s1 * s1;
            sq_sum2 += s2 * s2;
        }
        sum[0] = sum0;
        sum[1] = sum1;
        sum[2] = sum2;
        suqare_sum[0] = sq_sum0;
        suqare_sum[1] = sq_sum1;
        suqare_sum[2] = sq_sum2;
    } else {
        double sum0 = 0, sum1 = 0, sum2 = 0, sum3 = 0;
        double sq_sum0 = 0, sq_sum1 = 0, sq_sum2 = 0, sq_sum3 = 0;
        for (int i = 0; i < len; i++, src += cn) {
            T s0 = src[0], s1 = src[1], s2 = src[2], s3 = src[2];
            sum0 += s0;
            sum1 += s1;
            sum2 += s2;
            sum3 += s3;
            sq_sum0 += s0 * s0;
            sq_sum1 += s1 * s1;
            sq_sum2 += s2 * s2;
            sq_sum3 += s3 * s3;
        }
        sum[0] = sum0;
        sum[1] = sum1;
        sum[2] = sum2;
        sum[3] = sum3;
        suqare_sum[0] = sq_sum0;
        suqare_sum[1] = sq_sum1;
        suqare_sum[2] = sq_sum2;
        suqare_sum[3] = sq_sum3;
    }

    return len;
}

int sum_sqr_common_u8(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn) {
    return sum_sqr_common(static_cast<const unsigned char*>(src), sum, suqare_sum, len, cn);
}

int sum_sqr_common_u16(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn) {
    return sum_sqr_common(static_cast<const unsigned short*>(src), sum, suqare_sum, len, cn);
}

int sum_sqr_common_s32(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn) {
    return sum_sqr_common(static_cast<const int*>(src), sum, suqare_sum, len, cn);
}

int sum_sqr_common_f32(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn) {
    return sum_sqr_common(static_cast<const float*>(src), sum, suqare_sum, len, cn);
}

static SumCommonFunc get_sum_common_func(DataType type) {
    static std::map<DataType, SumCommonFunc> funcs = {
        {DataType::UINT8, sum_common_u8},
        {DataType::UINT16, sum_common_u16},
        {DataType::SINT32, sum_common_s32},
        {DataType::F32, sum_common_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

static SumMaskCommonFunc get_sum_mask_func(DataType type) {
    static std::map<DataType, SumMaskCommonFunc> funcs = {
        {DataType::UINT8, sum_mask_u8},
        {DataType::UINT16, sum_mask_u16},
        {DataType::SINT32, sum_mask_s32},
        {DataType::F32, sum_mask_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

static SumRectCommonFunc get_sum_rect_func(DataType type) {
    static std::map<DataType, SumRectCommonFunc> funcs = {
        {DataType::UINT8, sum_rect_u8},
        {DataType::UINT16, sum_rect_u16},
        {DataType::SINT32, sum_rect_s32},
        {DataType::F32, sum_rect_f32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

static SumSqrCommonFunc get_sum_sqr_common_func(DataType type) {
    static std::map<DataType, SumSqrCommonFunc> funcs = {
        {DataType::UINT8, sum_sqr_common_u8},
        {DataType::UINT16, sum_sqr_common_u16},
        {DataType::SINT32, sum_sqr_common_s32},
        {DataType::F32, sum_sqr_common_s32}
    };

    if (funcs.find(type) != funcs.end()) {
        return funcs[type];
    } else {
        return nullptr;
    }
}

Scalar mean_common(const Mat& src) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumCommonFunc sum_func = get_sum_common_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();

    double* sum = (double*)malloc(cn * sizeof(double));
    memset(sum, 0, cn * sizeof(double));
    int len = src.stride() * src.height() / (cn * src.type_byte_size());

    //return the number of the total size of input
    int nz = sum_func(src.data(), sum, len, cn);

    for (int i = 0; i < cn; i++) {
        res.set_val(i, sum[i] / nz);
    }

    free(sum);
    return res;
}

Scalar mean_common(const Mat& src, const Mat& mask) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumMaskCommonFunc sum_func = get_sum_mask_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();   //the channel of src

    double* sum = (double*)malloc(cn * sizeof(double));
    memset(sum, 0, cn * sizeof(int));
    int len = src.stride() * src.height() / (cn * src.type_byte_size());

    unsigned char *mask_data = (unsigned char *)mask.data();

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

Scalar mean_common(const Mat& src, const Rect& rect) {
    Scalar res = Scalar::all(0);

    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return res;
    }

    SumRectCommonFunc sum_func = get_sum_rect_func(type_info.data_type);

    if (sum_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return res;
    }

    int cn = src.channels();
    double* sum = (double*)malloc(cn * sizeof(double));
    memset(sum, 0, cn * sizeof(double));

    int x = rect.x();
    int y = rect.y();
    int w = rect.width();
    int h = rect.height();
    int len = w * h;
    int stride = src.stride() / src.type_byte_size();

    //return the number of the total size of input
    sum_func(src.data(), stride, sum, x, y, w, h, cn);

    for (int i = 0; i < cn; i++) {
        res.set_val(i, sum[i] / len);
    }

    free(sum);
    return res;
}

void mean_stddev_common(const Mat& src, Mat& mean, Mat& stddev) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return;
    }

    SumSqrCommonFunc sum_sqr_func = get_sum_sqr_common_func(type_info.data_type);

    if (sum_sqr_func == nullptr) {
        LOG_ERR("There is no matching function!");
        return;
    }

    int cn = src.channels();   //the channel of src

    if (cn == 1) {
        mean = Mat(1, 1, FCVImageType::GRAY_F64);
        stddev = Mat(1, 1, FCVImageType::GRAY_F64);
    } else if (cn == 3) {
        mean = Mat(1, 1, FCVImageType::PKG_BGR_F64);
        stddev = Mat(1, 1, FCVImageType::PKG_BGR_F64);
    } else if (cn == 4) {
        mean = Mat(1, 1, FCVImageType::PKG_BGRA_F64);
        stddev = Mat(1, 1, FCVImageType::PKG_BGRA_F64);
    } else {
        LOG_ERR("Unsupport mat channel, the channel should be 1, 3, 4!");
    }

    double* sum = (double*)malloc((cn * sizeof(double)) << 1);
    memset(sum, 0, (cn * sizeof(double)) << 1);

    double* sum_sqr = sum + cn;
    int len = src.stride() * src.height() / (cn * src.type_byte_size());

    int nz = sum_sqr_func(src.data(), sum, sum_sqr, len, cn);

    double* mean_data = (double*)mean.data();

    for (int i = 0; i < cn; i++) {
       mean_data[i] = (double)sum[i] * (1.0 / nz);
    }

    double* stddev_data = (double*)stddev.data();
    for (int i = 0; i < cn; i++) {
       stddev_data[i] = (double)sqrt((sum_sqr[i] * (1.0 / nz)) - mean_data[i] * mean_data[i]);
    }

    if (sum != NULL) {
        free(sum);
        sum = NULL;
    }
}

G_FCV_NAMESPACE1_END()
