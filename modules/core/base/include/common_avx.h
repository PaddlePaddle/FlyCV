// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
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

#pragma once

#include <cstdint>

#ifdef HAVE_AVX
#include <immintrin.h>
#endif

#ifdef ELDER_COMPILER
#include "immintrin_extend.h"
#endif

#include "flycv_namespace.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef HAVE_AVX
// planar to package mask
static __m256i s_pl_to_pa_b0_shuff = _mm256_set_epi8(
        5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0,
        5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0);

static __m256i s_pl_to_pa_b1_shuff = _mm256_set_epi8(
        -1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1,
        -1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1);

static __m256i s_pl_to_pa_b2_shuff = _mm256_set_epi8(
        -1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1,
        -1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1);

static __m256i s_pl_to_pa_g0_shuff = _mm256_set_epi8(
        -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1,
        -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1);

static __m256i s_pl_to_pa_g1_shuff = _mm256_set_epi8(
        10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5,
        10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5);

static __m256i s_pl_to_pa_g2_shuff = _mm256_set_epi8(
        -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1,
        -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1);

static __m256i s_pl_to_pa_r0_shuff = _mm256_set_epi8(
        -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1,
        -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1);

static __m256i s_pl_to_pa_r1_shuff = _mm256_set_epi8(
        -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1,
        -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1);

static __m256i s_pl_to_pa_r2_shuff = _mm256_set_epi8(
        15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10,
        15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10);

static __m128i package_to_planner_bshuff0 = _mm_set_epi8(
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);

static __m128i package_to_planner_bshuff1 = _mm_set_epi8(
        -1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);

