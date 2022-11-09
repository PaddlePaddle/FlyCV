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

#include "modules/math_operator/vector_operator/include/vector_operator_common.h"

#include <cmath>
#include <climits>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

float get_l2_common(int dimension, float* vector) {
    float nrm = 0.0;
    for (int idx = 0; idx < dimension; ++idx) {
        float v = vector[idx];
        nrm += v * v;
    }
    if (nrm <= FCV_EPSILON) {
        return 0.0;
    }
    nrm = sqrtf(nrm);
    return nrm;
}

void scaler_common(
        int dimension,
        float *vector1,
        float scaler,
        int8_t *vector2) {
     for (int idx = 0; idx < dimension; ++idx) {
        int32_t temp = static_cast<int32_t> (vector1[idx] * scaler);
        if (temp < 0) {
            vector2[idx] = fcv_cast_s8(temp < SCHAR_MIN ? SCHAR_MIN : temp);
        } else {
            vector2[idx] = fcv_cast_s8(temp > SCHAR_MAX ? SCHAR_MAX : temp);
        }
    }
}

void normalize_vector_common(int dimension, float* vector) {
    float nrm = get_l2_common(dimension, vector);
    if (nrm <= FCV_EPSILON) {
        vector[0] = 1.0;
        return;
    }

    float dnrm = 1.0f / nrm;
    for (int idx = 0; idx < dimension; ++idx) {
        vector[idx] *= dnrm;
    }

    return;
}

void dot_vectors_common(
        int dimension,
        float *v0,
        float *v1,
        float *dot) {
    float sum = 0.0;
    for (int idx = 0; idx < dimension; ++idx) {
        sum += v0[idx] * v1[idx];
    }
    *dot = sum;

}

void dot_vectors_common(
        int dimension,
        int8_t* v0,
        int8_t* v1,
        int32_t* dot) {
    int32_t sum = 0;
    for (int idx = 0; idx < dimension; ++idx) {
        sum += v0[idx] * v1[idx];
    }
    *dot = sum;
}

void axpy_vector_common(
        int dimension,
        float scalar,
        float* vector_x,
        float* vector_y) {
    for (int idx = 0; idx < dimension; ++idx) {
        vector_y[idx] += scalar * vector_x[idx];
    }
}

void axpy_vector_common(
        int dimension,
        float scalar,
        int8_t* vector_x,
        float* vector_y) {
    for (int idx = 0; idx < dimension; ++idx) {
        vector_y[idx] += scalar * vector_x[idx];
    }
}

G_FCV_NAMESPACE1_END()
