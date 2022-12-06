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

#include <cuda_runtime.h>

#include "flycv_namespace.h"
#include "macro_export.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief cuda memory types.
 */
enum class CUDAMemoryType {
    GENERAL,
    UNIFIED
};

/**
 * @brief cuda device info.
 */
struct CUDADeviceInfo {
    //! GUDA Driver Version
    int driver_version;
    //! GUDA Runtime Version
    int runtime_version;
    //! CUDA Device Property
    cudaDeviceProp device_prop;
};

G_FCV_NAMESPACE1_END()
