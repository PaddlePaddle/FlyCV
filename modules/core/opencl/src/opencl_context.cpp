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

#include "modules/core/opencl/include/opencl_context.h"

namespace ocl {

OpenCLContext::OpenCLContext() : Context() {
    opencl_runtime_ = OpenCLRuntime::get_instance();
    OpenCLRuntime::increase_ref();
}

OpenCLContext::~OpenCLContext() {
    command_queue_.reset();
    OpenCLRuntime::decrease_ref();
}

Status OpenCLContext::get_command_queue(void **command_queue) {
    *command_queue = command_queue_.get();
    return MARLIN_OK;
}

cl::CommandQueue *OpenCLContext::command_queue() {
    return command_queue_.get();
}

Status OpenCLContext::load_library(std::vector<std::string> path) {
    return init();
}

Status OpenCLContext::init() {
    if (opencl_runtime_ == nullptr) {
        return Status(MARLINERR_OPENCL_RUNTIME_ERROR, "opencl_runtime is nullptr");
    }
    Status status = opencl_runtime_->init();
    if (status != MARLIN_OK) {
        LOGE("OpenCL Runtime Init() failed (ret = %d)!\n", (int) status);
        return status;
    }

    cl_command_queue_properties properties = 0;

    cl_int err;
    command_queue_ = std::make_shared<cl::CommandQueue>(*opencl_runtime_->get_context(),
                                                        *opencl_runtime_->get_device(), properties, &err);
    if (err != CL_SUCCESS) {
        LOGE("Command Queue create failed ! (ERROR CODE: %d)\n", err);
        return Status(MARLINERR_DEVICE_CONTEXT_CREATE, "Command Queue create failed!");
    }

    opencl_runtime_->setPrecision(precision_);
    LOGD("opencl set precision %d\n", precision_);

#ifdef OPENCL_FORCE_FP16
    bool ret = opencl_runtime_->set_fp16_enable(true);
    if (!ret) {
        LOGE("disable fp 16 failed!\n");
    } else {
        LOGD("force fp16 success!\n");
    }
#else
    bool ret = opencl_runtime_->set_fp16_enable(false);
    if (!ret) {
        LOGE("enable fp 16 failed!\n");
    } else {
        LOGD("enable fp 32 success!\n");
    }
#endif

    return MARLIN_OK;
}

}   //namespace bd_marlin
