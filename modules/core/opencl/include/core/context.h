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

#ifndef MARLIN_INCLUDE_CORE_CONTEXT_H
#define MARLIN_INCLUDE_CORE_CONTEXT_H

#include "modules/core/opencl/include/core/common.h"
#include "modules/core/opencl/include/core/sign.h"

namespace ocl {
class Context {
  public:
    virtual ~Context() {}

    virtual Status load_library(std::vector<std::string> path) = 0;

    virtual Status get_command_queue(void **command_queue) = 0;

    virtual Precision get_precision();

    virtual Status set_precision(Precision precision);

  protected:
    Precision precision_ = PRECISION_AUTO;
};
}
#endif //MARLIN_INCLUDE_CORE_CONTEXT_H
