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

#include "modules/core/opencl/interface/opencl.h"
#include "modules/core/base/interface/log.h"
#include "modules/core/opencl/include/opencl_context.h"

namespace ocl {

bool is_use_opencl = false;
bool init_success_;
ocl::OpenCLContext *context_ = nullptr;

int setUseOpenCL(bool flag) {
    is_use_opencl = flag;
    return 0;
}

bool isUseOpenCL() {
    return is_use_opencl;
}

bool init(void **command_queue) {

    if (init_success_) {
        context_->get_command_queue(command_queue);
        return init_success_;
    }

    context_ = new ocl::OpenCLContext();

    std::vector <std::string> library_path = {};
    ocl::Status ret = context_->load_library(library_path);
    if (ret != ocl::MARLIN_OK) {
        LOGE("%s", ret.description().c_str());
        return false;
    }

    context_->get_command_queue(command_queue);
    if (ret != ocl::MARLIN_OK) {
        LOGE("%s", ret.description().c_str());
        return false;
    }

    init_success_ = true;
    return init_success_;
}

void close() {
    init_success_ = false;
    if (context_ != nullptr) {
        delete context_;
    }
}

}