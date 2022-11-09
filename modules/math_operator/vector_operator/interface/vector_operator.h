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

#include <stdlib.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief malloc sizeof(T) * dimension, align by cache line
 * @param[in] dimension vector dimensions
*/
template<typename T>
T* alloc_vector(int dimension) {
    T* ptr = (T*)malloc(dimension * sizeof(T));
    return (ptr == nullptr) ? nullptr : ptr;
}

/**
 * @brief free sizeof(T) * dimension, align by cache line
 * @param[in] block vector data pointer
*/
template<typename T>
void free_vector(T* block) {
    free(block);
}

/**
 * @brief malloc sizeof(float) * dimension * rows, align by cache line
 * @param[in] dimension matrix row vector dimensions
 * @param[in] rows matrix rows number
*/
template<typename T>
T* alloc_matrix(int dimension, int rows) {
    T* ptr = (T*)malloc(rows * dimension * sizeof(T));
    return (ptr == nullptr) ? nullptr : ptr;
}

/**
 * @brief free sizeof(T) * dimension * rows, align by cache line
 * @param[in] block matrix data pointer
*/
template<typename T>
void free_matrix(T* block) {
    free(block);
}

/**
 * @brief get vector pointer for matrix(rowid)
 * @param[in] dimension matrix row dimension
 * @param[in] rowid matrix row sequence number
 * @param[in] matrix matrix data pointer
*/
template<typename T>
inline T* peek_vector(int dimension, int rowid, T* matrix) {
    return matrix + dimension * rowid;
}

/**
 * @brief get sum(i^2) by vector(float type)
 * @param[in] dimension vector dimensions
 * @param[in] vector vector data pointer
 * @return L2_Norm result of vector
*/
FCV_API float get_l2(int dimension, float* vector);

/**
 * @brief float value * scaler = int8_t value
 * @param[in] dimension vector dimensions
 * @param[in] vector1 float type vector data pointer
 * @param[in] scaler Conversion factor
 * @param[out] vector2 int8_t type vector data pointer
*/
FCV_API void scaler(
        int dimension,
        float* vector1,
        float scaler,
        int8_t* vector2);

/**
 * @brief normalize the vector, let L2 = 1 for each vector
 * @param[in] dimension vector dimensions
 * @param[out] vector float type vector data pointer
*/
FCV_API void normalize_vector(int dimension, float* vector);

/**
 * @brief normalize the matrix, let L2 = 1 for each row
 * @param[in] dimension vector dimensions
 * @param[in] rows matrix rows number
 * @param[out] matrix float type matrix data pointer
*/
FCV_API void normalize_matrix(
        int dimension,
        int rows,
        float* matrix);

/**
 * @brief dot v1 to v2, output to dot
 * @param[in] dimension vector dimensions
 * @param[in] v0 float type vector data pointer
 * @param[in] v1 float type vector data pointer
 * @param[out] data float type dot result
*/
FCV_API void dot_vectors(
        int dimension,
        float* v0,
        float* v1,
        float* dot);

/**
 * @brief dot v1 to v2, output to dot
 * @param[in] dimension vector dimensions
 * @param[in] v0 int8_t type vector data pointer
 * @param[in] v1 int8_t type vector data pointer
 * @param[out] data int32_t type dot result
*/
FCV_API void dot_vectors(
        int dimension,
        int8_t* v0,
        int8_t* v1,
        int32_t* dot);

/**
 * @brief dot matrix to vector, output to dots
 * @param[in] dimension vector dimensions
 * @param[in] rows matrix rows number
 * @param[in] matrix float type matrix data pointer
 * @param[in] vector float type vector data pointer
 * @param[out] dots float type dot result array for each row
*/
FCV_API void dot_matrix_vector(
        int dimension,
        int rows,
        float* matrix,
        float* vector,
        float* dots);

/**
 * @brief dot matrix to vector, output to dots
 * @param[in] dimension vector dimensions
 * @param[in] rows matrix rows number
 * @param[in] matrix int8_t type matrix data pointer
 * @param[in] vector int8_t type vector data pointer
 * @param[out] dots int32_t type dot result array for each row
*/
FCV_API void dot_matrix_vector(
        int dimension,
        int rows,
        int8_t* matrix,
        int8_t* vector,
        int32_t* dots);

