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

#pragma once

#include "modules/core/mat/interface/mat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief get sum(i^2) by vector(float type)
 * @param dimension vector dimensions
 * @param vector vector data pointer
 * @return L2_Norm result of vector
*/
float get_l2_neon(int dimension, float* vector);

/**
 * @brief float value * scaler = int8_t value
 * @param dimension vector dimensions
 * @param vector1 float type vector data pointer
 * @param scaler Conversion factor
 * @param vector2 int8_t type vector data pointer
*/
void scaler_neon(int dimension, float* vector1, float scaler, int8_t* vector2);

/**
 * @brief normalize the vector, let L2 = 1 for each vector
 * @param dimension vector dimensions
 * @param vector float type vector data pointer
*/
void normalize_vector_neon(int dimension, float* vector);

/**
 * @brief dot v1 to v2, output to dot
 * @param dimension vector dimensions
 * @param v0 float type vector data pointer
 * @param v1 float type vector data pointer
 * @param data float type dot result
*/
void dot_vectors_neon(
        int dimension,
        float* v0,
        float* v1,
        float* dot);

/**
 * @brief dot v1 to v2, output to dot
 * @param dimension vector dimensions
 * @param v0 int8_t type vector data pointer
 * @param v1 int8_t type vector data pointer
 * @param data int32_t type dot result
*/
void dot_vectors_neon(
        int dimension,
        int8_t* v0,
        int8_t* v1,
        int32_t* dot);

/**
 * @brief vector_y = vector_y + vector_x * scalar
 * @param dimension vector dimensions
 * @param scaler caculate data coefficient
 * @param vector_x float type vector data pointer
 * @param vector_y float type vector data pointer
*/
void axpy_vector_neon(
        int dimension,
        float scalar,
        float* vector_x,
        float* vector_y);

/**
 * @brief vector_y = vector_y + vector_x * scalar
 * @param dimension vector dimensions
 * @param scaler caculate data coefficient
 * @param vector_x int8_t type vector data pointer
 * @param vector_y float type vector data pointer
*/
void axpy_vector_neon(
        int dimension,
        float scalar,
        int8_t* vector_x,
        float* vector_y);

G_FCV_NAMESPACE1_END()
