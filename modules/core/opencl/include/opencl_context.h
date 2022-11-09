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

#ifndef MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_CONTEXT_H_
#define MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_CONTEXT_H_

#include "modules/core/opencl/include/core/context.h"
#include "modules/core/opencl/include/opencl_runtime.h"

namespace ocl {

class OpenCLContext : public Context {
public:
    OpenCLContext();
    ~OpenCLContext();

    Status get_command_queue(void **command_queue) override;

    cl::CommandQueue *command_queue();

    virtual Status load_library(std::vector<std::string> path) override;

public:
    Status init();

private:
    std::shared_ptr<cl::CommandQueue> command_queue_ = nullptr;
    OpenCLRuntime *opencl_runtime_ = nullptr;
};

}   //namespace ocl

#endif //MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_CONTEXT_H_
