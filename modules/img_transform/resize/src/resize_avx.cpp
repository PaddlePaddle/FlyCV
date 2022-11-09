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

#include "modules/img_transform/resize/include/resize_avx.h"

#if (defined HAVE_AVX) || (defined HAVE_AVX2)
#include <immintrin.h>
#endif

#include <math.h>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/utils.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

//static void cal_xcoord_and_coeff(
//        int dst_w,
//        double scale,
//        int src_w,
//        int channel,
//        int* xofs,
//        unsigned short* alpha) {
//    double fx = 0.f;
//    int sx = 0, dx = 0, tx = 0;
//    for (; dx < dst_w; dx++) {
//        fx = (dx + 0.5) * scale - 0.5;
//        sx = (int)floorf(static_cast<float>(fx));
//        fx -= sx;
//        tx = static_cast<int>(fcv_round(fx * 128));
//
//        if (sx < 0) {
//            sx = 0;
//            tx = 0;
//        }
//
//        if (sx >= src_w - 1) {
//            sx = src_w - 2;
//            tx = 128;
//        }
//
//        int idx = dx << 1;
//        xofs[dx] = sx * channel;
//
//        alpha[idx] = fcv_cast_u16(128 - tx);
//        alpha[idx + 1] = fcv_cast_u16(tx);
//    }
//}
//
//static void cal_ycoord_and_coeff(
//        int dst_h,
//        double scale,
//        int src_h,
//        int* yofs,
//        unsigned short* beta) {
//    double fy = 0.f;
//    int dy = 0, sy = 0, ty = 0;
//    for (; dy < dst_h; dy++) {
//        fy = (double)((dy + 0.5) * scale - 0.5);
//        sy = (int)floorf(static_cast<float>(fy));
//        fy -= sy;
//        ty = static_cast<int>(fcv_round(fy * 128));
//
//        if (sy < 0) {
//            sy = 0;
//            ty = 0;
//        }
//
//        if (sy >= src_h - 1) {
//            sy = src_h - 2;
//            ty = 128;
//        }
//
//        int idy = dy << 1;
//        yofs[dy] = sy;
//
//        beta[idy] = fcv_cast_u16(128 - ty);
//        beta[idy + 1] = fcv_cast_u16(ty);
//    }
//}
//
//static void get_resize_bilinear_buf(
//        int src_w,
//        int src_h,
//        int w,
//        int h,
//        int c,
//        int** buf) {
//    double scale_x = (double)src_w / w;
//    double scale_y = (double)src_h / h;
//    int dou_w = w + w;
//    int* xofs = *buf;
//    int* yofs = xofs + w;
//    unsigned short* alpha = (unsigned short*)(yofs + h);
//    unsigned short* beta = (unsigned short*)(alpha + dou_w);
//
//    cal_xcoord_and_coeff(w, scale_x, src_w, c, xofs, alpha);
//    cal_ycoord_and_coeff(h, scale_y, src_h, yofs, beta);
//}
//
//static void hresize_bn_one_row_avx(
//        const unsigned char* src_ptr,
//        int* xofs,
//        int sy,
//        int stride,
//        int w,
//        int c,
//        unsigned short* alphap,
//        unsigned short* rows0,
//        unsigned short* rows1) {
//    const int sy_off = sy * stride;
//    const unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
//    const unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + stride);
//
//    unsigned short* rows0p = rows0;
//    unsigned short* rows1p = rows1;
//
//    for (int dx = 0; dx < w; dx++) {
//        int cx = xofs[dx];
//        const unsigned char* S0p = src0 + cx;
//        const unsigned char* S1p = src1 + cx;
//
//        unsigned short a0 = *(alphap++);
//        unsigned short a1 = *(alphap++);
//
//        if (c == 1) {
//            rows0p[0] = (S0p[0] * a0 + S0p[c] * a1) >> 4;
//            rows1p[0] = (S1p[0] * a0 + S1p[c] * a1) >> 4;
//        } else if (c == 2) {
//            rows0p[0] = (S0p[0] * a0 + S0p[c] * a1) >> 4;
//            rows1p[0] = (S1p[0] * a0 + S1p[c] * a1) >> 4;
//            rows0p[1] = (S0p[1] * a0 + S0p[c + 1] * a1) >> 4;
//            rows1p[1] = (S1p[1] * a0 + S1p[c + 1] * a1) >> 4;
//        } else if (c == 3) {
//            rows0p[0] = (S0p[0] * a0 + S0p[c] * a1) >> 4;
//            rows1p[0] = (S1p[0] * a0 + S1p[c] * a1) >> 4;
//            rows0p[1] = (S0p[1] * a0 + S0p[c + 1] * a1) >> 4;
//            rows1p[1] = (S1p[1] * a0 + S1p[c + 1] * a1) >> 4;
//            rows0p[2] = (S0p[2] * a0 + S0p[c + 2] * a1) >> 4;
//            rows1p[2] = (S1p[2] * a0 + S1p[c + 2] * a1) >> 4;
//        } else if (c == 4) {
//            rows0p[0] = (S0p[0] * a0 + S0p[c] * a1) >> 4;
//            rows1p[0] = (S1p[0] * a0 + S1p[c] * a1) >> 4;
//            rows0p[1] = (S0p[1] * a0 + S0p[c + 1] * a1) >> 4;
//            rows1p[1] = (S1p[1] * a0 + S1p[c + 1] * a1) >> 4;
//            rows0p[2] = (S0p[2] * a0 + S0p[c + 2] * a1) >> 4;
//            rows1p[2] = (S1p[2] * a0 + S1p[c + 2] * a1) >> 4;
//            rows0p[3] = (S0p[3] * a0 + S0p[c + 3] * a1) >> 4;
//            rows1p[3] = (S1p[3] * a0 + S1p[c + 3] * a1) >> 4;
//        }
//
//        rows0p += c;
//        rows1p += c;
//    }
//}

