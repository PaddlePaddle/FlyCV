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

#include <algorithm>

#include "modules/core/opencl/include/opencl_memory_manager.h"
#include "modules/core/opencl/include/opencl_runtime.h"

namespace ocl {

std::shared_ptr<OpenCLMemoryManager> OpenCLMemoryManager::opencl_memory_manager_singleton_ = nullptr;
static std::mutex g_mtx;

OpenCLMemoryManager *OpenCLMemoryManager::get_instance() {
    std::unique_lock<std::mutex> lck(g_mtx);
    if (nullptr == opencl_memory_manager_singleton_) {
        opencl_memory_manager_singleton_.reset(new OpenCLMemoryManager());
    }
    return opencl_memory_manager_singleton_.get();
}

OpenCLMemoryInfo *OpenCLMemoryManager::obtain_buffer(uint32_t size) {
    return OpenCLMemoryManager::obtain_buffer(size, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR, nullptr);
}

OpenCLMemoryInfo *OpenCLMemoryManager::obtain_buffer(uint32_t size, cl_mem_flags flags, void *host_ptr) {
    std::unique_lock<std::mutex> lck(g_mtx);
    for (const auto &buffer_ptr:buffer_pool_) {
        if ((!buffer_ptr->is_busy) && flags == buffer_ptr->flags && buffer_ptr->size >= size) {
            sort_vector_.push_back(buffer_ptr);
        }
    }
    if (!sort_vector_.empty()) {
        std::sort(sort_vector_.begin(), sort_vector_.end(),
                  [](std::shared_ptr<OpenCLBufferInfo> ptr1, std::shared_ptr<OpenCLBufferInfo> ptr2) { return ptr1->size < ptr2->size; });
        OpenCLMemoryInfo *info = sort_vector_.front().get();
        info->is_busy = true;
#ifdef DEBUG
        info->obtain_times++;
        LOGD("reuse : %llu \n , buffer pool size : %d ", info->obtain_times, buffer_pool_.size());
#endif
        sort_vector_.clear();
        return info;
    }

    //create a new one
    cl_int ret = CL_SUCCESS;
    auto *cl_buffer = new cl::Buffer(*opencl_runtime_->get_context(), flags, size, host_ptr, &ret);
    if (ret != CL_SUCCESS) {
        CHECK_CL_SUCCESS(ret)
        delete cl_buffer;
    }

    auto *info = new OpenCLBufferInfo();
    info->cl_buffer = cl_buffer;
    info->size = size;
    info->flags = flags;
    info->is_busy = true;
    buffer_pool_.push_back(std::shared_ptr<OpenCLBufferInfo>(info));

    LOGD("Create Buffer at Buffer Pool , Buffer Size : %d", info->size);
    return info;
}

OpenCLMemoryManager::OpenCLMemoryManager() {
    opencl_runtime_ = OpenCLRuntime::get_instance();
}

OpenCLMemoryManager::~OpenCLMemoryManager() {
    buffer_pool_.clear();
    image_pool_.clear();
}

}
