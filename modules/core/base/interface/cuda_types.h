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
#include <cstdio>
#include <cstdlib>

#include <memory>

#include "flycv_namespace.h"
#include "macro_export.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define CHECK(call)                                                         \
    do {                                                                    \
        const cudaError_t error_code = call;                                \
        if (error_code != cudaSuccess) {                                    \
            printf("CUDA Error:\n");                                        \
            printf("    File:       %s\n", __FILE__);                       \
            printf("    Line:       %d\n", __LINE__);                       \
            printf("    Error code: %d\n", error_code);                     \
            printf("    Error text: %s\n", cudaGetErrorString(error_code)); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

/**
 * @brief cuda memory types.
 */
enum class CUDAMemoryType {
    GENERAL,
    UNIFIED,
    SHARED,
    UNDEFINED
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

class FCV_CLASS Stream {
public:
    Stream() : _stream(0) {
        cudaStreamCreate(&_stream);
        CHECK(cudaGetLastError());
    }

    ~Stream() {
        if (_stream) {
            cudaStreamDestroy(_stream);
        }
    }

    static Stream& Null() {
        static Stream stream(0);
        return stream;
    }

private:
    cudaStream_t _stream;

private:
    Stream(cudaStream_t st) : _stream(st) {}
};

G_FCV_NAMESPACE1_END()
