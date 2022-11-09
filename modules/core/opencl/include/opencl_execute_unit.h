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

#ifndef MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_EXECUTE_UNIT_H_
#define MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_EXECUTE_UNIT_H_

#include "modules/core/opencl/include/opencl_runtime.h"

namespace ocl {

struct OpenCLExecuteUnit {
    cl::Kernel ocl_kernel;
    uint32_t work_group_size_max = 0;
    std::vector <uint32_t> global_work_size = {};
    std::vector <uint32_t> local_work_size = {};
    uint32_t sub_group_size = 0;
};
}

#endif //MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_EXECUTE_UNIT_H_
