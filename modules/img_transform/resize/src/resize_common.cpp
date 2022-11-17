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

#include "modules/img_transform/resize/include/resize_common.h"

#include <cmath>
#include <stdlib.h>
#include <cassert>
#include <algorithm>

#include "modules/img_transform/resize/interface/resize.h"
#include <float.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static void cal_coord_and_coeff(
        int dst_size,
        double scale,
        int src_size,
        int channel,
        int* ofs,
        float* alpha) {

    for (int dst_xy = 0; dst_xy < dst_size; dst_xy++) {
        // The point (x, y) in the destination image corresponds to the point in the source image
        // +0.5（-0.5） center aligned
        double src_xy = (double) ((dst_xy + 0.5) * scale - 0.5);
        // Round down to find the nearest integer the point (x, y)
        int src_xy_down = (int) floor(static_cast<float>(src_xy));
        // Find the nearest integer distance from the point (x, y) in the target image, f(i+u, j+v)
        float uv_xy = src_xy - src_xy_down;

        // Check for minimum out-of-bounds
        if (src_xy_down < 0) {
            src_xy_down = 0;
            uv_xy = 0.f;
        }

        // Check for max out-of-bounds
        if (src_xy_down >= src_size - 1) {
            src_xy_down = src_size - 2;
            uv_xy = 1.f;
        }

        // Stores a collection of nearest point (x, y) integers，same for each channel
        ofs[dst_xy] = src_xy_down * channel;

        // Stores a collection of nearest integer distance from the point (x, y) in the target image, u and v
        int id_xy = dst_xy << 1;
        alpha[id_xy] = 1.f - uv_xy;
        alpha[id_xy + 1] = uv_xy;
    }
}

static void cal_coord_and_coeff(
        int dst_size,
        double scale,
        int src_size,
        int channel,
        int* ofs,
        unsigned short* alpha) {

    for (int dst_xy = 0; dst_xy < dst_size; dst_xy++) {
        // The point (x, y) in the destination image corresponds to the point in the source image
        // +0.5（-0.5） center aligned
        double src_xy = (double) ((dst_xy + 0.5) * scale - 0.5);
        // Round down to find the nearest integer the point (x, y)
        int src_xy_down = (int) floor(static_cast<float>(src_xy));
        src_xy -= src_xy_down;
        int uv_xy = static_cast<int>(fcv_round(src_xy * INTER_RESIZE_COEF_SCALE));

        // Check for minimum out-of-bounds
        if (src_xy_down < 0) {
            src_xy_down = 0;
            uv_xy = 0;
        }

        // Check for max out-of-bounds
        if (src_xy_down >= src_size - 1) {
            src_xy_down = src_size - 2;
            uv_xy = 2048;
        }

        // Stores a collection of nearest point (x, y) integers，same for each channel
        ofs[dst_xy] = src_xy_down * channel;

        // Stores a collection of nearest integer distance from the point (x, y) in the target image, u and v
        int id_xy = dst_xy << 1;
        alpha[id_xy] = fcv_cast_u16(2048 - uv_xy);
        alpha[id_xy + 1] = fcv_cast_u16(uv_xy);
    }
}

static void cal_coord_and_coeff(
        int dst_size,
        double scale,
        int src_size,
        int channel,
        int* ofs,
        uint8_t* alpha) {

    for (int dst_xy = 0; dst_xy < dst_size; dst_xy++) {
        // The point (x, y) in the destination image corresponds to the point in the source image
        // +0.5（-0.5） center aligned
        double src_xy = (double) ((dst_xy + 0.5) * scale - 0.5);
        // Round down to find the nearest integer the point (x, y)
        int src_xy_down = (int) floor(static_cast<float>(src_xy));
        src_xy -= src_xy_down;
        int uv_xy = static_cast<int>(fcv_round(src_xy * 128));

        // Check for minimum out-of-bounds
        if (src_xy_down < 0) {
            src_xy_down = 0;
            uv_xy = 0;
        }

        // Check for max out-of-bounds
        if (src_xy_down >= src_size - 1) {
            src_xy_down = src_size - 2;
            uv_xy = 128;
        }

        // Stores a collection of nearest point (x, y) integers，same for each channel
        ofs[dst_xy] = src_xy_down * channel;

        // Stores a collection of nearest integer distance from the point (x, y) in the target image, u and v
        int id_xy = dst_xy << 1;
        alpha[id_xy] = fcv_cast_u8(128 - uv_xy);
        alpha[id_xy + 1] = fcv_cast_u8(uv_xy);
    }
}

void nearest_cal_offset(
        int* offset,
        const int dst_size,
        const int src_size) {
    int fixed_scale = (int)((src_size << 16) / dst_size);
    offset[0] = fixed_scale >> 1;
    int dx = 0;
    for (dx = 1; dx < dst_size - 7; dx += 8) {
        offset[dx] = fixed_scale * dx + offset[0];
        offset[dx+1] = fixed_scale * (dx + 1) + offset[0];
        offset[dx+2] = fixed_scale * (dx + 2) + offset[0];
        offset[dx+3] = fixed_scale * (dx + 3) + offset[0];
        offset[dx+4] = fixed_scale * (dx + 4) + offset[0];
        offset[dx+5] = fixed_scale * (dx + 5) + offset[0];
        offset[dx+6] = fixed_scale * (dx + 6) + offset[0];
        offset[dx+7] = fixed_scale * (dx + 7) + offset[0];
    }

    for (; dx < dst_size; ++dx) {
        offset[dx] = fixed_scale * dx + offset[0];
    }
}

