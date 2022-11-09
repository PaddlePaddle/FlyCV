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

#include "modules/img_transform/resize/include/resize_sse.h"

#include <math.h>

#ifdef HAVE_SSE
#include <xmmintrin.h>
#include <mmintrin.h>
#endif

#ifdef HAVE_SSE2
#include <emmintrin.h>
#endif

#ifdef HAVE_SSE3
#include <pmmintrin.h>
#endif

#ifdef HAVE_SSE4_1
#include <smmintrin.h>
#endif

#ifdef HAVE_SSE4_2
#include <nmmintrin.h>
#endif

#include "modules/img_transform/resize/include/resize_common.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/utils.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

static void cal_xcoord_and_coeff(
        int dst_w,
        float scale,
        int src_w,
        int channel,
        int* xofs,
        unsigned short* alpha) {
    float fx = 0.f;
    int sx = 0, tx = 0;

    for (int dx = 0; dx < dst_w; dx++) {
        fx = (dx + 0.5) * scale - 0.5;
        sx = (int)floorf(fx);
        fx -= sx;
        tx = static_cast<int>(fcv_round(fx * 128));

        if (sx < 0) {
            sx = 0;
            tx = 0;
        }

        if (sx >= src_w - 1) {
            sx = src_w - 2;
            tx = 128;
        }

        int idx = dx << 1;
        xofs[dx] = sx * channel;

        alpha[idx] = fcv_cast_u16(128 - tx);
        alpha[idx + 1] = fcv_cast_u16(tx);
    }
}

static void cal_ycoord_and_coeff(
        int dst_h,
        float scale,
        int src_h,
        int* yofs,
        unsigned short* beta) {
    float fy = 0.f;
    int dy = 0, sy = 0, ty = 0;
    for (; dy < dst_h; dy++) {
        fy = (dy + 0.5) * scale - 0.5;
        sy = (int)floorf(fy);
        fy -= sy;
        ty = static_cast<int>(fcv_round(fy * 128));

        if (sy < 0) {
            sy = 0;
            ty = 0;
        }

        if (sy >= src_h - 1) {
            sy = src_h - 2;
            ty = 128;
        }

        int idy = dy << 1;
        yofs[dy] = sy;

        beta[idy] = fcv_cast_u16(128 - ty);
        beta[idy + 1] = fcv_cast_u16(ty);
    }
}
                               
static void get_resize_bilinear_buf_sse(
        int src_w,
        int src_h,
        int w,
        int h,
        int c,
        int** buf) {
    float scale_x = (float)src_w / w;
    float scale_y = (float)src_h / h;
    int dou_w = w + w;
    int* xofs = *buf;
    int* yofs = xofs + w;
    unsigned short* alpha = (unsigned short*)(yofs + h);
    unsigned short* beta = (unsigned short*)(alpha + dou_w);

    cal_xcoord_and_coeff(w, scale_x, src_w, c, xofs, alpha);
    cal_ycoord_and_coeff(h, scale_y, src_h, yofs, beta);
}