static __m128i package_to_planner_bshuff2 = _mm_set_epi8(
        13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

static __m128i package_to_planner_gshuff0 = _mm_set_epi8(
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);

static __m128i package_to_planner_gshuff1 = _mm_set_epi8(
        -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);

static __m128i package_to_planner_gshuff2 = _mm_set_epi8(
        14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

static __m128i package_to_planner_rshuff0 = _mm_set_epi8(
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);

static __m128i package_to_planner_rshuff1 = _mm_set_epi8(
        -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);

static __m128i package_to_planner_rshuff2 = _mm_set_epi8(
        15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

inline void vld3_u8x16_avx(
        const unsigned char* src,
        __m128i* b,
        __m128i* g,
        __m128i* r) {
    const __m128i bgr0 = _mm_loadu_si128((const __m128i*)(src +  0));
    const __m128i bgr1 = _mm_loadu_si128((const __m128i*)(src + 16));
    const __m128i bgr2 = _mm_loadu_si128((const __m128i*)(src + 32));

    const __m128i b0 = _mm_shuffle_epi8(bgr0, package_to_planner_bshuff0);
    const __m128i g0 = _mm_shuffle_epi8(bgr0, package_to_planner_gshuff0);
    const __m128i r0 = _mm_shuffle_epi8(bgr0, package_to_planner_rshuff0);

    const __m128i b1 = _mm_shuffle_epi8(bgr1, package_to_planner_bshuff1);
    const __m128i g1 = _mm_shuffle_epi8(bgr1, package_to_planner_gshuff1);
    const __m128i r1 = _mm_shuffle_epi8(bgr1, package_to_planner_rshuff1);

    const __m128i b2 = _mm_shuffle_epi8(bgr2, package_to_planner_bshuff2);
    const __m128i g2 = _mm_shuffle_epi8(bgr2, package_to_planner_gshuff2);
    const __m128i r2 = _mm_shuffle_epi8(bgr2, package_to_planner_rshuff2);

    *b = _mm_or_si128(_mm_or_si128(b0, b1), b2);
    *g = _mm_or_si128(_mm_or_si128(g0, g1), g2);
    *r = _mm_or_si128(_mm_or_si128(r0, r1), r2);
}

inline void vld4_u8x16_avx(
        const uint8_t* data,
        __m128i* b,
        __m128i* g,
        __m128i* r,
        __m128i* a) {
    __m128i bgra_0 = _mm_loadu_si128((const __m128i*)(data + 0)); // bgra[ 0 ~  3]
    __m128i bgra_1 = _mm_loadu_si128((const __m128i*)(data + 16)); // bgra[ 4 ~  7]
    __m128i bgra_2 = _mm_loadu_si128((const __m128i*)(data + 32)); // bgra[ 8 ~ 11]
    __m128i bgra_3 = _mm_loadu_si128((const __m128i*)(data + 48)); // bgra[12 ~ 15]

    __m128i layer_0_0 = _mm_unpacklo_epi8(bgra_0, bgra_2); // bgra[(0,  8), (1,  9)]
    __m128i layer_0_1 = _mm_unpackhi_epi8(bgra_0, bgra_2); // bgra[(2, 10), (3, 11)]
    __m128i layer_0_2 = _mm_unpacklo_epi8(bgra_1, bgra_3); // bgra[(4, 12), (5, 13)]
    __m128i layer_0_3 = _mm_unpackhi_epi8(bgra_1, bgra_3); // bgra[(6, 14), (7, 15)]

    __m128i layer_1_0 = _mm_unpacklo_epi8(layer_0_0, layer_0_2); // bgra[ (0,4, 8,12) ]
    __m128i layer_1_1 = _mm_unpackhi_epi8(layer_0_0, layer_0_2); // bgra[ (1,5, 9,13) ]
    __m128i layer_1_2 = _mm_unpacklo_epi8(layer_0_1, layer_0_3); // bgra[ (2,6,10,14) ]
    __m128i layer_1_3 = _mm_unpackhi_epi8(layer_0_1, layer_0_3); // bgra[ (3,7,11,15) ]

    __m128i layer_2_0 = _mm_unpacklo_epi8(layer_1_0, layer_1_2); // bg[ (0, 2, 4, 6, 8, 10, 12, 14) ]
    __m128i layer_2_1 = _mm_unpackhi_epi8(layer_1_0, layer_1_2); // ra[ (0, 2, 4, 6, 8, 10, 12, 14) ]
    __m128i layer_2_2 = _mm_unpacklo_epi8(layer_1_1, layer_1_3); // bg[ (1, 3, 5, 7, 9, 11, 13, 15) ]
    __m128i layer_2_3 = _mm_unpackhi_epi8(layer_1_1, layer_1_3); // ra[ (1, 3, 5, 7, 9, 11, 13, 15) ]

    *b = _mm_unpacklo_epi8(layer_2_0, layer_2_2); // b[ ( 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13, 14, 15) ] x b
    *g = _mm_unpackhi_epi8(layer_2_0, layer_2_2); // g[ ( 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13, 14, 15) ] x g
    *r = _mm_unpacklo_epi8(layer_2_1, layer_2_3); // r[ ( 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13, 14, 15) ] x r
    *a = _mm_unpackhi_epi8(layer_2_1, layer_2_3); // a[ ( 0, 1, 2, 3, 4, 5, 6, 7, 8,  9, 10, 11, 12, 13, 14, 15) ] x a
    return;
}

inline void vst3_u8x32_avx(
        __m256i& b,
        __m256i& g,
        __m256i& r,
        uint8_t* dst) {
    __m256i bgr0, bgr1, bgr2;
    __m256i B0 = _mm256_shuffle_epi8(b, s_pl_to_pa_b0_shuff);
    __m256i B1 = _mm256_shuffle_epi8(b, s_pl_to_pa_b1_shuff);
    __m256i B2 = _mm256_shuffle_epi8(b, s_pl_to_pa_b2_shuff);

    __m256i G0 = _mm256_shuffle_epi8(g, s_pl_to_pa_g0_shuff);
    __m256i G1 = _mm256_shuffle_epi8(g, s_pl_to_pa_g1_shuff);
    __m256i G2 = _mm256_shuffle_epi8(g, s_pl_to_pa_g2_shuff);

    __m256i R0 = _mm256_shuffle_epi8(r, s_pl_to_pa_r0_shuff);
    __m256i R1 = _mm256_shuffle_epi8(r, s_pl_to_pa_r1_shuff);
    __m256i R2 = _mm256_shuffle_epi8(r, s_pl_to_pa_r2_shuff);

    bgr0 = _mm256_or_si256(_mm256_or_si256(R0, G0), B0);
    bgr1 = _mm256_or_si256(_mm256_or_si256(R1, G1), B1);
    bgr2 = _mm256_or_si256(_mm256_or_si256(R2, G2), B2);

    // TODO (chenlong22@baidu.com): May optimize store here later!
    // _mm_storeu_si128((__m128i*)(dst +  0), _mm256_castsi256_si128(bgr0));
    // _mm_storeu_si128((__m128i*)(dst + 16), _mm256_castsi256_si128(bgr1));
    // _mm_storeu_si128((__m128i*)(dst + 32), _mm256_castsi256_si128(bgr2));

    // _mm_storeu_si128((__m128i*)(dst + 48), _mm256_extracti128_si256(bgr0, 1));
    // _mm_storeu_si128((__m128i*)(dst + 64), _mm256_extracti128_si256(bgr1, 1));
    // _mm_storeu_si128((__m128i*)(dst + 80), _mm256_extracti128_si256(bgr2, 1));

    _mm256_storeu_si256((__m256i*)(dst +  0),
            _mm256_permute2x128_si256(bgr0, bgr1, 0b00100000));
    _mm256_storeu_si256((__m256i*)(dst + 32),
            _mm256_permute2x128_si256(bgr2, bgr0, 0b00110000));
    _mm256_storeu_si256((__m256i*)(dst + 64),
            _mm256_permute2x128_si256(bgr1, bgr2, 0b00110001));
}

inline void vst4_u8x32_avx(
        const __m256i& vec_b,
        const __m256i& vec_g,
        const __m256i& vec_r,
        const __m256i& vec_a,
        uint8_t* ptr_dst) {
    __m256i b0_r0_b2_r2 = _mm256_unpacklo_epi8(vec_b, vec_r);
    __m256i b1_r1_b3_r3 = _mm256_unpackhi_epi8(vec_b, vec_r);

    __m256i g0_a0_g2_a2 = _mm256_unpacklo_epi8(vec_g, vec_a);
    __m256i g1_a1_g3_a3 = _mm256_unpackhi_epi8(vec_g, vec_a);

    __m256i bgra_0_l_bgra_2_l = _mm256_unpacklo_epi8(b0_r0_b2_r2, g0_a0_g2_a2);
    __m256i bgra_0_h_bgra_2_h = _mm256_unpackhi_epi8(b0_r0_b2_r2, g0_a0_g2_a2);

    __m256i bgra_1_l_bgra_3_l = _mm256_unpacklo_epi8(b1_r1_b3_r3, g1_a1_g3_a3);
    __m256i bgra_1_h_bgra_3_h = _mm256_unpackhi_epi8(b1_r1_b3_r3, g1_a1_g3_a3);

    __m256i bgra_0 = _mm256_permute2x128_si256(bgra_0_l_bgra_2_l, bgra_0_h_bgra_2_h, 0b00100000);
    __m256i bgra_2 = _mm256_permute2x128_si256(bgra_0_l_bgra_2_l, bgra_0_h_bgra_2_h, 0b00110001);
    __m256i bgra_1 = _mm256_permute2x128_si256(bgra_1_l_bgra_3_l, bgra_1_h_bgra_3_h, 0b00100000);
    __m256i bgra_3 = _mm256_permute2x128_si256(bgra_1_l_bgra_3_l, bgra_1_h_bgra_3_h, 0b00110001);

    _mm256_storeu_si256((__m256i*)(ptr_dst +  0), bgra_0);
    _mm256_storeu_si256((__m256i*)(ptr_dst + 32), bgra_1);
    _mm256_storeu_si256((__m256i*)(ptr_dst + 64), bgra_2);
    _mm256_storeu_si256((__m256i*)(ptr_dst + 96), bgra_3);
}

inline void vst4_u8x16_avx(
        const __m128i& vec_b,
        const __m128i& vec_g,
        const __m128i& vec_r,
        const __m128i& vec_a,
        uint8_t* ptr_dst) {
    __m128i br_l = _mm_unpacklo_epi8(vec_b, vec_r);
    __m128i br_h = _mm_unpackhi_epi8(vec_b, vec_r);
    __m128i ga_l = _mm_unpacklo_epi8(vec_g, vec_a);
    __m128i ga_h = _mm_unpackhi_epi8(vec_g, vec_a);

    __m128i bgra_0 = _mm_unpacklo_epi8(br_l, ga_l);
    __m128i bgra_1 = _mm_unpackhi_epi8(br_l, ga_l);
    __m128i bgra_2 = _mm_unpacklo_epi8(br_h, ga_h);
    __m128i bgra_3 = _mm_unpackhi_epi8(br_h, ga_h);

    _mm_storeu_si128((__m128i*)(ptr_dst +  0), bgra_0);
    _mm_storeu_si128((__m128i*)(ptr_dst + 16), bgra_1);
    _mm_storeu_si128((__m128i*)(ptr_dst + 32), bgra_2);
    _mm_storeu_si128((__m128i*)(ptr_dst + 48), bgra_3);
}

class Vst3_U8x16_Avx {
public:
    void store(uint8_t* dst, __m128i& vec_c0, __m128i& vec_c1, __m128i& vec_c2) {
        __m128i B0 = _mm_shuffle_epi8(vec_c0, _planner_to_b0_shuff);
        __m128i B1 = _mm_shuffle_epi8(vec_c0, _planner_to_b1_shuff);
        __m128i B2 = _mm_shuffle_epi8(vec_c0, _planner_to_b2_shuff);

        __m128i G0 = _mm_shuffle_epi8(vec_c1, _planner_to_g0_shuff);
        __m128i G1 = _mm_shuffle_epi8(vec_c1, _planner_to_g1_shuff);
        __m128i G2 = _mm_shuffle_epi8(vec_c1, _planner_to_g2_shuff);

        __m128i R0 = _mm_shuffle_epi8(vec_c2, _planner_to_r0_shuff);
        __m128i R1 = _mm_shuffle_epi8(vec_c2, _planner_to_r1_shuff);
        __m128i R2 = _mm_shuffle_epi8(vec_c2, _planner_to_r2_shuff);

        __m128i bgr0 = _mm_or_si128(_mm_or_si128(R0, G0), B0);
        __m128i bgr1 = _mm_or_si128(_mm_or_si128(R1, G1), B1);
        __m128i bgr2 = _mm_or_si128(_mm_or_si128(R2, G2), B2);

        _mm_storeu_si128((__m128i*)(dst +  0), bgr0);
        _mm_storeu_si128((__m128i*)(dst + 16), bgr1);
        _mm_storeu_si128((__m128i*)(dst + 32), bgr2);
        return;
    }
private:
    __m128i _planner_to_b0_shuff = _mm_set_epi8(
        5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0);
    __m128i _planner_to_b1_shuff = _mm_set_epi8(
        -1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1);
    __m128i _planner_to_b2_shuff = _mm_set_epi8(
        -1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1);

    __m128i _planner_to_g0_shuff = _mm_set_epi8(
        -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1);
    __m128i _planner_to_g1_shuff = _mm_set_epi8(
        10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5);
    __m128i _planner_to_g2_shuff = _mm_set_epi8(
        -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1);

    __m128i _planner_to_r0_shuff = _mm_set_epi8(
        -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1);
    __m128i _planner_to_r1_shuff = _mm_set_epi8(
        -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1);
    __m128i _planner_to_r2_shuff = _mm_set_epi8(
        15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1, 10);
};

class Vst3_F32x8_Avx {
public:
    void store(float* dst_f32, __m256& vec_c0, __m256& vec_c1, __m256& vec_c2) {
        __m256i b0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), _mask_c0_0);
        __m256i b1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), _mask_c0_1);
        __m256i b2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c0), _mask_c0_2);

        __m256i g0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), _mask_c1_0);
        __m256i g1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), _mask_c1_1);
        __m256i g2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c1), _mask_c1_2);

        __m256i r0 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), _mask_c2_0);
        __m256i r1 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), _mask_c2_1);
        __m256i r2 = _mm256_shuffle_epi8(_mm256_castps_si256(vec_c2), _mask_c2_2);

        __m256i bgr0 = _mm256_or_si256(_mm256_or_si256(b0, g0), r0);
        __m256i bgr1 = _mm256_or_si256(_mm256_or_si256(b1, g1), r1);
        __m256i bgr2 = _mm256_or_si256(_mm256_or_si256(b2, g2), r2);

        _mm256_storeu_si256((__m256i*)(dst_f32 +  0),
                _mm256_permute2x128_si256(bgr0, bgr1, 0b00100000));
        _mm256_storeu_si256((__m256i*)(dst_f32 +  8),
                _mm256_permute2x128_si256(bgr2, bgr0, 0b00110000));
        _mm256_storeu_si256((__m256i*)(dst_f32 + 16),
                _mm256_permute2x128_si256(bgr1, bgr2, 0b00110001));

        return;
    }

