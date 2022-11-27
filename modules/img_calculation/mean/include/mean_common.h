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

/** @brief Calculates an average (mean) of mat elements.

The function fcv::mean calculates the mean value of array elements,
independently for each channel.
@param src input array that should have from 1 to 4 channels so that the result can be stored in
Scalar.
*/
Scalar mean_common(const Mat& src);

/** @brief Calculates an average (mean) of mat elements.

The function fcv::mean calculates the mean value of array elements,
independently for each channel.
When all the mask elements are 0's, the function returns Scalar::all(0)
@param src input array that should have from 1 to 4 channels so that the result can be stored in
Scalar.
@param mask optional operation mask, if the mask is not empty, it has to be of type unsigned char and can have 1 or multiple channels.
*/
Scalar mean_common(const Mat& src, const Mat& mask);

/** @brief Calculates an average (mean) of mat elements in specific rectangle.

The function fcv::mean calculates the mean value of specific rectangle array elements,
independently for each channel.
@param src, input array that should have from 1 to 4 channels so that the result can be stored in
Scalar.
@param rect, the rectangle area to be calcutated, specific see Rect.
*/
Scalar mean_common(const Mat& src, const Rect& rect);


/** @brief Calculates an average (mean) and standard deviation of different data type of mat elements.

@param src input array that should have from 1 to 4 channels
@param sum the sum of input array elements value.
@param suqare_sum the sum of squares of input array elements value, a^2 + b^2 + c^2 +.....
@param len the num of input array elements, width * height * channel.
@param cn the number of the channels of input array, supported 1 to 4 currently.
*/

int sum_sqr_u8_common(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn);

int sum_sqr_u16_common(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn);

int sum_sqr_s32_common(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn);

int sum_sqr_f32_common(
        const void* src,
        double* sum,
        double* suqare_sum,
        int len,
        int cn);

void mean_stddev_common(const Mat& src, Mat& mean, Mat& stddev);

G_FCV_NAMESPACE1_END()
