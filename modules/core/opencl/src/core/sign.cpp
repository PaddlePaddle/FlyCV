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

#include "modules/core/opencl/include/core/sign.h"

#include <iomanip>
#include <sstream>

namespace ocl {
std::string get_status_message(int code) {
    switch (code) {
    case MARLINERR_INVALID_NETCFG:
        return "invalid net config , protocol or model is invalid";
    case MARLINERR_SET_CPU_AFFINITY:
        return "failed to set cpu affinity";
    case MARLINERR_DEVICE_NOT_SUPPORT:
        return "device is nil or not support";
    case MARLINERR_DEVICE_CONTEXT_CREATE:
        return "context is nil or created failed";
    default:
        return "default";
    }
}

Status::~Status() {
    code_ = 0;
    message_ = "";
}

Status::Status(int code, const std::string &message) {
    code_ = code;
    message_ = (message != "OK" && message.length() > 0) ? message : get_status_message(code);
}

Status &Status::operator=(int code) {
    code_ = code;
    message_ = get_status_message(code);
    return *this;
}

bool Status::operator==(int code) const {
    return code_ == code;
}

bool Status::operator!=(int code) {
    return code_ != code;
}

Status::operator int() {
    return code_;
}

Status::operator bool() {
    return code_ == MARLIN_OK;
}

std::string Status::description() {
    std::ostringstream os;
    os << "code: 0x" << std::uppercase << std::setfill('0') << std::setw(4) << std::hex <<
       code_ << " msg: " << message_;
    return os.str();
}
}
