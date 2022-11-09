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

#include <map>

#include "modules/core/opencl/include/opencl_runtime.h"

namespace ocl {

std::shared_ptr<OpenCLRuntime> OpenCLRuntime::opencl_runtime_singleton_ = nullptr;
extern const std::map<std::string, std::vector<unsigned char>> g_opencl_program_map;

static std::mutex g_mtx;

static std::map<int, int> AdrenoSubGroup{
    {640, 128}, {630, 128}, {616, 128}, {612, 64}, {610, 64}, {540, 32}, {530, 32},
    {512, 32}, {510, 32}, {509, 32}, {506, 32}, {505, 32}, {405, 32}, {330, 16},
};

bool OpenCLRuntime::enable_increase_count_ = false;
int OpenCLRuntime::ref_count_ = 0;
bool OpenCLRuntime::init_done_ = false;

OpenCLRuntime::OpenCLRuntime() {
    default_build_opts_ = "-cl-mad-enable -cl-fast-relaxed-math -Werror";
}

OpenCLRuntime *OpenCLRuntime::get_instance() {
    std::unique_lock<std::mutex> lck(g_mtx);
    if (nullptr == opencl_runtime_singleton_) {
        opencl_runtime_singleton_.reset(new OpenCLRuntime());
        ref_count_++;
    }
    return opencl_runtime_singleton_.get();
}

void OpenCLRuntime::increase_ref() {
    std::unique_lock<std::mutex> lck(g_mtx);
    if (enable_increase_count_) {
        ref_count_++;
    }
    enable_increase_count_ = true;
    LOGD("OpenCLRuntime::IncreaseRef() count=%d\n", ref_count_);
}

void OpenCLRuntime::decrease_ref() {
    std::unique_lock<std::mutex> lck(g_mtx);
    ref_count_--;
    if (0 == ref_count_) {
        opencl_runtime_singleton_.reset();
        init_done_ = false;
    }
    LOGD("OpenCLRuntime::DecreaseRef() count=%d\n", ref_count_);
}

Status OpenCLRuntime::init() {
    std::unique_lock<std::mutex> lck(g_mtx);

    if (!init_done_) {
        LOGD("Init OpenCL Runtime\n");
        LOGI(
            "OpenCL version : CL_TARGET_OPENCL_VERSION %d"
            "CL_HPP_TARGET_OPENCL_VERSION %d CL_HPP_MINIMUM_OPENCL_VERSION"
            "%d\n",
            CL_TARGET_OPENCL_VERSION, CL_HPP_TARGET_OPENCL_VERSION, CL_HPP_MINIMUM_OPENCL_VERSION);

        if (!OpenCLSymbols::get_instance()->load_opencl_library()) {
            return Status(MARLINERR_DEVICE_LIBRARY_LOAD, "load opencl library failed!");
        }

        std::vector<cl::Platform> platforms;
        cl::Platform::get(&platforms);
        if (platforms.empty()) {
            LOGE("OpenCL Platform not found!\n");
            return Status(MARLINERR_OPENCL_RUNTIME_ERROR, "OpenCL Platform not found!");
        }

        LOGD("find %u platforms\n", platforms.size());

        //search GPU
        std::vector<cl::Device> devices;
        for (auto &platform : platforms) {
            std::string platform_name;
            platform.getInfo(CL_PLATFORM_NAME, &platform_name);
            platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
            LOGD("platform (%s) has %u GPUs\n", platform_name.c_str(), devices.size());

            if (!devices.empty()) {
                std::string device_name = devices[0].getInfo<CL_DEVICE_NAME>();
                LOGD("find GPU: %s\n", device_name.c_str());
                cl::Platform::setDefault(platform);
                break;
            }
        }

        //not found
        if (devices.empty()) {
            LOGE("OpenCL Devcie not found!\n");
            return Status(MARLINERR_OPENCL_RUNTIME_ERROR, "OpenCL Device not found!");
        }

        device_ = std::make_shared<cl::Device>();
        *device_ = devices[0];
        const std::string device_name = device_->getInfo<CL_DEVICE_NAME>();
        const std::string device_version = device_->getInfo<CL_DEVICE_VERSION>();
        const std::string opencl_version = device_->getInfo<CL_DEVICE_OPENCL_C_VERSION>();
        LOGD("device name:\t%s\n", device_name.c_str());
        LOGD("opencl version:\t%s\n", device_version.c_str());
        LOGD("highest opencl c version:\t%s\n", opencl_version.c_str());

        gpu_info_ = parse_gpu_info(device_name, device_version);

        cl_int err;

        //create context
        context_ = std::shared_ptr<cl::Context>(new cl::Context(*device_, nullptr, nullptr, nullptr, &err));
        if (err != CL_SUCCESS) {
            LOGE("Context create failed! (ERROR CODE: %d)\n", err);
            return Status(MARLINERR_OPENCL_RUNTIME_ERROR, "Context create failed!");
        }

        //get cache size , compute units and frequency
        device_->getInfo(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, &global_memory_cachesize_);
        device_->getInfo(CL_DEVICE_MAX_COMPUTE_UNITS, &compute_units_);
        device_->getInfo(CL_DEVICE_MAX_CLOCK_FREQUENCY, &max_freq_);
        cl_device_fp_config fp_config;
        auto success = device_->getInfo(CL_DEVICE_HALF_FP_CONFIG, &fp_config);
        support_fp16_ = CL_SUCCESS == success && fp_config > 0;

        LOGD("Global Mem Cache Size: %d\n", (int) global_memory_cachesize_);
        LOGD("Compute Unit: %d\n", (int) compute_units_);
        LOGD("Clock Frequency: %d MHz\n", (int) max_freq_);
        init_done_ = true;
        LOGD("OpenCLRuntime init done!\n");
    }

    return MARLIN_OK;
}

cl::Context *OpenCLRuntime::get_context() {
    return context_.get();
}

cl::Device *OpenCLRuntime::get_device() {
    return device_.get();
}

uint64_t OpenCLRuntime::get_global_memory_cache_size() const {
    return global_memory_cachesize_;
}

uint32_t OpenCLRuntime::get_compute_units() const {
    return compute_units_;
}

uint32_t OpenCLRuntime::get_max_freq() const {
    return max_freq_;
}

uint64_t OpenCLRuntime::get_max_work_group_size(const cl::Kernel &kernel) {
    uint64_t max_workgroup_size = 0;
    int ret = kernel.getWorkGroupInfo(*device_, CL_KERNEL_WORK_GROUP_SIZE, &max_workgroup_size);
    if (ret != 0) {
        max_workgroup_size = 0;
    }
    return max_workgroup_size;
}

//opencl 2.0 can get SubGroupSize
uint32_t OpenCLRuntime::get_sub_group_size(const cl::Kernel &kernel, const cl::NDRange &range) {
    uint32_t sub_group_size = 0;

    if (AdrenoSubGroup.find(gpu_info_.model_num) != AdrenoSubGroup.end()) {
        sub_group_size = AdrenoSubGroup[gpu_info_.model_num];
    }
    return sub_group_size;
}

GpuInfo OpenCLRuntime::get_gpu_info() {
    return gpu_info_;
}

bool OpenCLRuntime::get_fp16_enable() const {
    return fp16_enable_;
}

bool OpenCLRuntime::set_fp16_enable(bool enable) {
    fp16_enable_ = enable && support_fp16_;
    return fp16_enable_ == enable;
}

void OpenCLRuntime::setPrecision(Precision precision) {
    precision_ = precision;
}

Status OpenCLRuntime::build_kernel(cl::Kernel &kernel, const std::string &program_name, const std::string &kernel_name,
                                  const std::set<std::string> &build_options) {
    std::string build_options_str;
    bool force_fp32 = false;
    auto it = build_options.find("-DFORCE_FP32");
    if (it != build_options.end()) {
        force_fp32 = true;
    }
    //set default macro
    if (fp16_enable_ && (PRECISION_LOW == precision_ || PRECISION_AUTO == precision_) && !force_fp32) {
        LOGD("OpenCL Calculate Precision is Half!\n");
        build_options_str =
            "-DFLOAT=half -DFLOAT4=half4 -DRI_F=read_imagef"
            "-DWI_F=write_imagef";
    }
    for (auto &option:build_options) {
        build_options_str + " " + option;
    }
    build_options_str += default_build_opts_;
    //program identifier = program_name + build_options
    std::string build_program_key = program_name + build_options_str;

    auto build_program_it = program_map_.find(build_program_key);
    cl::Program program;
    //if search program exist , then use it
    if (build_program_it != program_map_.end()) {
        LOGD("find program: %s\n", build_program_key.c_str());
        program = build_program_it->second;
    } else {
        //load program and build program
        LOGD("build program:%s\n", build_program_key.c_str());
        bool status = this->load_program(program_name, &program);
        if (!status) {
            LOGE("load program (%s) failed!\n", program_name.c_str());
            return Status(MARLINERR_OPENCL_KERNELBUILD_ERROR, "load program failed");
        }
        status = this->build_program(build_options_str, &program);
        if (!status) {
            LOGE("%s build failed!\n", program_name.c_str());
            return Status(MARLINERR_OPENCL_KERNELBUILD_ERROR, "build program failed");
        }
        program_map_.emplace(build_program_key, program);
    }

    LOGD("build kernel: %s\n", kernel_name.c_str());
    cl_int err;
    kernel = cl::Kernel(program, kernel_name.c_str(), &err);
    if (err != CL_SUCCESS) {
        LOGE("Kernel create failed! (ERROR CODE: %d)\n", err);
        return Status(MARLINERR_OPENCL_KERNELBUILD_ERROR, "create kernel failed");
    }
    return MARLIN_OK;
}

GpuInfo OpenCLRuntime::parse_gpu_info(const std::string &device_name, const std::string &device_version) {
    GpuInfo gpu_info;

    if (device_name == "QUALCOMM adreno(TM)") {
        LOGD("GPU type is ADRENO\n");
        gpu_info.type = ADRENO;
        sscanf(device_version.c_str(), "%*s%f%*s%d", &gpu_info.opencl_version, &gpu_info.model_num);
    } else if (device_name.find("Mali") != -1) {
        LOGD("GPU type is MALI\n");
        gpu_info.type = MALI;

        //Mali type MALI-G or MALI-T
        if (device_name.find("Mali-G") != -1) {
            LOGD("GPU type is Mali-G\n");
            gpu_info.type = MALI_G;
            sscanf(device_name.c_str(), "Mali-G%d", &gpu_info.model_num);
        } else if (device_name.find("Mali-T") != -1) {
            LOGD("GPU type is Mali-T\n");
            gpu_info.type = MALI_T;
            sscanf(device_name.c_str(), "Mali-T%d", &gpu_info.model_num);
        }
        sscanf(device_version.c_str(), "%*s%f%*s", &gpu_info.opencl_version);
    }
    LOGD("GPU Type: %d , model_num: %d, opencl version: %f\n", gpu_info.type, gpu_info.model_num, gpu_info.opencl_version);

    return gpu_info;
}

//load program with program name
bool OpenCLRuntime::load_program(const std::string &program_name, cl::Program *program) {
    auto it_source = g_opencl_program_map.find(program_name);
    if (it_source != g_opencl_program_map.end()) {
        cl::Program::Sources sources;
        std::string source(it_source->second.begin(), it_source->second.end());
        sources.push_back(source);
        *program = cl::Program(*get_context(), sources);
        return true;
    } else {
        LOGE("Can't find kernel source !\n");
        return false;
    }
}

bool OpenCLRuntime::build_program(const std::string &build_options, cl::Program *program) {
    cl_int ret = program->build({*device_}, build_options.c_str());
    if (ret != CL_SUCCESS) {
        if (program->getBuildInfo<CL_PROGRAM_BUILD_STATUS>(*device_) == CL_BUILD_ERROR) {
            std::string build_log = program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*device_);
            LOGE("Program build log: %s \n", build_log.c_str());
        }
        LOGE("Build program failed ! \n");
        return false;
    }
    return true;
}

OpenCLRuntime::~OpenCLRuntime() {
    LOGD("~OpenCLRuntime() start\n");
    program_map_.clear();
    context_.reset();
    device_.reset();
    LOGD("~OpenCLRuntime() end\n");
}

}   //namespace bd_marlin