/**
 * @brief copy vector data from vector to matrix(rowid)
 * @param[in] dimension vector dimensions
 * @param[in] rowid matrix rows sequence number
 * @param[in] matrix float type matrix data pointer
 * @param[out] vector float type vector data pointer
*/
FCV_API void put_vector(
        int dimension,
        int rowid,
        float* matrix,
        float* vector);

/**
 * @brief copy vector data from vector to matrix(rowid)
 * @param[in] dimension vector dimensions
 * @param[in] rowid matrix rows sequence number
 * @param[in] matrix int8_t type matrix data pointer
 * @param[out] vector int8_t type vector data pointer
*/
FCV_API void put_vector(
        int dimension,
        int rowid,
        int8_t* matrix,
        int8_t* vector);

/**
 * @brief copy vector data from matrix(rowid) to vector
 * @param[in] dimension vector dimensions
 * @param[in] rowid matrix rows sequence number
 * @param[in] matrix float type matrix data pointer
 * @param[out] vector float type vector data pointer
*/
FCV_API void get_vector(
        int dimension,
        int rowid,
        float* matrix,
        float* vector);

/**
 * @brief copy vector data from matrix(rowid) to vector
 * @param[in] dimension vector dimensions
 * @param[in] rowid matrix rows sequence number
 * @param[in] matrix int8_t type matrix data pointer
 * @param[out] vector int8_t type vector data pointer
*/
FCV_API void get_vector(
        int dimension,
        int rowid,
        int8_t* matrix,
        int8_t* vector);

/**
 * @brief convert data type from float to int8_t by scalar
 * @param[in] dimension vector dimensions
 * @param[in] vector1 float type matrix data pointer
 * @param[in] scaler convert data type coefficient
 * @param[out] vector2 int8_t type vector data pointer
*/
FCV_API void scaler(
        int dimension,
        float* vector1,
        float scaler,
        int8_t* vector2);

/**
 * @brief vector_y = vector_y + vector_x * scalar
 * @param[in] dimension vector dimensions
 * @param[in] scaler caculate data coefficient
 * @param[in] vector_x float type vector data pointer
 * @param[inout] vector_y float type vector data pointer
*/
FCV_API void axpy_vector(
        int dimension,
        float ratio,
        float* vector_x,
        float* vector_y);

/**
 * @brief vector_y = vector_y + vector_x * scalar
 * @param[in] dimension vector dimensions
 * @param[in] scaler caculate data coefficient
 * @param[in] vector_x int8_t type vector data pointer
 * @param[inout] vector_y float type vector data pointer
*/
FCV_API void axpy_vector(
        int dimension,
        float ratio,
        int8_t* vector_x,
        float* vector_y);

/**
 * @brief vector = vector * scalar
 * @param[in] dimension vector dimensions
 * @param[in] scaler caculate data coefficient
 * @param[inout] vector float type vector data pointer
*/
FCV_API void product_vector(
        int dimension,
        float scalar,
        float* vector);

/**
 * @brief normalize(center * scalar + matrix rows[dim] values) -> center
 * @param[in] dimension vector dimensions
 * @param[in] rows matrix rows number
 * @param[in] matrix float type matrix data pointer
 * @param[in] scaler caculate data coefficient
 * @param[out] center float type vector data pointer
*/
FCV_API void product_center(
        int dimension,
        int rows,
        float* matrix,
        int scalar,
        float* center);

/**
 * @brief get matrix center
 * @param[in] dimension vector dimensions
 * @param[in] rows matrix rows number
 * @param[in] matrix float type matrix data pointer
 * @param[out] center float type vector data pointer
*/
FCV_API inline void get_center(
        int dimension,
        int rows,
        float* matrix,
        float* center) {
    product_center(dimension, rows, matrix, 0, center);
}

G_FCV_NAMESPACE1_END()