//void resize_bilinear_cn_sse(Mat& src, Mat& dst, const int channel) {
//    const int src_w = src.width();
//    const int src_h = src.height();
//    const int dst_w = dst.width();
//    const int dst_h = dst.height();
//    unsigned char *src_ptr = (unsigned char *)src.data();
//    unsigned char *dst_ptr = (unsigned char *)dst.data();
//    const int s_stride = src.stride();
//    const int d_stride = dst.stride();
//
//    int buf_size = (dst_w + dst_h) << 3;
//    int rows_size = d_stride << 3;
//    int* buf = (int*)malloc(buf_size);
//    get_resize_bilinear_buf_sse(src_w, src_h, dst_w, dst_h, channel, &buf);
//
//    int* xofs = buf;
//    int* yofs = buf + dst_w;
//    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
//    unsigned short* beta = (unsigned short*)(alpha + dst_w + dst_w);
//
//    int sx0 = 0;
//    int sy0 = 0;
//    int a_idx0 = 0;
//    int a_idx1 = 0;
//    int b_idx0 = 0;
//    int b_idx1 = 0;
//
//    int loop_cnt = dst_w / 16;
//
//    __m128i align0 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127);
//    __m128i align1 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0);
//    __m128i align2 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0);
//    __m128i align3 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0);
//    __m128i align4 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0);
//    __m128i align5 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0);
//    __m128i align6 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0);
//    __m128i align7 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align8 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align9 = _mm_setr_epi8(0, 0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align10 = _mm_setr_epi8(0, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align11 = _mm_setr_epi8(0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align12 = _mm_setr_epi8(0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align13 = _mm_setr_epi8(0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align14 = _mm_setr_epi8(0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//    __m128i align15 = _mm_setr_epi8(127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
//
//    __declspec(align(16)) int a_idx[32] = {0};
//
//    for (int dy = 0; dy < dst_h; ++dy) {
//        sy0 = yofs[dy];
//        b_idx0 = dy << 1;
//        b_idx1 = b_idx0 + 1;
//
//        unsigned char* src_ptr_start = src_ptr + sy0 * s_stride;
//        __m128i b = _mm_set_epi32(beta[b_idx0], beta[b_idx0], beta[b_idx1], beta[b_idx1]);
//
//        for (int count = 0; count < loop_cnt; ++count) {
//            int x_pos = count << 4;
//            int idx_pos = count << 5;
//
//            a_idx[0] = idx_pos;
//            a_idx[1] = idx_pos + 1;
//            a_idx[2] = idx_pos + 2;
//            a_idx[3] = idx_pos + 3;
//            a_idx[4] = idx_pos + 4;
//            a_idx[5] = idx_pos + 5;
//            a_idx[6] = idx_pos + 6;
//            a_idx[7] = idx_pos + 7;
//            a_idx[8] = idx_pos + 8;
//            a_idx[9] = idx_pos + 9;
//            a_idx[10] = idx_pos + 10;
//            a_idx[11] = idx_pos + 11;
//            a_idx[12] = idx_pos + 12;
//            a_idx[13] = idx_pos + 13;
//            a_idx[14] = idx_pos + 14;
//            a_idx[15] = idx_pos + 15;
//            a_idx[16] = idx_pos + 16;
//            a_idx[17] = idx_pos + 17;
//            a_idx[18] = idx_pos + 18;
//            a_idx[19] = idx_pos + 19;
//            a_idx[20] = idx_pos + 20;
//            a_idx[21] = idx_pos + 21;
//            a_idx[22] = idx_pos + 22;
//            a_idx[23] = idx_pos + 23;
//            a_idx[24] = idx_pos + 24;
//            a_idx[25] = idx_pos + 25;
//            a_idx[26] = idx_pos + 26;
//            a_idx[27] = idx_pos + 27;
//            a_idx[28] = idx_pos + 28;
//            a_idx[29] = idx_pos + 29;
//            a_idx[30] = idx_pos + 30;
//            a_idx[31] = idx_pos + 31;
//
//            unsigned char* src_data0 = src_ptr_start + xofs[x_pos];
//            unsigned char* src_data1 = src_ptr_start + xofs[x_pos + 1];
//            unsigned char* src_data2 = src_ptr_start + xofs[x_pos + 2];
//            unsigned char* src_data3 = src_ptr_start + xofs[x_pos + 3];
//            unsigned char* src_data4 = src_ptr_start + xofs[x_pos + 4];
//            unsigned char* src_data5 = src_ptr_start + xofs[x_pos + 5];
//            unsigned char* src_data6 = src_ptr_start + xofs[x_pos + 6];
//            unsigned char* src_data7 = src_ptr_start + xofs[x_pos + 7];
//            unsigned char* src_data8 = src_ptr_start + xofs[x_pos + 8];
//            unsigned char* src_data9 = src_ptr_start + xofs[x_pos + 9];
//            unsigned char* src_data10 = src_ptr_start + xofs[x_pos + 10];
//            unsigned char* src_data11 = src_ptr_start + xofs[x_pos + 11];
//            unsigned char* src_data12 = src_ptr_start + xofs[x_pos + 12];
//            unsigned char* src_data13 = src_ptr_start + xofs[x_pos + 13];
//            unsigned char* src_data14 = src_ptr_start + xofs[x_pos + 14];
//            unsigned char* src_data15 = src_ptr_start + xofs[x_pos + 15];
//
//            __m128i a0 = _mm_set_epi32(a_idx[0], a_idx[1], a_idx[0], a_idx[1]);
//            __m128i a1 = _mm_set_epi32(a_idx[2], a_idx[3], a_idx[2], a_idx[3]);
//            __m128i a2 = _mm_set_epi32(a_idx[4], a_idx[5], a_idx[4], a_idx[5]);
//            __m128i a3 = _mm_set_epi32(a_idx[6], a_idx[7], a_idx[6], a_idx[7]);
//            __m128i a4 = _mm_set_epi32(a_idx[8], a_idx[9], a_idx[8], a_idx[9]);
//            __m128i a5 = _mm_set_epi32(a_idx[10], a_idx[11], a_idx[10], a_idx[11]);
//            __m128i a6 = _mm_set_epi32(a_idx[12], a_idx[13], a_idx[12], a_idx[13]);
//            __m128i a7 = _mm_set_epi32(a_idx[14], a_idx[15], a_idx[14], a_idx[15]);
//            __m128i a8 = _mm_set_epi32(a_idx[16], a_idx[17], a_idx[16], a_idx[17]);
//            __m128i a9 = _mm_set_epi32(a_idx[18], a_idx[19], a_idx[18], a_idx[19]);
//            __m128i a10 = _mm_set_epi32(a_idx[20], a_idx[21], a_idx[20], a_idx[21]);
//            __m128i a11 = _mm_set_epi32(a_idx[22], a_idx[23], a_idx[22], a_idx[23]);
//            __m128i a12 = _mm_set_epi32(a_idx[24], a_idx[25], a_idx[24], a_idx[25]);
//            __m128i a13 = _mm_set_epi32(a_idx[26], a_idx[27], a_idx[26], a_idx[27]);
//            __m128i a14 = _mm_set_epi32(a_idx[28], a_idx[29], a_idx[28], a_idx[29]);
//            __m128i a15 = _mm_set_epi32(a_idx[30], a_idx[31], a_idx[30], a_idx[31]);
//
//            __m128i uv0 = _mm_mullo_epi32(a0, b);
//            __m128i res0 = calculate_result(src_data0, s_stride, &uv0, 3, &align0, 0);
//            __m128i res1 = calculate_result(src_data0 + 1, s_stride, &uv0, 3, &align1, 8);
//            __m128i res2 = calculate_result(src_data0 + 2, s_stride, &uv0, 3, &align2, 16);
//
//            __m128i uv1 = _mm_mullo_epi32(a1, b);
//            __m128i res3 = calculate_result(src_data1, s_stride, &uv1, 3, &align3, 24);
//            __m128i res4 = calculate_result(src_data1 + 1, s_stride, &uv1, 3, &align4, 32);
//            __m128i res5 = calculate_result(src_data1 + 2, s_stride, &uv1, 3, &align5, 40);
//
//            __m128i uv2 = _mm_mullo_epi32(a2, b);
//            __m128i res6 = calculate_result(src_data2, s_stride, &uv2, 3, &align6, 48);
//            __m128i res7 = calculate_result(src_data2 + 1, s_stride, &uv2, 3, &align7, 56);
//            __m128i res8 = calculate_result(src_data2 + 2, s_stride, &uv2, 3, &align8, 64);
//
//            __m128i uv3 = _mm_mullo_epi32(a3, b);
//            __m128i res9 = calculate_result(src_data3, s_stride, &uv3, 3, &align9, 72);
//            __m128i res10 = calculate_result(src_data3 + 1, s_stride, &uv3, 3, &align10, 80);
//            __m128i res11 = calculate_result(src_data3 + 2, s_stride, &uv3, 3, &align11, 88);
//
//            __m128i uv4 = _mm_mullo_epi32(a4, b);
//            __m128i res12 = calculate_result(src_data4, s_stride, &uv4, 3, &align12, 96);
//            __m128i res13 = calculate_result(src_data4 + 1, s_stride, &uv4, 3, &align13, 104);
//            __m128i res14 = calculate_result(src_data4 + 2, s_stride, &uv4, 3, &align14, 112);
//
//            __m128i uv5 = _mm_mullo_epi32(a5, b);
//            __m128i res15 = calculate_result(src_data5, s_stride, &uv5, 3, &align15, 120);
//
//            __m128i merge1 = _mm_or_si128(res1, res0);
//            __m128i merge2 = _mm_or_si128(res2, merge1);
//            __m128i merge3 = _mm_or_si128(res3, merge2);
//            __m128i merge4 = _mm_or_si128(res4, merge3);
//            __m128i merge5 = _mm_or_si128(res5, merge4);
//            __m128i merge6 = _mm_or_si128(res6, merge5);
//            __m128i merge7 = _mm_or_si128(res7, merge6);
//            __m128i merge8 = _mm_or_si128(res8, merge7);
//            __m128i merge9 = _mm_or_si128(res9, merge8);
//            __m128i merge10 = _mm_or_si128(res10, merge9);
//            __m128i merge11 = _mm_or_si128(res11, merge10);
//            __m128i merge12 = _mm_or_si128(res12, merge11);
//            __m128i merge13 = _mm_or_si128(res13, merge12);
//            __m128i merge14 = _mm_or_si128(res14, merge13);
//            __m128i merge15 = _mm_or_si128(res15, merge14);
//
//            _mm_storeu_si128((__m128i*)dst_ptr, merge15);
//            dst_ptr += 16;
//
//            __m128i res16 = calculate_result(src_data5 + 1, s_stride, &uv5, 3, &align0, 0);
//            __m128i res17 = calculate_result(src_data5 + 2, s_stride, &uv5, 3, &align1, 8);
//            __m128i merge17 = _mm_or_si128(res17, res16);
//
//            __m128i uv6 = _mm_mullo_epi32(a6, b);
//            __m128i res18 = calculate_result(src_data6, s_stride, &uv6, 3, &align2, 16);
//            __m128i merge18 = _mm_or_si128(res18, merge17);
//            __m128i res19 = calculate_result(src_data6 + 1, s_stride, &uv6, 3, &align3, 24);
//            __m128i merge19 = _mm_or_si128(res19, merge18);
//            __m128i res20 = calculate_result(src_data6 + 2, s_stride, &uv6, 3, &align4, 32);
//            __m128i merge20 = _mm_or_si128(res20, merge19);
//
//            __m128i uv7 = _mm_mullo_epi32(a7, b);
//            __m128i res21 = calculate_result(src_data7, s_stride, &uv7, 3, &align5, 40);
//            __m128i merge21 = _mm_or_si128(res21, merge20);
//            __m128i res22 = calculate_result(src_data7 + 1, s_stride, &uv7, 3, &align6, 48);
//            __m128i merge22 = _mm_or_si128(res22, merge21);
//            __m128i res23 = calculate_result(src_data7 + 2, s_stride, &uv7, 3, &align7, 56);
//            __m128i merge23 = _mm_or_si128(res23, merge22);
//
//            __m128i uv8 = _mm_mullo_epi32(a8, b);
//            __m128i res24 = calculate_result(src_data8, s_stride, &uv8, 3, &align8, 64);
//            __m128i merge24 = _mm_or_si128(res24, merge23);
//            __m128i res25 = calculate_result(src_data8 + 1, s_stride, &uv8, 3, &align9, 72);
//            __m128i merge25 = _mm_or_si128(res25, merge24);
//            __m128i res26 = calculate_result(src_data8 + 2, s_stride, &uv8, 3, &align10, 80);
//            __m128i merge26 = _mm_or_si128(res26, merge25);
//
//            __m128i uv9 = _mm_mullo_epi32(a9, b);
//            __m128i res27 = calculate_result(src_data9, s_stride, &uv9, 3, &align11, 88);
//            __m128i merge27 = _mm_or_si128(res24, merge23);
//            __m128i res28 = calculate_result(src_data9 + 1, s_stride, &uv9, 3, &align12, 96);
//            __m128i merge28 = _mm_or_si128(res25, merge24);
//            __m128i res29 = calculate_result(src_data9 + 2, s_stride, &uv9, 3, &align13, 104);
//            __m128i merge29 = _mm_or_si128(res26, merge25);
//
//            __m128i uv10 = _mm_mullo_epi32(a10, b);
//            __m128i res30 = calculate_result(src_data10, s_stride, &uv10, 3, &align14, 112);
//            __m128i merge30 = _mm_or_si128(res30, merge29);
//            __m128i res31 = calculate_result(src_data10 + 1, s_stride, &uv10, 3, &align15, 120);
//            __m128i merge31 = _mm_or_si128(res25, merge24);
//
//            _mm_storeu_si128((__m128i*)dst_ptr, merge31);
//            dst_ptr += 16;
//
//            __m128i res32 = calculate_result(src_data10 + 2, s_stride, &uv9, 3, &align0, 0);
//            __m128i merge32 = _mm_or_si128(res26, merge25);
//
//            __m128i uv11 = _mm_mullo_epi32(a11, b);
//            __m128i res33 = calculate_result(src_data11, s_stride, &uv11, 3, &align1, 8);
//            __m128i res34 = calculate_result(src_data11 + 1, s_stride, &uv11, 3, &align2, 16);
//            __m128i merge34 = _mm_or_si128(res34, res33);
//            __m128i res35 = calculate_result(src_data11 + 2, s_stride, &uv11, 3, &align3, 24);
//            __m128i merge35 = _mm_or_si128(res35, merge34);
//
//            __m128i uv12 = _mm_mullo_epi32(a12, b);
//            __m128i res36 = calculate_result(src_data12, s_stride, &uv12, 3, &align4, 32);
//            __m128i merge36 = _mm_or_si128(res36, merge35);
//            __m128i res37 = calculate_result(src_data12 + 1, s_stride, &uv12, 3, &align5, 40);
//            __m128i merge37 = _mm_or_si128(res37, merge36);
//            __m128i res38 = calculate_result(src_data12 + 2, s_stride, &uv12, 3, &align6, 48);
//            __m128i merge38 = _mm_or_si128(res38, merge37);
//
//            __m128i uv13 = _mm_mullo_epi32(a13, b);
//            __m128i res39 = calculate_result(src_data13, s_stride, &uv13, 3, &align7, 56);
//            __m128i merge39 = _mm_or_si128(res39, merge38);
//            __m128i res40 = calculate_result(src_data13 + 1, s_stride, &uv13, 3, &align8, 64);
//            __m128i merge40 = _mm_or_si128(res40, merge39);
//            __m128i res41 = calculate_result(src_data13 + 2, s_stride, &uv13, 3, &align9, 72);
//            __m128i merge41 = _mm_or_si128(res41, merge40);
//
//            __m128i uv14 = _mm_mullo_epi32(a14, b);
//            __m128i res42 = calculate_result(src_data14, s_stride, &uv14, 3, &align10, 80);
//            __m128i merge42 = _mm_or_si128(res42, merge41);
//            __m128i res43 = calculate_result(src_data14 + 1, s_stride, &uv14, 3, &align11, 88);
//            __m128i merge43 = _mm_or_si128(res43, merge42);
//            __m128i res44 = calculate_result(src_data14 + 2, s_stride, &uv14, 3, &align12, 96);
//            __m128i merge44 = _mm_or_si128(res44, merge43);
//
//            __m128i uv15 = _mm_mullo_epi32(a15, b);
//            __m128i res45 = calculate_result(src_data15, s_stride, &uv15, 3, &align13, 104);
//            __m128i merge45 = _mm_or_si128(res45, merge44);
//            __m128i res46 = calculate_result(src_data15 + 1, s_stride, &uv15, 3, &align5, 112);
//            __m128i merge46 = _mm_or_si128(res46, merge45);
//            __m128i res47 = calculate_result(src_data15 + 2, s_stride, &uv15, 3, &align6, 120);
//            __m128i merge47 = _mm_or_si128(res47, merge46);
//
//            _mm_storeu_si128((__m128i*)dst_ptr, merge47);
//            dst_ptr += 16;
//        }
//
//        for (int dx = 16 * loop_cnt; dx < dst_w; ++dx) {
//            sx0 = xofs[dx];
//            a_idx0 = dx << 1;
//            a_idx1 = a_idx0 + 1;
//
//            unsigned char* src_data = src_ptr + sy0 * s_stride + sx0;
//            int u = alpha[a_idx0] * beta[b_idx0];
//            int v = alpha[a_idx1] * beta[b_idx0];
//            int m = alpha[a_idx0] * beta[b_idx1];
//            int n = alpha[a_idx1] * beta[b_idx1];
//
//            for (int c = 0; c < channel; ++c) {
//                unsigned char* q11 = src_data + c;
//                unsigned char* q21 = q11 + channel;
//                unsigned char* q12 = src_data + s_stride + c;
//                unsigned char* q22 = q12 + channel;
//
//                *(dst_ptr++) = (*q11 * u + (*q21) * v + (*q12) * m + (*q22) * n) >> 14;
//            }
//        }
//    }
//
//    if (buf != nullptr) {
//        free(buf);
//        buf = nullptr;
//    }
//}

