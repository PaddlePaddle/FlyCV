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

#include "modules/math_operator/vector_operator/interface/vector_operator.h"
#include "modules/math_operator/vector_operator/include/vector_operator_common.h"

#ifdef HAVE_NEON
#include "modules/math_operator/vector_operator/include/vector_operator_arm.h"
#endif

#ifndef WIN32
#include <unistd.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

float get_l2(int dimension, float* vector) {
    float nrm = 0.0;
#ifdef HAVE_NEON
    nrm = get_l2_neon(dimension, vector);
#else
    nrm = get_l2_common(dimension, vector);
#endif
    return nrm;
}

void scaler(
        int dimension,
        float *vector1,
        float scaler,
        int8_t *vector2) {
#ifdef HAVE_NEON
    scaler_neon(dimension, vector1, scaler, vector2);
#else
    scaler_common(dimension, vector1, scaler, vector2);
#endif
}

// TODO: dimension % 4 != 0
void normalize_vector(int dimension, float* vector) {
#ifdef HAVE_NEON
    normalize_vector_neon(dimension, vector);
#else
    normalize_vector_common(dimension, vector);
#endif
}

void normalize_matrix(int dimension, int rows, float* matrix) {
    for (int idx = 0; idx < rows; ++idx) {
        normalize_vector(dimension, peek_vector(dimension, idx, matrix));
    }
    return;
}

void dot_vectors(
        int dimension,
        float* v0,
        float* v1,
        float* dot) {
#ifdef HAVE_NEON
    dot_vectors_neon(dimension, v0, v1, dot);
#else
    dot_vectors_common(dimension, v0, v1, dot);
#endif
}

void dot_vectors(
        int dimension,
        int8_t* v0,
        int8_t* v1,
        int32_t* dot) {
#ifdef HAVE_NEON
    dot_vectors_neon(dimension, v0, v1, dot);
#else
    dot_vectors_common(dimension, v0, v1, dot);
#endif
}

void axpy_vector(
        int dimension,
        float ratio,
        float* vector_x,
        float* vector_y) {
#ifdef HAVE_NEON
    axpy_vector_neon(dimension, ratio, vector_x, vector_y);
#else
    axpy_vector_common(dimension, ratio, vector_x, vector_y);
#endif
}

void axpy_vector(
        int dimension,
        float ratio,
        int8_t* vector_x,
        float* vector_y) {
#ifdef HAVE_NEON
    axpy_vector_neon(dimension, ratio, vector_x, vector_y);
#else
    axpy_vector_common(dimension, ratio, vector_x, vector_y);
#endif
}

void dot_matrix_vector(
        int dimension,
        int rows,
        float *matrix,
        float *vector,
        float *dots) {
    for (int idx = 0; idx < rows; ++idx) {
        dot_vectors(dimension, peek_vector(dimension, idx, matrix),
                vector, dots + idx);
    }
}

void dot_matrix_vector(
        int dimension,
        int rows,
        int8_t *matrix,
        int8_t *vector,
        int32_t *dots) {
    for (int idx = 0; idx < rows; ++idx) {
        dot_vectors(dimension, peek_vector(dimension, idx, matrix),
                vector, dots + idx);
    }
}

void put_vector(
        int dimension,
        int rowid,
        float* matrix,
        float* vector) {
    memcpy(peek_vector(dimension, rowid, matrix),
            vector, dimension * sizeof(float));
}

void put_vector(
        int dimension,
        int rowid,
        int8_t* matrix,
        int8_t* vector) {
    memcpy(peek_vector(dimension, rowid, matrix),
            vector, dimension * sizeof(int8_t));
}

void get_vector(
        int dimension,
        int rowid,
        float* matrix,
        float* vector) {
    memcpy(vector, peek_vector(dimension, rowid, matrix),
            dimension * sizeof(float));
}

void get_vector(
        int dimension,
        int rowid,
        int8_t* matrix,
        int8_t* vector) {
    memcpy(vector, peek_vector(dimension, rowid, matrix),
            dimension * sizeof(int8_t));
}

void product_vector(
        int dimension,
        float scalar,
        float* vector) {
    for (int idx = 0; idx < dimension; ++idx) {
        vector[idx] *= scalar;
    }
}

void product_center(
        int dimension,
        int rows,
        float *matrix,
        int scalar,
        float* center) {
    for (int dim = 0; dim < dimension; ++dim) {
        float* ptr = matrix + dim;
        float sum = center[dim] * scalar;
        for (int idx = 0; idx < rows; ++idx) {
            sum += *ptr;
            ptr += dimension;
        }
        center[dim] = sum;
    }
    normalize_vector(dimension, center);
}

G_FCV_NAMESPACE1_END()
