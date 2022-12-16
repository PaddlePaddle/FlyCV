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

#include <vector>

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_export.h"
#include "modules/core/base/interface/macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Get the num of CPU cores.
 * @return the cpu nums
 */
EXTERN_C FCV_API int get_cpu_num();

/**
 * @brief Get the ordered cpu ids array.
 * @param[out] cpu_ids The array of CPU ids sorted by frequency, the front is high frequency.
 * @return the execution state of the function, zero means succeed, non-zero means failed.
 */
FCV_API int get_ordered_cpu_ids(std::vector<int>& cpu_ids);

/**
 * @brief Set the CPU affinity.
 * @param cpu_ids Expected CPU ids to set affinity.
 * @return the execution state of the function, zero means succeed, non-zero means failed. 
 */
FCV_API int set_sched_affinity(const std::vector<int>& cpu_ids);

G_FCV_NAMESPACE1_END()