#define  GetBnResizeBufPreparation(type)                                    \
    double scale_x = (double)src_w / w;                                   \
    double scale_y = (double)src_h / h;                                   \
    int dou_w = w + w;                                                    \
    int* xofs = *buf;                                                     \
    int* yofs = xofs + w;                                                 \
    type* alpha = (type*)(yofs + h);                                     \
    type* beta  = (type*)(alpha + dou_w);

void get_resize_bilinear_buf_c1(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        int** buf) {
    GetBnResizeBufPreparation(uint8_t);

    cal_coord_and_coeff(w, scale_x, src_w, c, xofs, alpha);
    cal_coord_and_coeff(h, scale_y, src_h, 1, yofs, beta);
}

void get_resize_bilinear_buf(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        int** buf) {
    GetBnResizeBufPreparation(unsigned short);

    cal_coord_and_coeff(w, scale_x, src_w, c, xofs, alpha);
    cal_coord_and_coeff(h, scale_y, src_h, 1, yofs, beta);
}

void get_resize_bilinear_buf(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        float** coeff_buf,
        int** coord_buf) {
    double scale_x = (double)src_w / w;
    double scale_y = (double)src_h / h;
    int* xofs = *coord_buf;
    int* yofs = xofs + w;

    float* alpha = *coeff_buf;
    float* beta = alpha + w + w;

    cal_coord_and_coeff(w, scale_x, src_w, c, xofs, alpha);
    cal_coord_and_coeff(h, scale_y, src_h, 1, yofs, beta);
}

void hresize_bn_one_row(
        const unsigned char* src_ptr,
        int* xofs,
        int sy,
        int stride,
        int w,
        int c,
        unsigned short* alphap,
        unsigned short* rows0,
        unsigned short* rows1) {
    const int sy_off = sy * stride;
    const unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
    const unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + stride);

    unsigned short* rows0p = rows0;
    unsigned short* rows1p = rows1;

    int dx = 0, dc = 0;
    for (; dx < w; dx++) {
        int cx = xofs[dx];
        const unsigned char* S0p = src0 + cx;
        const unsigned char* S1p = src1 + cx;

        unsigned short a0 = *(alphap++);
        unsigned short a1 = *(alphap++);

        for (dc = 0; dc < c; dc++) {
            rows0p[dc]= (S0p[dc] * a0 + S0p[dc + c] * a1) >> 4;
            rows1p[dc]= (S1p[dc] * a0 + S1p[dc + c] * a1) >> 4;
        }

        rows0p += c;
        rows1p += c;
    }
}

void hresize_bn_one_row(
        const float* src_ptr,
        int* xofs,
        int sy,
        int stride,
        int w,
        int c,
        float* alphap,
        float* rows0,
        float* rows1) {
    const int sy_off = sy * stride;
    const float *src0 = (float*)(src_ptr + sy_off);
    const float *src1 = (float*)(src_ptr + sy_off + stride);

    float* rows0p = rows0;
    float* rows1p = rows1;

    int dx = 0, dc = 0;
    for (; dx < w; dx++) {
        int cx = xofs[dx];
        const float* S0p = src0 + cx;
        const float* S1p = src1 + cx;

        float a0 = *(alphap++);
        float a1 = *(alphap++);

        for (dc = 0; dc < c; dc++) {
            rows0p[dc]= S0p[dc] * a0 + S0p[dc + c] * a1;
            rows1p[dc]= S1p[dc] * a0 + S1p[dc + c] * a1;
        }

        rows0p += c;
        rows1p += c;
    }
}