private:
    __m256i _mask_c0_0 = _mm256_set_epi8(
            7, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0,
            7, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0);

    __m256i _mask_c0_1 = _mm256_set_epi8(
            -1, -1, -1, -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c0_2 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1);

    __m256i _mask_c1_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1);

    __m256i _mask_c1_1 = _mm256_set_epi8(
            11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4,
            11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4);

    __m256i _mask_c1_2 = _mm256_set_epi8(
            -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c2_0 = _mm256_set_epi8(
            -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c2_1 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1);

    __m256i _mask_c2_2 = _mm256_set_epi8(
            15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8,
            15, 14, 13, 12, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8);
};

class Vld3_F32x8_Avx {
public:
    void load(const float* src_f32,
            __m256i* vec_c0,
            __m256i* vec_c1,
            __m256i* vec_c2) {
        __m256i bgr_0 = _mm256_loadu2_m128i((__m128i const*)(src_f32 + 12), (__m128i const*)(src_f32 + 0));
        __m256i bgr_1 = _mm256_loadu2_m128i((__m128i const*)(src_f32 + 16), (__m128i const*)(src_f32 + 4));
        __m256i bgr_2 = _mm256_loadu2_m128i((__m128i const*)(src_f32 + 20), (__m128i const*)(src_f32 + 8));

        __m256i b0 = _mm256_shuffle_epi8(bgr_0, _mask_c0_0);
        __m256i b1 = _mm256_shuffle_epi8(bgr_1, _mask_c0_1);
        __m256i b2 = _mm256_shuffle_epi8(bgr_2, _mask_c0_2);

        __m256i g0 = _mm256_shuffle_epi8(bgr_0, _mask_c1_0);
        __m256i g1 = _mm256_shuffle_epi8(bgr_1, _mask_c1_1);
        __m256i g2 = _mm256_shuffle_epi8(bgr_2, _mask_c1_2);

        __m256i r0 = _mm256_shuffle_epi8(bgr_0, _mask_c2_0);
        __m256i r1 = _mm256_shuffle_epi8(bgr_1, _mask_c2_1);
        __m256i r2 = _mm256_shuffle_epi8(bgr_2, _mask_c2_2);

        *vec_c0 = _mm256_or_si256(_mm256_or_si256(b0, b1), b2);
        *vec_c1 = _mm256_or_si256(_mm256_or_si256(g0, g1), g2);
        *vec_c2 = _mm256_or_si256(_mm256_or_si256(r0, r1), r2);
        return;
    }

private:
    __m256i _mask_c0_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, 3, 2, 1, 0,
            -1, -1, -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, 3, 2, 1, 0);

    __m256i _mask_c0_1 = _mm256_set_epi8(
            -1, -1, -1, -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, 11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c0_2 = _mm256_set_epi8(
            7, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            7, 6, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c1_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4);

    __m256i _mask_c1_1 = _mm256_set_epi8(
            -1, -1, -1, -1, 15, 14, 13, 12, 3, 2, 1, 0, -1, -1, -1, -1,
            -1, -1, -1, -1, 15, 14, 13, 12, 3, 2, 1, 0, -1, -1, -1, -1);

    __m256i _mask_c1_2 = _mm256_set_epi8(
            11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            11, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c2_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 11, 10, 9, 8);

    __m256i _mask_c2_1 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, 7, 6, 5, 4, -1, -1, -1, -1);

    __m256i _mask_c2_2 = _mm256_set_epi8(
            15, 14, 13, 12, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1,
            15, 14, 13, 12, 3, 2, 1, 0, -1, -1, -1, -1, -1, -1, -1, -1);
};

