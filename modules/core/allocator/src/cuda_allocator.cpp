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

#include "modules/core/allocator/include/cuda_allocator.h"

#include <assert.h>
#include <stdlib.h>

#include <atomic>
#include <cstdint>
#include <forward_list>
#include <mutex>
#include <unordered_map>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class CUDAMemoryPool;

namespace {
class CUDAMemoryManager {
public:
    CUDAMemoryManager() {}
    ~CUDAMemoryManager() {}

    // 根据size, Pool ID, type创建不同的内存池, TODO
    void create_memorypool(int pool_id, size_t size, CUDAMemoryType type) {}

    // 根据Pool ID销毁对应内存池, TODO
    void destroy_memorypool(int pool_id) {}

    // 根据Pool ID获取对应内存池实例, TODO
    CUDAMemoryPool* get_memorypool(int pool_id) { return nullptr; }

private:
    size_t _all_size;
    std::unordered_map<int, CUDAMemoryPool*> _pools;
};
}  // namespace

// global handle of CUDA Memory Pool.
std::unique_ptr<CUDAMemoryManager> memory_pool_ptr(new CUDAMemoryManager);

namespace {

class CUDAMemoryPool {
public:
    // TODO
    CUDAMemoryPool()
        : data_start(nullptr),
          data_end(nullptr),
          data_sp(nullptr),
          manager(nullptr) {}
    virtual ~CUDAMemoryPool() {}

    CUDAMemoryPool(const CUDAMemoryPool&) = delete;
    CUDAMemoryPool& operator=(const CUDAMemoryPool&) = delete;

    // 创建内存池CUDA内存
    virtual void create(size_t size) = 0;
    // 销毁内存池CUDA内存
    virtual void destroy() = 0;
    // 申请内存池CUDA内存, TODO
    virtual uint8_t* request_memory(size_t size) { return nullptr; }
    // 归还内存池CUDA内存, TODO
    virtual void return_memory(uint8_t* ptr) {}

    uint8_t* data_start;
    uint8_t* data_end;
    uint8_t* data_sp;

    CUDAMemoryManager* manager;

private:
    std::mutex _mutex;
};

class CUDAGeneralMemoryPool : public CUDAMemoryPool {
public:
    CUDAGeneralMemoryPool() {}
    virtual ~CUDAGeneralMemoryPool() {}

    CUDAGeneralMemoryPool(const CUDAGeneralMemoryPool&) = delete;
    CUDAGeneralMemoryPool& operator=(const CUDAGeneralMemoryPool&) = delete;

    // 创建内存池CUDA内存, TODO
    virtual void create(size_t size) override {}
    // 销毁内存池CUDA内存, TODO
    virtual void destroy() override {}
};

class CUDAUnifiedMemoryPool : public CUDAMemoryPool {
public:
    CUDAUnifiedMemoryPool() {}
    virtual ~CUDAUnifiedMemoryPool() {}

    CUDAUnifiedMemoryPool(const CUDAUnifiedMemoryPool&) = delete;
    CUDAUnifiedMemoryPool& operator=(const CUDAUnifiedMemoryPool&) = delete;

    // 创建内存池CUDA内存, TODO
    virtual void create(size_t size) override {}
    // 销毁内存池CUDA内存, TODO
    virtual void destroy() override {}
};

}  // namespace

CUDAAllocator::CUDAAllocator(size_t size) : _data_addr(nullptr) {
    cudaMalloc(&_data_addr, size);

    if (!_data_addr) {
#if (SIZE_MAX == 0xFFFFFFFF)  /// 32bit
        LOG_ERR("Failed to allocate %u bytes memory from GPU platform", size);
#elif (SIZE_MAX == 0xFFFFFFFFFFFFFFFF)  /// 64bit
        LOG_ERR("Failed to allocate %zu bytes memory from GPU platform", size);
#else
        LOG_ERR("Failed to allocate %d bytes memory from GPU platform", size);
#endif
        return;
    }
}

CUDAAllocator::~CUDAAllocator() {
    if (_data_addr) {
        cudaFree(_data_addr);
        _data_addr = nullptr;
    }
}

bool CUDAAllocator::get_data(void** ptr_data_addr) const {
    if (ptr_data_addr) {
        *ptr_data_addr = _data_addr;
        return true;
    }
    return false;
}

G_FCV_NAMESPACE1_END()