void resize_bilinear_cn_common(
        const unsigned char* src,
        unsigned char* dst,
        const int src_w,
        const int src_h,
        const int dst_w,
        const int dst_h,
        const int src_stride,
        const int dst_stride,
        const int channel) {
    int* buf = nullptr;
    unsigned short *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = dst_stride << 3;
    buf = (int *)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, channel, &buf);

    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short* rows0 = nullptr;
    unsigned short* rows1 = nullptr;
    unsigned char* ptr_dst = nullptr;

    int dy = 0, dx= 0, sy0 = 0;
    for (; dy < dst_h; dy++) {
        rows0 = rows;
        rows1 = rows + dst_stride;
        sy0 = *(yofs + dy);
        unsigned short *alphap = alpha;
        ptr_dst = dst + dst_stride * dy;

        hresize_bn_one_row(src, xofs, sy0, src_stride,
                dst_w, channel, alphap, rows0, rows1);

        unsigned int b0 = *(beta++);
        unsigned int b1 = *(beta++);
        for (dx = 0; dx < dst_stride; dx++) {
            *(ptr_dst++) = fcv_cast_u8((*(rows0++) * b0 + *(rows1++) * b1 + (1 << 17)) >> 18);
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_bilinear_cn_common(
        const float* src,
        float* dst,
        const int src_w,
        const int src_h,
        const int dst_w,
        const int dst_h,
        const int src_stride,
        const int dst_stride,
        const int channel) {
    int size = dst_w + dst_h;
    int rows_size = dst_stride << 1;

    float* coeff_buf = (float *)malloc(size * 2 * sizeof(float));
    int* coord_buf = (int *)malloc(size * sizeof(int));
    float* rows = (float*)malloc(rows_size * sizeof(float));
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, channel, &coeff_buf, &coord_buf);

    int* xofs = coord_buf;
    int* yofs = coord_buf + dst_w;
    float* alpha = (float*)coeff_buf;
    float* beta  = (float*)(alpha + dst_w + dst_w);

    float* rows0 = nullptr;
    float* rows1 = nullptr;
    float* ptr_dst = nullptr;
    int dy = 0;
    int dx= 0;
    int sy0 = 0;

    for (; dy < dst_h; dy++) {
        rows0 = rows;
        rows1 = rows + dst_stride / 4;
        sy0 = *(yofs + dy);
        float *alphap = alpha;
        ptr_dst = dst + dst_stride / 4 * dy;

        hresize_bn_one_row(src, xofs, sy0, src_stride / 4,
                dst_w, channel, alphap, rows0, rows1);

        float b0 = *(beta++);
        float b1 = *(beta++);

        for (dx = 0; dx < dst_stride / 4; dx++) {
            *(ptr_dst++) = *(rows0++) * b0 + *(rows1++) * b1;
        }
    }

    if (coeff_buf != nullptr) {
        free(coeff_buf);
        coeff_buf = nullptr;
    }

    if (coord_buf != nullptr) {
        free(coord_buf);
        coord_buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_bilinear_yuv_common(
        const unsigned char* src,
        unsigned char* dst,
        const int src_w,
        const int src_h,
        const int dst_w,
        const int dst_h,
        const int src_stride,
        const int dst_stride) {
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;

    unsigned char *dst_uv_ptr = dst + dst_stride * dst_h;
    const unsigned char *src_uv_ptr = src + src_stride * src_h;

    const int d2_stride = dst_stride << 1;
    const int d3_stride = dst_stride + d2_stride;

    int* buf = nullptr;
    unsigned short *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = dst_stride << 3;
    buf = (int*)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 1, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows00 = nullptr;
    unsigned short *rows01 = nullptr;
    unsigned short *rows10 = nullptr;
    unsigned short *rows11 = nullptr;
    unsigned char *ptr_dst0 = nullptr;
    unsigned char *ptr_dst1 = nullptr;
    unsigned char *ptr_uv = nullptr;

    int dy = 0, dx= 0, sy0 = 0, sy1 = 0;
    for (; dy < dst_h; dy += 2) {
        const int dst_offset = dst_w * dy;
        rows00 = rows;
        rows01 = rows + dst_stride;
        rows10 = rows + d2_stride;
        rows11 = rows + d3_stride;
        sy0 = *(yofs + dy);
        sy1 = *(yofs + dy + 1);

        unsigned short *alphap = alpha;
        ptr_dst0 = dst + dst_offset;
        ptr_dst1 = dst + dst_offset + dst_w;
        ptr_uv = dst_uv_ptr + (dst_offset >> 1);

        hresize_bn_one_row(src, xofs, sy0, src_w,
                dst_w, 1, alphap, rows00, rows01);
        hresize_bn_one_row(src, xofs, sy1, src_w,
                dst_w, 1, alphap, rows10, rows11);

        unsigned int b0 = *(beta++);
        unsigned int b1 = *(beta++);

        unsigned int b2 = *(beta++);
        unsigned int b3 = *(beta++);

        int src_y = (int)((dy >> 1) * scale_y);
        unsigned char uv0 = 0, uv1 = 0;
        const unsigned char *uv_ptr = src_uv_ptr + src_y * src_w;
        for (dx = 0; dx < dst_w; dx += 2) {
            int src_x = (int)(dx * scale_x) / 2 * 2;
            // calculate y
            *(ptr_dst0++) = fcv_cast_u8((*(rows00++) * b0 +
                    *(rows01++) * b1 + (1 << 17)) >> 18);
            *(ptr_dst0++) = fcv_cast_u8((*(rows00++) * b0 +
                    *(rows01++) * b1 + (1 << 17)) >> 18);

            *(ptr_dst1++) = fcv_cast_u8((*(rows10++) * b2 +
                    *(rows11++) * b3 + (1 << 17)) >> 18);
            *(ptr_dst1++) = fcv_cast_u8((*(rows10++) * b2 +
                    *(rows11++) * b3 + (1 << 17)) >> 18);

            // calculate uv, nearest interpolation
            uv0 = uv_ptr[src_x];
            uv1 = uv_ptr[src_x + 1];
            *(ptr_uv++) = uv0; *(ptr_uv++) = uv1;
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

int resize_bilinear_common(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_bilinear_cn_common((const unsigned char*)src.data(),
                (unsigned char*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 1);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_bilinear_cn_common((const unsigned char*)src.data(),
                (unsigned char*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 3);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        resize_bilinear_cn_common((const unsigned char*)src.data(),
                (unsigned char*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 4);
        break;
    case FCVImageType::GRAY_F32:
        resize_bilinear_cn_common((const float*)src.data(),
                (float*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 1);
        break;
    case FCVImageType::PKG_RGB_F32:
    case FCVImageType::PKG_BGR_F32:
        resize_bilinear_cn_common((const float*)src.data(), 
                (float*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 3);
        break;
    case FCVImageType::PKG_RGBA_F32:
    case FCVImageType::PKG_BGRA_F32:
        resize_bilinear_cn_common((const float*)src.data(),
                (float*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride(), 4);
        break;
    case FCVImageType::NV12:
    case FCVImageType::NV21:
        resize_bilinear_yuv_common((const unsigned char*)src.data(),
                (unsigned char*)dst.data(), src.width(), src.height(),
                dst.width(), dst.height(), src.stride(), dst.stride());
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

static void cal_ycoord_cubic_coeff(
        int size,
        int s_size,
        float scale,
        int *offset,
        short *coeff) {
    float cbuf[4];
    short ty[4];
    const float A = -0.75f;

    short *buf = coeff;

    for (int dy = 0; dy < size; dy++) {
        float fy = (float)((dy + 0.5) * scale - 0.5);
        int sy = fcv_floor(fy);
        fy -= sy;
        sy -= 1;

        cbuf[0] = ((A * (fy + 1) - 5 * A) * (fy + 1) + 8 * A) * (fy + 1) - 4 * A;
        cbuf[1] = ((A + 2) * fy - (A + 3)) * fy * fy + 1;
        cbuf[2] = ((A + 2) * (1 - fy) - (A + 3)) * (1 - fy) * (1 - fy) + 1;
        cbuf[3] = 1.f - cbuf[0] - cbuf[1] - cbuf[2];

        ty[0] = fcv_cast_s16(fcv_round(cbuf[0] * 2048));
        ty[1] = fcv_cast_s16(fcv_round(cbuf[1] * 2048));
        ty[2] = fcv_cast_s16(fcv_round(cbuf[2] * 2048));
        ty[3] = fcv_cast_s16(fcv_round(cbuf[3] * 2048));

        if (sy >= 0 && sy <= (s_size - 4)) {
            offset[dy] = sy;
            buf[0] = ty[0];
            buf[1] = ty[1];
            buf[2] = ty[2];
            buf[3] = ty[3];
        } else if ((-2) == sy) {
            offset[dy] = 0;
            buf[0] = ty[0] + ty[1] + ty[2];
            buf[1] = ty[3];
            buf[2] = 0;
            buf[3] = 0;
        } else if ((-1) == sy) {
            offset[dy] = 0;
            buf[0] = ty[0] + ty[1];
            buf[1] = ty[2];
            buf[2] = ty[3];
            buf[3] = 0;
        } else if ((s_size - 3) == sy) {
            offset[dy] = s_size - 4;
            buf[0] = 0;
            buf[1] = ty[0];
            buf[2] = ty[1];
            buf[3] = ty[2] + ty[3];
        } else if ((s_size - 2) == sy) {
            offset[dy] = s_size - 4;
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = ty[0];
            buf[3] = ty[1] + ty[2] + ty[3];
        }

        buf += 4;
    }
}

static void cal_xcoord_cubic_coeff(
        int size,
        int s_size,
        float scale,
        int c,
        int *offset,
        short *coeff) {
    float cbuf[4];
    short tx[4];
    const float A = -0.75f;

    short *buf = coeff;

    for (int dx = 0; dx < size; dx++) {
        float fx = (float)((dx + 0.5) * scale - 0.5);
        int sx = fcv_floor(fx);
        fx -= sx;
        sx -= 1;

        cbuf[0] = ((A * (fx + 1) - 5 * A) * (fx + 1) + 8 * A) * (fx + 1) - 4 * A;
        cbuf[1] = ((A + 2) * fx - (A + 3)) * fx * fx + 1;
        cbuf[2] = ((A + 2) * (1 - fx) - (A + 3)) * (1 - fx) * (1 - fx) + 1;
        cbuf[3] = 1.f - cbuf[0] - cbuf[1] - cbuf[2];

        tx[0] = fcv_cast_s16(fcv_round(cbuf[0] * 2048));
        tx[1] = fcv_cast_s16(fcv_round(cbuf[1] * 2048));
        tx[2] = fcv_cast_s16(fcv_round(cbuf[2] * 2048));
        tx[3] = fcv_cast_s16(fcv_round(cbuf[3] * 2048));

        if (sx >= 0 && sx <= (s_size - 4)) {
            offset[dx] = sx * c;
            buf[0] = tx[0];
            buf[1] = tx[1];
            buf[2] = tx[2];
            buf[3] = tx[3];
        } else if ((-2) == sx) {
            offset[dx] = 0;
            buf[0] = tx[0] + tx[1] + tx[2];
            buf[1] = tx[3];
            buf[2] = 0;
            buf[3] = 0;
        } else if ((-1) == sx) {
            offset[dx] = 0;
            buf[0] = tx[0] + tx[1];
            buf[1] = tx[2];
            buf[2] = tx[3];
            buf[3] = 0;
        } else if ((s_size - 3) == sx) {
            offset[dx] = (s_size - 4) * c;
            buf[0] = 0;
            buf[1] = tx[0];
            buf[2] = tx[1];
            buf[3] = tx[2] + tx[3];
        } else if ((s_size - 2) == sx) {
            offset[dx] = (s_size - 4) * c;
            buf[0] = 0;
            buf[1] = 0;
            buf[2] = tx[0];
            buf[3] = tx[1] + tx[2] + tx[3];
        }

        buf += 4;
    }
}

void get_resize_cubic_buf(
        int src_w,
        int src_h,
        int dst_w,
        int dst_h,
        int c,
        int** buf) {
    float scale_x = (float)src_w / dst_w;
    float scale_y = (float)src_h / dst_h;
    int fou_w = dst_w << 2;
    int* xofs = *buf;
    int* yofs = xofs + dst_w;
    short* alpha = (short*)(yofs + dst_h);
    short* beta  = (short*)(alpha + fou_w);

    cal_xcoord_cubic_coeff(dst_w, src_w, scale_x, c, xofs, alpha);
    cal_ycoord_cubic_coeff(dst_h, src_h, scale_y, yofs, beta);
}

void resize_cubic_cn_common(Mat& src, Mat& dst, int channels) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_stride = src.stride();
    const int d_stride = dst.stride();

    int* buf = nullptr;
    int* rows = nullptr;
    int buf_size  = (dst_h + dst_w) * sizeof(int) + (dst_h + dst_w) * 4 * sizeof(short);
    int rows_size = d_stride * 4 * sizeof(int);
    buf = (int*)malloc(buf_size);
    rows = (int*)malloc(rows_size);
    get_resize_cubic_buf(src_w, src_h, dst_w, dst_h, channels, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    short* alpha = (short*)(yofs + dst_h);
    short* beta = (short*)(alpha + (dst_w << 2));

    int* rows0 = nullptr;
    int* rows1 = nullptr;
    int* rows2 = nullptr;
    int* rows3 = nullptr;

    unsigned char* ptr_dst = nullptr;

    rows0 = rows;
    rows1 = rows0 + d_stride;
    rows2 = rows1 + d_stride;
    rows3 = rows2 + d_stride;
    for (int i = 0; i < dst_h; i++) {
        int sy = yofs[i];
        const unsigned char *src0 = (unsigned char*)(src_ptr + sy * s_stride);
        const unsigned char *src1 = (unsigned char*)(src_ptr + (sy + 1) * s_stride);
        const unsigned char *src2 = (unsigned char*)(src_ptr + (sy + 2) * s_stride);
        const unsigned char *src3 = (unsigned char*)(src_ptr + (sy + 3) * s_stride);

        ptr_dst = dst_ptr + d_stride * i;

        short *alpha0 = alpha;
        int *rows0p = rows0;
        int *rows1p = rows1;
        int *rows2p = rows2;
        int *rows3p = rows3;

        int dx = 0;
        for (; dx < dst_w; dx++) {
            int sx = xofs[dx];
            const unsigned char *s0p = src0 + sx;
            const unsigned char *s1p = src1 + sx;
            const unsigned char *s2p = src2 + sx;
            const unsigned char *s3p = src3 + sx;

            short a0 = alpha0[0];
            short a1 = alpha0[1];
            short a2 = alpha0[2];
            short a3 = alpha0[3];

            for (int dc = 0; dc < channels; dc++) {
                rows0p[dc] = s0p[0 + dc] * a0 + s0p[channels + dc] * a1 +
                        s0p[2 * channels + dc] * a2 + s0p[3 * channels + dc] * a3;
                rows1p[dc] = s1p[0 + dc] * a0 + s1p[channels + dc] * a1 +
                        s1p[2 * channels + dc] * a2 + s1p[3 * channels + dc] * a3;
                rows2p[dc] = s2p[0 + dc] * a0 + s2p[channels + dc] * a1 +
                        s2p[2 * channels + dc] * a2 + s2p[3 * channels + dc] * a3;
                rows3p[dc] = s3p[0 + dc] * a0 + s3p[channels + dc] * a1 +
                        s3p[2 * channels + dc] * a2 + s3p[3 * channels + dc] * a3;
            }

            alpha0 += 4;
            rows0p += channels;
            rows1p += channels;
            rows2p += channels;
            rows3p += channels;
        }

        short b0 = beta[0];
        short b1 = beta[1];
        short b2 = beta[2];
        short b3 = beta[3];

        int *rows0v = rows0;
        int *rows1v = rows1;
        int *rows2v = rows2;
        int *rows3v = rows3;

        for (dx = 0; dx < d_stride; dx++) {
            int res = (*rows0v * b0 + *rows1v * b1 + *rows2v * b2 + *rows3v * b3);
            *(ptr_dst++) = fcv_cast_u8((FCV_MAX(res, 0) + (1 << 21)) >> 22);

            rows0v++;
            rows1v++;
            rows2v++;
            rows3v++;
        }

        beta += 4;
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

int resize_cubic_common(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_cubic_cn_common(src, dst, 1);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_cubic_cn_common(src, dst, 3);
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

void get_resize_area_buf_c1(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        double inv_scale_x,
        double inv_scale_y,
        int** buf) {
    double scale_x = 1. / inv_scale_x;
    double scale_y = 1. / inv_scale_y;
    int dou_w = w + w;
    int* xofs = *buf;
    int* yofs = xofs + w;
    uint8_t* alpha = (uint8_t*)(yofs + h);
    uint8_t* beta  = (uint8_t*)(alpha + dou_w);

    double fx = 0.f;
    int sx = 0, dx = 0, tx = 0;
    for (; dx < w; dx++) {
        sx = fcv_floor(dx * scale_x);
        fx = (float)((dx + 1) - (sx + 1) * inv_scale_x);
        fx = fx <= 0 ? 0.f : fx - fcv_floor(fx);
        tx = static_cast<uint8_t>(fcv_round(fx * 128));
        if (sx < 0) {
            sx = 0;
            tx = 0;
        }
        if (sx >= src_w - 1) {
            sx = src_w - 2;
            tx = 128;
        }
        int idx = dx << 1;
        xofs[dx] = sx * c;
        alpha[idx] = fcv_cast_u8(128 - tx);
        alpha[idx + 1] = fcv_cast_u8(tx);
    }

    double fy = 0.f;
    int dy = 0, sy = 0, ty = 0;
    for (; dy < h; dy++) {
        sy = fcv_floor(dy * scale_y);
        fy = (float)((dy + 1) - (sy + 1) * inv_scale_y);
        fy = fy <= 0 ? 0.f : fy - fcv_floor(fy);
        ty = static_cast<uint8_t>(fcv_round(fy * 128));
        if (sy < 0) {
            sy = 0;
            ty = 0;
        }
        if (sy >= src_h - 1) {
            sy = src_h - 2;
            ty = 128;
        }
        int idy = dy << 1;
        yofs[dy] =  sy;
        beta[idy] = fcv_cast_u8(128 - ty);
        beta[idy + 1] = fcv_cast_u8(ty);
    }
}

void get_resize_area_buf(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        double inv_scale_x,
        double inv_scale_y,
        int** buf) {
    double scale_x = 1. / inv_scale_x;
    double scale_y = 1. / inv_scale_y;
    int dou_w = w + w;
    int* xofs = *buf;
    int* yofs = xofs + w;
    unsigned short* alpha = (unsigned short*)(yofs + h);
    unsigned short* beta  = (unsigned short*)(alpha + dou_w);

    double fx = 0.f;
    int sx = 0, dx = 0, tx = 0;
    for (; dx < w; dx++) {
        sx = fcv_floor(dx * scale_x);
        fx = (float)((dx + 1) - (sx + 1) * inv_scale_x);
        fx = fx <= 0 ? 0.f : fx - fcv_floor(fx);
        tx = static_cast<int>(fcv_round(fx * INTER_RESIZE_COEF_SCALE));
        if (sx < 0) {
            sx = 0;
            tx = 0;
        }
        if (sx >= src_w - 1) {
            sx = src_w - 2;
            tx = 2048;
        }
        int idx = dx << 1;
        xofs[dx] = sx * c;
        alpha[idx] = fcv_cast_u16(2048 - tx);
        alpha[idx + 1] = fcv_cast_u16(tx);
    }

    double fy = 0.f;
    int dy = 0, sy = 0, ty = 0;
    for (; dy < h; dy++) {
        sy = fcv_floor(dy * scale_y);
        fy = (float)((dy + 1) - (sy + 1) * inv_scale_y);
        fy = fy <= 0 ? 0.f : fy - fcv_floor(fy);
        ty = static_cast<int>(fcv_round(fy * INTER_RESIZE_COEF_SCALE));
        if (sy < 0) {
            sy = 0;
            ty = 0;
        }
        if (sy >= src_h - 1) {
            sy = src_h - 2;
            ty = 2048;
        }
        int idy = dy << 1;
        yofs[dy] =  sy;
        beta[idy] = fcv_cast_u16(2048 - ty);
        beta[idy + 1] = fcv_cast_u16(ty);
    }
}

struct DecimateAlpha {
    int si;
    int di;
    float alpha;
};

template<typename T>
static unsigned char fcv_saturate_cast(T val) {
    return (unsigned char)std::max(std::min((int)std::round(val), (int)U8_MAX_VAL), 0);
}

static int compute_resize_area_tab(
        int ssize,
        int dsize,
        int cn,
        double scale,
        DecimateAlpha* tab) {
    int k = 0;
    for (int dx = 0; dx < dsize; dx++) {
        double fsx1 = dx * scale;
        double fsx2 = fsx1 + scale;
        double cell_width = std::min(scale, ssize - fsx1);
        int sx1 = std::ceil(fsx1);
        int sx2 = std::floor(fsx2);

        sx2 = std::min(sx2, ssize - 1);
        sx1 = std::min(sx1, sx2);

        if (sx1 - fsx1 > 1e-3) {
            tab[k].di = dx * cn;
            tab[k].si = (sx1 - 1) * cn;
            tab[k++].alpha = (float)((sx1 - fsx1) / cell_width);
        }

        for (int sx = sx1; sx < sx2; sx++) {
            tab[k].di = dx * cn;
            tab[k].si = sx * cn;
            tab[k++].alpha = float(1.0 / cell_width);
        }

        if (fsx2 - sx2 > 1e-3) {
            tab[k].di = dx * cn;
            tab[k].si = sx2 * cn;
            tab[k++].alpha = (float)(std::min(std::min(fsx2 - sx2, 1.), cell_width) / cell_width);
        }
    }

    return k;
}

void resize_area_decimal(
        const Mat& src,
        Mat& dst,
        const DecimateAlpha* xtab,
        int xtab_size,
        const DecimateAlpha* ytab,
        const int* tabofs) {
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    int cn = dst.channels();
    int src_w = src.width();
    int dst_w1 = dst.width() * cn;
    int dst_h = dst.height();
    float* buf = (float*)malloc(sizeof(float) * dst_w1 * 2);
    float* sum = buf + dst_w1;
    int j_start = tabofs[0];
    int j_end = tabofs[dst_h];
    int j = 0, k = 0, dx = 0;
    int prev_dy = ytab[j_start].di;

    for (dx = 0; dx < dst_w1; dx++) {
        sum[dx] = 0;
    }

    for (j = j_start; j < j_end; j++) {
        float beta = ytab[j].alpha;
        int dy = ytab[j].di;
        int sy = ytab[j].si;
        {
            const unsigned char* S = src_ptr + sy * src_w * cn;
            for (dx = 0; dx < dst_w1; dx++) {
                buf[dx] = 0;
            }

            if (cn == 1) {
                for (k = 0; k < xtab_size; k++) {
                    int dxn = xtab[k].di;
                    float alpha = xtab[k].alpha;
                    buf[dxn] += S[xtab[k].si] * alpha;
                }
            } else if (cn == 2) {
                for (k = 0; k < xtab_size; k++) {
                    int sxn = xtab[k].si;
                    int dxn = xtab[k].di;
                    float alpha = xtab[k].alpha;
                    float t0 = buf[dxn] + S[sxn] * alpha;
                    float t1 = buf[dxn+1] + S[sxn + 1] * alpha;
                    buf[dxn] = t0;
                    buf[dxn + 1] = t1;
                }
            } else if (cn == 3) {
                for (k = 0; k < xtab_size; k++) {
                    int sxn = xtab[k].si;
                    int dxn = xtab[k].di;
                    float alpha = xtab[k].alpha;
                    float t0 = buf[dxn] + S[sxn] * alpha;
                    float t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
                    float t2 = buf[dxn + 2] + S[sxn + 2] * alpha;
                    buf[dxn] = t0;
                    buf[dxn + 1] = t1;
                    buf[dxn + 2] = t2;
                }
            } else if (cn == 4) {
                for (k = 0; k < xtab_size; k++) {
                    int sxn = xtab[k].si;
                    int dxn = xtab[k].di;
                    float alpha = xtab[k].alpha;
                    float t0 = buf[dxn] + S[sxn] * alpha;
                    float t1 = buf[dxn + 1] + S[sxn + 1] * alpha;
                    buf[dxn] = t0;
                    buf[dxn + 1] = t1;
                    t0 = buf[dxn + 2] + S[sxn + 2] * alpha;
                    t1 = buf[dxn + 3] + S[sxn + 3] * alpha;
                    buf[dxn + 2] = t0;
                    buf[dxn + 3] = t1;
                }
            } else {
                for (k = 0; k < xtab_size; k++) {
                    int sxn = xtab[k].si;
                    int dxn = xtab[k].di;
                    float alpha = xtab[k].alpha;
                    for (int c = 0; c < cn; c++) {
                        buf[dxn + c] += S[sxn + c] * alpha;
                    }
                }
            }
        }

        if (dy != prev_dy) {
            unsigned char* D = dst_ptr + prev_dy * dst_w1;
            for (dx = 0; dx < dst_w1; dx++) {
                D[dx] = fcv_saturate_cast<float>(sum[dx]);
                sum[dx] = beta * buf[dx];
            }
            prev_dy = dy;
        } else {
            for (dx = 0; dx < dst_w1; dx++) {
                sum[dx] += beta * buf[dx];
            }
        }
    }

    unsigned char* D = dst_ptr + prev_dy * dst_w1;
    for (dx = 0; dx < dst_w1; dx++) {
        D[dx] = fcv_saturate_cast<float>(sum[dx]);
    }

    free(buf);
}

void resize_area_cn_common(
        Mat& src,
        Mat& dst,
        const int channel) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char* src_ptr = (unsigned char *)src.data();
    unsigned char* dst_ptr = (unsigned char *)dst.data();
    const int s_stride = src.stride();
    const int d_stride = dst.stride();

    double inv_scale_x = static_cast<double>(dst_w) / src_w;
    double inv_scale_y = static_cast<double>(dst_h) / src_h;

    double scale_x = 1. / inv_scale_x;
    double scale_y = 1. / inv_scale_y;

    int iscale_x = fcv_round(scale_x);
    int iscale_y = fcv_round(scale_y);

    bool is_area_fast = std::abs(scale_x - iscale_x) < FCV_EPSILON
            && std::abs(scale_y - iscale_y) < FCV_EPSILON;
    int cn = src.channels();

    // 缩小图像
    if (scale_x >= 1 && scale_y >= 1) {
        int k = 0, sx = 0, sy = 0, dx = 0, dy = 0;
        if (is_area_fast) {
            bool fast_mode = scale_x == 2 && scale_y == 2 && (cn == 1 || cn == 3 || cn == 4);
            int area = iscale_x * iscale_y;
            float scale = 1.f / (area);
            size_t srcstep = src_w * cn;
            int dwidth1 = (src_w / scale_x) * cn;
            int dst_w1 = dst_w * cn;
            int src_w1 = src_w * cn;

            int* ofs = (int*)malloc(area + dst_w * cn);
            int* xofs = ofs + area;

            for (sy = 0, k = 0; sy < iscale_y; sy++ ) {
                for (sx = 0; sx < iscale_x; sx++ ) {
                    ofs[k++] = (int)(sy * srcstep + sx * cn);
                }
            }

            for (dx = 0; dx < dst_w; dx++) {
                int j = dx * cn;
                sx = iscale_x * j;
                for (k = 0; k < cn; k++) {
                    xofs[j + k] = sx + k;
                }
            }

            for (dy = 0; dy < dst_h; dy++) {
                unsigned char *dst_row = dst_ptr + dy * dst_w * cn;
                int sy0 = dy * scale_y;
                int w = sy0 + scale_y <= src_h ? dwidth1 : 0;

                if ( sy0 >= src_h) {
                    for ( dx = 0; dx < dst_w1; dx++) {
                        dst_row[dx] = 0;
                    }
                    continue;
                }
                // 缩小图像为整倍数，且为2
                if (fast_mode) {
                    unsigned char* src_row = src_ptr + sy0 * src_w * cn;
                    unsigned char* nextS = src_row + src_w * cn;
                    dx = 0;
                    if (cn == 1) {
                        for (; dx < w; ++dx) {
                            int index = dx * 2;
                            dst_row[dx] = (src_row[index] + src_row[index + 1] +
                                    nextS[index] + nextS[index+1] + 2) >> 2;
                        }
                    } else if (cn == 3) {
                        for (; dx < w; dx += 3) {
                            int index = dx * 2;
                            dst_row[dx] = (src_row[index] + src_row[index+3] + nextS[index] + nextS[index+3] + 2) >> 2;
                            dst_row[dx+1] = (src_row[index+1] + src_row[index+4] +
                                    nextS[index+1] + nextS[index+4] + 2) >> 2;
                            dst_row[dx+2] = (src_row[index+2] + src_row[index+5] +
                                    nextS[index+2] + nextS[index+5] + 2) >> 2;
                        }
                    } else if (cn == 4) {
                        for (; dx < w; dx += 4) {
                            int index = dx * 2;
                            dst_row[dx] = (src_row[index] + src_row[index+4] + nextS[index] + nextS[index+4] + 2) >> 2;
                            dst_row[dx+1] = (src_row[index+1] + src_row[index+5] +
                                    nextS[index+1] + nextS[index+5] + 2) >> 2;
                            dst_row[dx+2] = (src_row[index+2] + src_row[index+6] +
                                    nextS[index+2] + nextS[index+6] + 2) >> 2;
                            dst_row[dx+3] = (src_row[index+3] + src_row[index+7] +
                                    nextS[index+2] + nextS[index+7] + 2) >> 2;
                        }
                    }
                    continue;
                }
                // 缩小图像为整倍数，不为2
                for (dx = 0; dx < w; dx++) {
                    unsigned char *src_row = src_ptr + sy0 * src_w * cn + xofs[dx];
                    int sum = 0;
                    k = 0;

                    for (; k <= area - 4; k += 4) {
                        sum += src_row[ofs[k]] + src_row[ofs[k + 1]] + src_row[ofs[k + 2]] + src_row[ofs[k + 3]];
                    }

                    for (; k < area; k++) {
                        sum += src_row[ofs[k]];
                    }

                    dst_row[dx] = fcv_saturate_cast<float>(sum * scale);
                }

                for(; dx < dst_w1; dx++) {
                    int sum = 0;
                    int count = 0;
                    int sx0 = xofs[dx];
                    if (sx0 >= src_w1) {
                        dst_row[dx] = 0;
                    }

                    for (int sy = 0; sy < scale_y; sy++) {
                        if (sy0 + sy >= src_h) break;
                        unsigned char *S = src_ptr + (sy0 + sy) * src_w * cn + sx0;

                        for (int sx = 0; sx < scale_x * cn; sx += cn) {
                            if (sx0 + sx >= src_w1) break;
                            sum += S[sx];
                            count++;
                        }
                    }

                    dst_row[dx] = fcv_saturate_cast<float>((float)sum / count);
                }
            }

            free(ofs);
            return;  
        }

        // 缩小图像不为整倍数
        DecimateAlpha* _xytab = (DecimateAlpha*)malloc(sizeof(DecimateAlpha) * (src_w + src_h) * 2);
        DecimateAlpha* xtab = _xytab;
        DecimateAlpha* ytab = xtab + src_w * 2;

        int xtab_size = compute_resize_area_tab(src_w, dst_w, cn, scale_x, xtab);
        int ytab_size = compute_resize_area_tab(src_h, dst_h, 1, scale_y, ytab);
        int* _tabofs = (int*)malloc(sizeof(int) * (dst_h + 1));
        int* tabofs = _tabofs;
        for (k = 0, dy = 0; k < ytab_size; k++) {
            if (k == 0 || ytab[k].di != ytab[k-1].di) {
                tabofs[dy++] = k;
            }
        }

        tabofs[dy] = ytab_size;
        resize_area_decimal(src, dst, xtab, xtab_size, ytab, tabofs);
        free(_xytab);
        free(_tabofs);
        return;
    }

    // 放大图像
    int* buf = nullptr;
    unsigned short *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = d_stride << 3;
    buf = (int*)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_area_buf(src_w, src_h, dst_w, dst_h, channel, inv_scale_x, inv_scale_y, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short* rows0 = nullptr;
    unsigned short* rows1 = nullptr;
    unsigned char * ptr_dst = nullptr;
    int dy = 0, dx= 0, sy0 = 0;
    for (; dy < dst_h; dy++) {
        rows0 = rows;
        rows1 = rows + d_stride;
        sy0 = *(yofs + dy);
        unsigned short *alphap = alpha;
        ptr_dst = dst_ptr + d_stride * dy;

        hresize_bn_one_row(src_ptr, xofs, sy0, s_stride,
                dst_w, channel, alphap, rows0, rows1);

        unsigned int b0 = *(beta++);
        unsigned int b1 = *(beta++);
        for (dx = 0; dx < d_stride; dx++) {
            *(ptr_dst++) = fcv_cast_u8((*(rows0++) * b0 + *(rows1++) * b1 + (1 << 17)) >> 18);
        }
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

int resize_area_common(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_area_cn_common(src, dst, 1);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_area_cn_common(src, dst, 3);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        resize_area_cn_common(src, dst, 4);
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