class Vld3_U8x32_Avx {
public:
    void load(const uint8_t* src_u8,
            __m256i* vec_c0,
            __m256i* vec_c1,
            __m256i* vec_c2) {
        __m256i bgr_0 = _mm256_loadu2_m128i((__m128i const*)(src_u8 + 48), (__m128i const*)(src_u8 +  0));
        __m256i bgr_1 = _mm256_loadu2_m128i((__m128i const*)(src_u8 + 64), (__m128i const*)(src_u8 + 16));
        __m256i bgr_2 = _mm256_loadu2_m128i((__m128i const*)(src_u8 + 80), (__m128i const*)(src_u8 + 32));

        __m256i b0 = _mm256_shuffle_epi8(bgr_0, _mask_c0_0);
        __m256i b1 = _mm256_shuffle_epi8(bgr_1, _mask_c0_1);
        __m256i b2 = _mm256_shuffle_epi8(bgr_2, _mask_c0_2);

        __m256i g0 = _mm256_shuffle_epi8(bgr_0, _mask_c1_0);
        __m256i g1 = _mm256_shuffle_epi8(bgr_1, _mask_c1_1);
        __m256i g2 = _mm256_shuffle_epi8(bgr_2, _mask_c1_2);

        __m256i r0 = _mm256_shuffle_epi8(bgr_0, _mask_c2_0);
        __m256i r1 = _mm256_shuffle_epi8(bgr_1, _mask_c2_1);
        __m256i r2 = _mm256_shuffle_epi8(bgr_2, _mask_c2_2);

        *vec_c0 = _mm256_or_si256(_mm256_or_si256(b0, b1), b2);
        *vec_c1 = _mm256_or_si256(_mm256_or_si256(g0, g1), g2);
        *vec_c2 = _mm256_or_si256(_mm256_or_si256(r0, r1), r2);
    }

private:
    __m256i _mask_c0_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);
    __m256i _mask_c0_1 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);
    __m256i _mask_c0_2 = _mm256_set_epi8(
            13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c1_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);
    __m256i _mask_c1_1 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);
    __m256i _mask_c1_2 = _mm256_set_epi8(
            14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m256i _mask_c2_0 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);
    __m256i _mask_c2_1 = _mm256_set_epi8(
            -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);
    __m256i _mask_c2_2 = _mm256_set_epi8(
            15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
};

