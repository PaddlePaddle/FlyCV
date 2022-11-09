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

#ifndef MARLIN_INCLUDE_CORE_RETURN_CODE_H
#define MARLIN_INCLUDE_CORE_RETURN_CODE_H

#include <memory>
#include <string>
#include <vector>

#include "modules/core/opencl/include/core/macro.h"
#include "modules/core/opencl/include/core/common.h"

#pragma warning(push)
#pragma warning(disable : 4251)

namespace ocl {

class PUBLIC Status {
public:
    ~Status();
    Status(int code = MARLIN_OK, const std::string& message = "OK");

    Status &operator=(int code);

    bool operator==(int code_) const;
    bool operator!=(int code_);
    operator int();
    operator bool();
    std::string description();

private:
    int code_ = 0;
    std::string message_ = "";
};

using DimsVector = std::vector<int>;

}  // namespace ocl

#pragma warning(pop)

#endif //MARLIN_INCLUDE_CORE_RETURN_CODE_H
