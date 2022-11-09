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

#ifndef MARLIN_INCLUDE_OPENCL_RUNTIME_H
#define MARLIN_INCLUDE_OPENCL_RUNTIME_H

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include "modules/core/opencl/include/core/sign.h"
#include "modules/core/opencl/include/core/common.h"
#include "modules/core/opencl/include/opencl_wrapper.h"

namespace ocl {
enum GpuType {
    OTHER = 0,
    MALI = 1,
    MALI_T = 2,
    MALI_G = 3,
    ADRENO = 4,
};

struct GpuInfo {
    GpuType type = OTHER;
    int model_num = 0;
    float opencl_version = 0;
};

const int g_base_gpu_mem_cachesize = 16384;

class OpenCLRuntime {
public:
    static OpenCLRuntime *get_instance();
    static void increase_ref();
    static void decrease_ref();

    ~OpenCLRuntime();
    OpenCLRuntime(const OpenCLRuntime &) = delete;
    OpenCLRuntime &operator=(const OpenCLRuntime &) = delete;

    Status init();
    cl::Context *get_context();
    cl::Device *get_device();
    uint64_t get_global_memory_cache_size() const;
    uint32_t get_compute_units() const;
    uint32_t get_max_freq() const;
    uint64_t get_max_work_group_size(const cl::Kernel &kernel);
    uint32_t get_sub_group_size(const cl::Kernel &kernel, const cl::NDRange &range = cl::NullRange);
    GpuInfo get_gpu_info();
    bool get_fp16_enable() const;
    bool set_fp16_enable(bool enable);
    void setPrecision(Precision precision);

    Status build_kernel(cl::Kernel &kernel, const std::string &program_name, const std::string &kernel_name,
                       const std::set<std::string> &build_options);

private:
    OpenCLRuntime();
    static GpuInfo parse_gpu_info(const std::string& device_name, const std::string& device_version);

    bool load_program(const std::string &program_name, cl::Program *program);
    bool build_program(const std::string &build_options, cl::Program *program);

private:
    static std::shared_ptr<OpenCLRuntime> opencl_runtime_singleton_;
    static bool enable_increase_count_;
    static int ref_count_;
    static bool init_done_;

    std::shared_ptr<cl::Context> context_ = nullptr;
    std::shared_ptr<cl::Device> device_ = nullptr;
    std::map<std::string, cl::Program> program_map_ = {};
    uint64_t global_memory_cachesize_ = 0;
    uint32_t compute_units_ = 0;
    uint32_t max_freq_ = 0;
    std::string default_build_opts_ = "";
    GpuInfo gpu_info_;
    bool support_fp16_ = false;
    bool fp16_enable_ = false;
    Precision precision_ = PRECISION_AUTO;
};
}   //namespace ocl

#endif //MARLIN_INCLUDE_OPENCL_RUNTIME_H
