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

#include "modules/core/cmat/interface/cmat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/** 
 * @brief Calculates an average (mean) of mat elements.
 * The function calculates the mean value of array elements
 * independently for each channel.
 * @param[in] src input array that should have from 1 to 4
 * channels so that the result can be stored in
 * @param[out] scalar pointer to CScalar instance.
*/
EXTERN_C FCV_API CScalar fcvMean(CMat* src);

/** 
 * @brief Calculates an average (mean) and standard deviation of cmat elements.
 * @param[in] src input array that should have from 1 to 4 channels
 * @param[out] mean output that store calculated mean value.
 * @param[out] stddev output that store calculated standard deviation.
 * it has to be of type unsigned char and can have 1 or multiple channels.
*/
EXTERN_C FCV_API int fcvMeanStddev(CMat* src, CMat* mean, CMat* stddev);

G_FCV_NAMESPACE1_END()