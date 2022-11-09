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

#ifndef MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_MEMORY_MANAGER_H_
#define MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_MEMORY_MANAGER_H_

#include "modules/core/opencl/include/opencl_runtime.h"
#include "modules/core/opencl/include/abstract_memory.h"

namespace ocl {

struct OpenCLMemoryInfo {
    cl_mem_flags flags;
    uint32_t size;
#ifdef DEBUG
    uint64_t obtain_times = 0;
#endif
    bool is_busy = false;
};

struct OpenCLBufferInfo : OpenCLMemoryInfo {
    cl::Buffer *cl_buffer;
};

struct OpenCLImageInfo : OpenCLMemoryInfo {
    cl::Image *cl_image;
};

class OpenCLMemoryManager : public AbstractMemoryManager {
public:
    static OpenCLMemoryManager *get_instance();
    OpenCLMemoryInfo *obtain_buffer(uint32_t size);
    OpenCLMemoryInfo *obtain_buffer(uint32_t size, cl_mem_flags flags, void *host_ptr);
    virtual ~OpenCLMemoryManager();
private:
    OpenCLMemoryManager();
    static std::shared_ptr<OpenCLMemoryManager> opencl_memory_manager_singleton_;
    OpenCLRuntime *opencl_runtime_;
    std::vector<std::shared_ptr<OpenCLBufferInfo>> buffer_pool_;
    std::vector<std::shared_ptr<OpenCLImageInfo>> image_pool_;
    std::vector<std::shared_ptr<OpenCLBufferInfo>> sort_vector_ = {};
};
}

#endif //MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_MEMORY_MANAGER_H_
