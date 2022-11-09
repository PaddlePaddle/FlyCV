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
 * @brief Calculates an average (mean) of mat elements. The function fcv::mean calculates the mean value of array elements,
 * independently for each channel.
 * @param[in] src input array that should have from 1 to 4 channels so that the result can be stored in
 * Scalar.
*/
FCV_API Scalar mean(const Mat& src);

/** 
 * @brief Calculates an average (mean) of mat elements.
 * The function fcv::mean calculates the mean value of array elements, independently for each channel.
 * When all the mask elements are 0's, the function returns Scalar::all(0)
 * @param[in] src input array that should have from 1 to 4 channels so that the result can be stored in
 * Scalar.
 * @param[in] mask optional operation mask, if the mask is not empty, it has to be of type unsigned char and can have 1 or multiple channels.
*/
FCV_API Scalar mean(const Mat& src, const Mat& mask);

/** 
 * @brief Calculates an average (mean) of mat elements in specific rectangle.
 * The function fcv::mean calculates the mean value of specific rectangle array elements, independently for each channel.
 * @param[in] src input array that should have from 1 to 4 channels so that the result can be stored in
 * Scalar.
 * @param[in] rect the rectangle area to be calcutated, specific see Rect.
*/
FCV_API Scalar mean(const Mat& src, const Rect& rect);

/** 
 * @brief Calculates an average (mean) and standard deviation of mat elements.
 * The function fcv::mean calculates the mean value and standard deviation of array elements
 * independently for each channel.
 * @param[in] src input array that should have from 1 to 4 channels
 * @param[out] mean output that store calculated mean value.
 * @param[out] stddev output that store calculated standard deviation.
 * @param mask, optional operation mask, if the mask is not empty, it has to be of type unsigned char and can have 1 or multiple channels.
*/
FCV_API void mean_stddev(const Mat& src, Mat& mean, Mat& stddev);

G_FCV_NAMESPACE1_END()