void resize_bilinear_cn_sse(Mat& src, Mat& dst, const int channel) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int s_stride = src.stride();

    int buf_size = (dst_w + dst_h) << 3;
    int* buf = (int*)malloc(buf_size);
    get_resize_bilinear_buf_sse(src_w, src_h, dst_w, dst_h, channel, &buf);

    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta = (unsigned short*)(alpha + dst_w + dst_w);

    int sx0 = 0;
    int sy0 = 0;
    int a_idx0 = 0;
    int a_idx1 = 0;
    int b_idx0 = 0;
    int b_idx1 = 0;

    for (int dy = 0; dy < dst_h; ++dy) {
        sy0 = yofs[dy];
        b_idx0 = dy << 1;
        b_idx1 = b_idx0 + 1;

        for (int dx = 0; dx < dst_w; ++dx) {
            sx0 = xofs[dx];
            a_idx0 = dx << 1;
            a_idx1 = a_idx0 + 1;

            unsigned char* src_data = src_ptr + sy0 * s_stride + sx0;

            int u = alpha[a_idx0] * beta[b_idx0];
            int v = alpha[a_idx1] * beta[b_idx0];
            int m = alpha[a_idx0] * beta[b_idx1];
            int n = alpha[a_idx1] * beta[b_idx1];

            for (int c = 0; c < channel; ++c) {
                unsigned char* q11 = src_data + c;
                unsigned char* q21 = q11 + channel;
                unsigned char* q12 = src_data + s_stride + c;
                unsigned char* q22 = q12 + channel;

                *(dst_ptr++) = (*q11 * u + (*q21) * v + (*q12) * m + (*q22) * n) >> 14;
            }
        }
    }
}

int resize_bilinear_sse(Mat& src, Mat& dst) {
    TypeInfo type_info;
    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    if (src.type() == FCVImageType::NV12 || src.type() == FCVImageType::NV21) {
        //resize_bilinear_yuv_common(src, dst);
    } else {
        resize_bilinear_cn_sse(src, dst, type_info.channels);
    }

    return 0;
}

int resize_cubic_sse(Mat& src, Mat& dst) {
    return resize_cubic_common(src, dst);
}

G_FCV_NAMESPACE1_END()
