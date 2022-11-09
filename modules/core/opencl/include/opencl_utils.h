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

#ifndef MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_UTILS_H_
#define MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_UTILS_H_

#include <string>
#include <vector>

#include "modules/core/opencl/include/opencl_context.h"
#include "modules/core/opencl/include/opencl_execute_unit.h"
#include "modules/core/opencl/include/opencl_runtime.h"
#include "modules/core/opencl/include/core/sign.h"

namespace ocl {

Status run_kernel(const cl::Kernel &kernel, const std::vector<uint32_t> &gws, const std::vector<uint32_t> &lws,
                 cl::CommandQueue *command_queue, std::string name = "");

std::vector<uint32_t> adreno_local_size2d(const std::vector<uint32_t> &gws, const GpuInfo gpu_info,
                                          const uint32_t compute_units, const uint32_t max_workgroup_size,
                                          const uint32_t subgroup_size);

Status adjust_build_option_for_fp32(std::set<std::string> &build_options);

std::vector<uint32_t> local_ws3d_default(OpenCLExecuteUnit &unit);

std::vector<uint32_t> local_ws3d_default(const std::vector<uint32_t> &gws, const uint32_t max_workgroup_size,
                                         const uint32_t subgroup_size = 0);

std::vector<uint32_t> local_ws2d_default(OpenCLExecuteUnit &unit);

std::vector<uint32_t> local_ws2d_default(const std::vector<uint32_t> &gws, const uint32_t max_workgroup_sze,
                                         const uint32_t subgroup_size = 0);

Status create_execute_unit(OpenCLExecuteUnit &unit, const std::string &program_name, const std::string &kernel_name,
                         const std::set<std::string> &build_opt = {});

uint32_t set_execute_unit3d_size_info_default(OpenCLExecuteUnit &unit, DimsVector dims);

uint32_t set_execute_unit2d_size_info_default(OpenCLExecuteUnit &unit, DimsVector dims);

}   //namespace ocl

#endif //MARLIN_INCLUDE_BACKEND_OPENCL_OPENCL_UTILS_H_