class Deinterleave_4c_Avx {
public:
    void load(const uint8_t* src_bgra,
            __m128i* vec_b,
            __m128i* vec_g,
            __m128i* vec_r) {
        __m128i bgra_0 = _mm_loadu_si128((const __m128i*)(src_bgra +  0));
        __m128i bgra_1 = _mm_loadu_si128((const __m128i*)(src_bgra + 16));
        __m128i bgra_2 = _mm_loadu_si128((const __m128i*)(src_bgra + 32));
        __m128i bgra_3 = _mm_loadu_si128((const __m128i*)(src_bgra + 48));

        __m128i b0 = _mm_shuffle_epi8(bgra_0, _mask_c0_0);
        __m128i b1 = _mm_shuffle_epi8(bgra_1, _mask_c0_1);
        __m128i b2 = _mm_shuffle_epi8(bgra_2, _mask_c0_2);
        __m128i b3 = _mm_shuffle_epi8(bgra_3, _mask_c0_3);

        __m128i g0 = _mm_shuffle_epi8(bgra_0, _mask_c1_0);
        __m128i g1 = _mm_shuffle_epi8(bgra_1, _mask_c1_1);
        __m128i g2 = _mm_shuffle_epi8(bgra_2, _mask_c1_2);
        __m128i g3 = _mm_shuffle_epi8(bgra_3, _mask_c1_3);

        __m128i r0 = _mm_shuffle_epi8(bgra_0, _mask_c2_0);
        __m128i r1 = _mm_shuffle_epi8(bgra_1, _mask_c2_1);
        __m128i r2 = _mm_shuffle_epi8(bgra_2, _mask_c2_2);
        __m128i r3 = _mm_shuffle_epi8(bgra_3, _mask_c2_3);

        *vec_b = _mm_or_si128(_mm_or_si128(b0, b1), _mm_or_si128(b2, b3));
        *vec_g = _mm_or_si128(_mm_or_si128(g0, g1), _mm_or_si128(g2, g3));
        *vec_r = _mm_or_si128(_mm_or_si128(r0, r1), _mm_or_si128(r2, r3));
    }

