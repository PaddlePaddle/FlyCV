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

#include "flycv_namespace.h"
#include "macro_ns.h"

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

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef HAVE_SSE
static __m128i planner_to_b0_shuff = _mm_set_epi8(
        5, -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0);
static __m128i planner_to_b1_shuff = _mm_set_epi8(
        -1, 10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1);
static __m128i planner_to_b2_shuff = _mm_set_epi8(
     -1, -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1);

static __m128i planner_to_g0_shuff = _mm_set_epi8(
        -1, -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1);
static __m128i planner_to_g1_shuff = _mm_set_epi8(
        10, -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5);
static __m128i planner_to_g2_shuff = _mm_set_epi8(
     -1, 15, -1, -1, 14, -1, -1, 13, -1, -1, 12, -1, -1, 11, -1, -1);

static __m128i planner_to_r0_shuff = _mm_set_epi8(
        -1, 4, -1, -1, 3, -1, -1, 2, -1, -1, 1, -1, -1, 0, -1, -1);
static __m128i planner_to_r1_shuff = _mm_set_epi8(
        -1, -1, 9, -1, -1, 8, -1, -1, 7, -1, -1, 6, -1, -1, 5, -1);
static __m128i planner_to_r2_shuff = _mm_set_epi8(
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

inline void vld3_u8x16_sse(
        unsigned char* src,
        __m128i* b,
        __m128i* g,
        __m128i* r) {
    const __m128i bgr0 = _mm_loadu_si128((const __m128i*)(src +  0));
    const __m128i bgr1 = _mm_loadu_si128((const __m128i*)(src + 16));
    const __m128i bgr2 = _mm_loadu_si128((const __m128i*)(src + 32));

    // cout << __m128i_toString<uint8_t>(bgr0) << endl;
    // cout << __m128i_toString<uint8_t>(bgr1) << endl;
    // cout << __m128i_toString<uint8_t>(bgr2) << endl;

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

inline void vst3_u8x16_sse(
        __m128i* b,
        __m128i* g,
        __m128i* r,
        unsigned char* dst) {
    __m128i bgr0, bgr1, bgr2;
    __m128i G0 = _mm_shuffle_epi8(*g, planner_to_g0_shuff);
    __m128i G1 = _mm_shuffle_epi8(*g, planner_to_g1_shuff);
    __m128i G2 = _mm_shuffle_epi8(*g, planner_to_g2_shuff);

    __m128i B0 = _mm_shuffle_epi8(*b, planner_to_b0_shuff);
    __m128i B1 = _mm_shuffle_epi8(*b, planner_to_b1_shuff);
    __m128i B2 = _mm_shuffle_epi8(*b, planner_to_b2_shuff);

    __m128i R0 = _mm_shuffle_epi8(*r, planner_to_r0_shuff);
    __m128i R1 = _mm_shuffle_epi8(*r, planner_to_r1_shuff);
    __m128i R2 = _mm_shuffle_epi8(*r, planner_to_r2_shuff);

    bgr0 = _mm_or_si128(_mm_or_si128(R0, G0), B0);
    bgr1 = _mm_or_si128(_mm_or_si128(R1, G1), B1);
    bgr2 = _mm_or_si128(_mm_or_si128(R2, G2), B2);

    _mm_storeu_si128((__m128i*)(dst +  0), bgr0);
    _mm_storeu_si128((__m128i*)(dst + 16), bgr1);
    _mm_storeu_si128((__m128i*)(dst + 32), bgr2);
}

inline void vst4_u8x16_sse(
        __m128i* b,
        __m128i* g,
        __m128i* r,
        __m128i* a,
        unsigned char* dst) {
    __m128i bgra0, bgra1, bgra2, bgra3;

    __m128i bg_lo = _mm_unpacklo_epi8(*b, *g);
    __m128i ra_lo = _mm_unpacklo_epi8(*r, *a);
    __m128i bg_hi = _mm_unpackhi_epi8(*b, *g);
    __m128i ra_hi = _mm_unpackhi_epi8(*r, *a);

    bgra0 = _mm_unpacklo_epi16(bg_lo, ra_lo);
    bgra1 = _mm_unpackhi_epi16(bg_lo, ra_lo);
    bgra2 = _mm_unpacklo_epi16(bg_hi, ra_hi);
    bgra3 = _mm_unpackhi_epi16(bg_hi, ra_hi);

    _mm_storeu_si128((__m128i*)(dst +  0), bgra0);
    _mm_storeu_si128((__m128i*)(dst + 16), bgra1);
    _mm_storeu_si128((__m128i*)(dst + 32), bgra2);
    _mm_storeu_si128((__m128i*)(dst + 48), bgra3);
}

#endif

G_FCV_NAMESPACE1_END()
