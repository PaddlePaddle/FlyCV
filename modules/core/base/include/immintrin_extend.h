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

// @brief : This header file is used to supplement the implementation of
// some mmx/sse/avx intrinsic functions in the old compilation environment
// on x86 platform
#pragma once
#include <immintrin.h>

inline __m256i _mm256_loadu2_m128i(__m128i const* __addr_hi, __m128i const* __addr_lo) {
    __m256i __v256 = _mm256_castsi128_si256(_mm_loadu_si128(__addr_lo));
    return _mm256_insertf128_si256(__v256, _mm_loadu_si128(__addr_hi), 1);
}

inline void _mm256_storeu2_m128i(__m128i* __addr_hi, __m128i* __addr_lo, __m256i __a) {
    __m128i __v128;
    __v128 = _mm256_castsi256_si128(__a);
    _mm_storeu_si128(__addr_lo, __v128);
    __v128 = _mm256_extractf128_si256(__a, 1);
    _mm_storeu_si128(__addr_hi, __v128);
}

inline void _mm_storeu_si32(void* mem_addr, __m128i a) {
    _mm_store_ss((float*)mem_addr, _mm_castsi128_ps(a));
}

inline void _mm_storeu_si64(void* mem_addr, __m128i a) {
    _mm_store_sd((double*)mem_addr, _mm_castsi128_pd(a));
}