//void resize_bilinear_cn_avx(Mat& src, Mat& dst, const int channel) {
    //const int src_w = src.width();
    //const int src_h = src.height();
    //const int dst_w = dst.width();
    //const int dst_h = dst.height();
    //unsigned char *src_ptr = (unsigned char *)src.data();
    //unsigned char *dst_ptr = (unsigned char *)dst.data();
    //const int s_stride = src.stride();
    //const int d_stride = dst.stride();

    //int buf_size = (dst_w + dst_h) << 3;
    //int rows_size = d_stride << 3;
    //int* buf = (int*)malloc(buf_size);
    //unsigned short* rows = (unsigned short*)malloc(rows_size);
    //get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, channel, &buf);

    //int* xofs = buf;
    //int* yofs = buf + dst_w;
    //unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    //unsigned short* beta = (unsigned short*)(alpha + dst_w + dst_w);

    //unsigned short *rows0 = nullptr;
    //unsigned short *rows1 = nullptr;
    //unsigned char *ptr_dst = nullptr;
    //int sy0 = 0;

    //for (int dy = 0; dy < dst_h; dy++) {
    //    rows0 = rows;
    //    rows1 = rows + d_stride;
    //    sy0 = *(yofs + dy);
    //    unsigned short *alphap = alpha;
    //    ptr_dst = dst_ptr + d_stride * dy;

    //    hresize_bn_one_row_avx(src_ptr, xofs, sy0, s_stride,
    //            dst_w, channel, alphap, rows0, rows1);

    //    unsigned short b0 = *(beta++);
    //    unsigned short b1 = *(beta++);

    //    int loop_cnt = d_stride / 8;
    //    int remain = d_stride - loop_cnt * 8;

    //    __m256i op_b0 = _mm256_set1_epi32(b0);
    //    __m256i op_b1 = _mm256_set1_epi32(b1);
    //    __m256i pre = _mm256_set1_epi32(1 << 9);

    //    for (int i = 0; i < loop_cnt; ++i) {
    //        //__m256i rows0_data = _mm256_cvtepu16_epi32(_mm_lddqu_si128((__m128i*)rows0));
    //        //__m256i rows1_data = _mm256_cvtepu16_epi32(_mm_lddqu_si128((__m128i*)rows1));

    //        //__m256i rows0_mul_data = _mm256_mullo_epi32(rows0_data, op_b0);
    //        //__m256i rows1_mul_data = _mm256_mullo_epi32(rows1_data, op_b1);

    //        //__m256i add_res_temp = _mm256_add_epi32(rows0_mul_data, rows1_mul_data);

    //        //rows0 += 8;
    //        //rows1 += 8;
    //        //ptr_dst += 8;
    //    }

    //    if (remain) {
    //        for (int dx = loop_cnt * 8; dx < d_stride; dx++) {
    //            *(ptr_dst++) = fcv_cast_u8((*(rows0++) * b0 + *(rows1++) * b1 + (1 << 9)) >> 10);
    //        }
    //    }
    //}

    //if (buf != nullptr) {
    //    free(buf);
    //    buf = nullptr;
    //}

    //if (rows != nullptr) {
    //    free(rows);
    //    rows = nullptr;
    //}
//}

//int resize_bilinear_avx(Mat& src, Mat& dst) {
//    TypeInfo type_info;
//    int status = get_type_info(src.type(), type_info);
//
//    if (status != 0) {
//        LOG_ERR("The src type is not supported!");
//        return -1;
//    }
//
//    if (src.type() == FCVImageType::NV12 || src.type() == FCVImageType::NV21) {
//        //resize_bilinear_yuv_common(src, dst);
//    } else {
//        resize_bilinear_cn_avx(src, dst, type_info.channels);
//    }
//
//    return 0;
//}

G_FCV_NAMESPACE1_END()