    void load(const uint8_t* src_bgra,
            __m128i* vec_b,
            __m128i* vec_g,
            __m128i* vec_r, 
            __m128i* vec_a) {
        __m128i bgra_0 = _mm_loadu_si128((const __m128i*)(src_bgra +  0));
        __m128i bgra_1 = _mm_loadu_si128((const __m128i*)(src_bgra + 16));
        __m128i bgra_2 = _mm_loadu_si128((const __m128i*)(src_bgra + 32));
        __m128i bgra_3 = _mm_loadu_si128((const __m128i*)(src_bgra + 48));

        __m128i b0 = _mm_shuffle_epi8(bgra_0, _mask_c0_0);
        __m128i b1 = _mm_shuffle_epi8(bgra_1, _mask_c0_1);
        __m128i b2 = _mm_shuffle_epi8(bgra_2, _mask_c0_2);
        __m128i b3 = _mm_shuffle_epi8(bgra_3, _mask_c0_3);

        __m128i g0 = _mm_shuffle_epi8(bgra_0, _mask_c1_0);
        __m128i g1 = _mm_shuffle_epi8(bgra_1, _mask_c1_1);
        __m128i g2 = _mm_shuffle_epi8(bgra_2, _mask_c1_2);
        __m128i g3 = _mm_shuffle_epi8(bgra_3, _mask_c1_3);

        __m128i r0 = _mm_shuffle_epi8(bgra_0, _mask_c2_0);
        __m128i r1 = _mm_shuffle_epi8(bgra_1, _mask_c2_1);
        __m128i r2 = _mm_shuffle_epi8(bgra_2, _mask_c2_2);
        __m128i r3 = _mm_shuffle_epi8(bgra_3, _mask_c2_3);

        __m128i a0 = _mm_shuffle_epi8(bgra_0, _mask_c3_0);
        __m128i a1 = _mm_shuffle_epi8(bgra_1, _mask_c3_1);
        __m128i a2 = _mm_shuffle_epi8(bgra_2, _mask_c3_2);
        __m128i a3 = _mm_shuffle_epi8(bgra_3, _mask_c3_3);

        *vec_b = _mm_or_si128(_mm_or_si128(b0, b1), _mm_or_si128(b2, b3));
        *vec_g = _mm_or_si128(_mm_or_si128(g0, g1), _mm_or_si128(g2, g3));
        *vec_r = _mm_or_si128(_mm_or_si128(r0, r1), _mm_or_si128(r2, r3));
        *vec_a = _mm_or_si128(_mm_or_si128(a0, a1), _mm_or_si128(a2, a3));
    }
private:
    __m128i _mask_c0_0 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0);
    __m128i _mask_c0_1 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0, -1, -1, -1, -1);
    __m128i _mask_c0_2 = _mm_set_epi8(
            -1, -1, -1, -1, 12, 8, 4, 0, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i _mask_c0_3 = _mm_set_epi8(
            12, 8, 4, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i _mask_c1_0 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 9, 5, 1);
    __m128i _mask_c1_1 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 13, 9, 5, 1, -1, -1, -1, -1);
    __m128i _mask_c1_2 = _mm_set_epi8(
            -1, -1, -1, -1, 13, 9, 5, 1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i _mask_c1_3 = _mm_set_epi8(
            13, 9, 5, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i _mask_c2_0 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 10, 6, 2);
    __m128i _mask_c2_1 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 14, 10, 6, 2, -1, -1, -1, -1);
    __m128i _mask_c2_2 = _mm_set_epi8(
            -1, -1, -1, -1, 14, 10, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i _mask_c2_3 = _mm_set_epi8(
            14, 10, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i _mask_c3_0 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 11, 7, 3);
    __m128i _mask_c3_1 = _mm_set_epi8(
            -1, -1, -1, -1, -1, -1, -1, -1, 15, 11, 7, 3, -1, -1, -1, -1);
    __m128i _mask_c3_2 = _mm_set_epi8(
            -1, -1, -1, -1, 15, 11, 7, 3, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i _mask_c3_3 = _mm_set_epi8(
            15, 11, 7, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
};
#endif

G_FCV_NAMESPACE1_END()
