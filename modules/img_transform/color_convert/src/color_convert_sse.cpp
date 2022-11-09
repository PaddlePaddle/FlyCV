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

#include "modules/img_transform/color_convert/include/color_convert_sse.h"
#include "modules/core/base/include/common_sse.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void convert_yuv420_to_bgr_sse(
        const Mat& src,
        Mat& src_u,
        Mat& src_v,
        Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    int dst_w = dst.width();
    const int width_align8 = src_w & (~15);

    const int doub_src_stride = src_stride << 1;
    const int doub_dst_stride = dst_stride << 1;

    const unsigned char *u = (const unsigned char *)src_u.data();
    const unsigned char *v = (const unsigned char *)src_v.data();

    const __m128i vc32 = _mm_set1_epi16(32);
    const __m128i vc16 = _mm_set1_epi16(16);
    const __m128i vc149 = _mm_set1_epi16(149 / 2);
    const __m128i vc102 = _mm_set1_epi16(102);
    const __m128i vc14216 = _mm_set1_epi16(14216);
    const __m128i vc52 = _mm_set1_epi16(52);
    const __m128i vc25 = _mm_set1_epi16(25);
    const __m128i vc8696 = _mm_set1_epi16(8696);
    const __m128i vc129 = _mm_set1_epi16(129);
    const __m128i vc17672 = _mm_set1_epi16(17672);
    const __m128i zero = _mm_setzero_si128();
    const __m128i vc255 = _mm_set1_epi16(255);
    __m128i uzip_index = _mm_setr_epi8(0, 1, 4, 5, 8, 9, 12, 13, 2, 3, 6, 7, 10, 11, 14, 15);
    __m128i zip_index = _mm_setr_epi8(0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15);

    for (int i = 0; i < src_h; i += 2) {
        const unsigned char *y0_ptr = src_ptr;
        const unsigned char *y1_ptr = src_ptr + src_w;

        unsigned char *dst0_ptr = dst_ptr ;
        unsigned char *dst1_ptr = dst_ptr + dst_stride;
        const unsigned char *u0 = u;
        const unsigned char *v0 = v;
        
        for (int j = 0; j < width_align8; j += 16) {
            __m128i y00_u8 = _mm_max_epi16(_mm_unpacklo_epi8(
                    _mm_loadl_epi64((__m128i const*)y0_ptr), zero), vc16);
            __m128i y01_u8 = _mm_max_epi16(_mm_unpacklo_epi8(
                    _mm_loadl_epi64((__m128i const*)(y0_ptr + 8)), zero), vc16);
            __m128i y10_u8 = _mm_max_epi16(_mm_unpacklo_epi8(
                    _mm_loadl_epi64((__m128i const*)y1_ptr), zero), vc16);
            __m128i y11_u8 = _mm_max_epi16(_mm_unpacklo_epi8(
                    _mm_loadl_epi64((__m128i const*)(y1_ptr + 8)), zero),vc16);

            __m128i y0_even = _mm_unpacklo_epi64(_mm_shuffle_epi8(y00_u8, uzip_index),
                    _mm_shuffle_epi8(y01_u8, uzip_index)); // separate the odd and even
            __m128i y0_odd = _mm_unpackhi_epi64(_mm_shuffle_epi8(y00_u8, uzip_index),
                    _mm_shuffle_epi8(y01_u8, uzip_index)); 
            __m128i y1_even = _mm_unpacklo_epi64(_mm_shuffle_epi8(y10_u8, uzip_index),
                    _mm_shuffle_epi8(y11_u8, uzip_index)); // separate the odd and even
            __m128i y1_odd = _mm_unpackhi_epi64(_mm_shuffle_epi8(y10_u8, uzip_index),
                    _mm_shuffle_epi8(y11_u8, uzip_index)); 

            __m128i u_u8 = _mm_unpacklo_epi8(_mm_loadl_epi64((const __m128i*)u0), zero);
            __m128i v_u8 = _mm_unpacklo_epi8( _mm_loadl_epi64((const __m128i*)v0), zero);

            __m128i v_ub = _mm_mullo_epi16(u_u8, vc129);
            __m128i v_vr = _mm_mullo_epi16(v_u8, vc102);

            __m128i v_ug = _mm_mullo_epi16(u_u8, vc25);
            __m128i v_vg = _mm_mullo_epi16(v_u8, vc52);
            __m128i v_uvg = _mm_add_epi16(v_ug, v_vg);

            __m128i ub = _mm_sub_epi16(v_ub, vc17672);
            __m128i vr = _mm_sub_epi16(v_vr, vc14216);
            __m128i uvg = _mm_sub_epi16(vc8696, v_uvg);

            __m128i y00 = _mm_mullo_epi16(y0_even, vc149);
            __m128i y01 = _mm_mullo_epi16(y0_odd, vc149);

            // y0
            __m128i y10 = _mm_mullo_epi16(y1_even, vc149);
            __m128i y11 = _mm_mullo_epi16(y1_odd, vc149);

            __m128i v_b0 = _mm_add_epi16(_mm_add_epi16(y00, ub), vc32);
            __m128i v_g0 = _mm_add_epi16(_mm_add_epi16(y00, uvg), vc32);
            __m128i v_r0 = _mm_add_epi16(_mm_add_epi16(y00, vr), vc32);

            __m128i v_b1 = _mm_add_epi16(_mm_add_epi16(y01, ub), vc32);
            __m128i v_g1 = _mm_add_epi16(_mm_add_epi16(y01, uvg), vc32);
            __m128i v_r1 = _mm_add_epi16(_mm_add_epi16(y01, vr), vc32);

            __m128i b00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_b0, 6), vc255), zero);
            __m128i g00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_g0, 6), vc255), zero);
            __m128i r00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_r0, 6), vc255), zero);

            __m128i b01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_b1, 6), vc255), zero);
            __m128i g01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_g1, 6), vc255), zero);
            __m128i r01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_r1, 6), vc255), zero);

            __m128i b0 = _mm_shuffle_epi8(_mm_packus_epi16(b00, b01), zip_index);
            __m128i g0 = _mm_shuffle_epi8(_mm_packus_epi16(g00, g01), zip_index);
            __m128i r0 = _mm_shuffle_epi8(_mm_packus_epi16(r00, r01), zip_index);
            // y1
            v_b0 = _mm_add_epi16(_mm_add_epi16(y10, ub), vc32);
            v_g0 = _mm_add_epi16(_mm_add_epi16(y10, uvg), vc32);
            v_r0 = _mm_add_epi16(_mm_add_epi16(y10, vr), vc32);

            v_b1 = _mm_add_epi16(_mm_add_epi16(y11, ub), vc32);
            v_g1 = _mm_add_epi16(_mm_add_epi16(y11, uvg), vc32);
            v_r1 = _mm_add_epi16(_mm_add_epi16(y11, vr), vc32);

            b00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_b0, 6), vc255), zero);
            g00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_g0, 6), vc255), zero);
            r00 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_r0, 6), vc255), zero);

            b01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_b1, 6), vc255), zero);
            g01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_g1, 6), vc255), zero);
            r01 = _mm_max_epi16(_mm_min_epi16(_mm_srai_epi16(v_r1, 6), vc255), zero);

            __m128i b1 = _mm_shuffle_epi8(_mm_packus_epi16(b00, b01), zip_index);
            __m128i g1 = _mm_shuffle_epi8(_mm_packus_epi16(g00, g01), zip_index);
            __m128i r1 = _mm_shuffle_epi8(_mm_packus_epi16(r00, r01), zip_index);

            vst3_u8x16_sse(&b0, &g0, &r0, dst0_ptr);
            vst3_u8x16_sse(&b1, &g1, &r1, dst1_ptr);

            y0_ptr   += 16;
            dst0_ptr += 48;
            y1_ptr   += 16;
            dst1_ptr += 48;
            u0 += 8;
            v0 += 8;
        }

        if (dst_w - width_align8) {
            int uv_off = width_align8 >> 1;
            convet_yuv420_to_bgr_one_row(src_ptr + width_align8, dst_ptr + width_align8 * 3, 
                    u + uv_off, v + uv_off, dst_w, src_stride, dst_stride, width_align8, 3);
        }

        src_ptr += doub_src_stride;
        dst_ptr += doub_dst_stride;
        u += (src_stride >> 1);
        v += (src_stride >> 1);
    }
}

int cvt_color_sse(
        const Mat& src_y,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_sse(src_y, src_u, src_v, dst);
        break;
    default:
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
