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

#include "modules/core/base/interface/cuda_types.h"
#include "modules/core/allocator/interface/base_allocator.h"

#include <vector>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class CUDAAllocator : public BaseAllocator {
public:
    CUDAAllocator(size_t elem_size);
    CUDAAllocator(int rows, int cols);
    ~CUDAAllocator();

    bool get_data(void** ptr_data_addr) const override;

private:
    size_t _size;
    void* _data_addr;
};

class CUDAPoolAllocator : public BaseAllocator {
public:
    CUDAPoolAllocator(int pool_id, size_t elem_size);
    CUDAPoolAllocator(int pool_id, int rows, int cols);
    ~CUDAPoolAllocator();

    bool get_data(void** ptr_data_addr) const override;

private:
    int _pool_id;
    size_t _size;
    void* _data_addr;
};

//! BufferPool management (must be called before Stream creation)
// FCV_API void setCUDAPoolUsage(bool on);
FCV_API void activate_cuda_pool(int pool_id, size_t size, CUDAMemoryType type);
FCV_API size_t check_cuda_pool_left(int pool_id);
FCV_API void deactivate_cuda_pool(int pool_id);

G_FCV_NAMESPACE1_END()
