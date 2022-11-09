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

#pragma once

#include "flycv_namespace.h"
#include "modules/core/mat/interface/mat.h"
#include "modules/img_transform/color_convert/interface/color_convert.h"

namespace ocl {

/**
 * @brief the implementation with or without OpenCL,supported element types:bool
 * @param[in] flag , The flag is true to turn on OpenCL, the flag is false to turn off OpenCL
 */
int setUseOpenCL(bool flag);

/**
 * @brief the implementation of using OpenCL states
 */
bool isUseOpenCL() ;

/**
 * @brief the implementation with initialization with non-empty OpenCL queue
 * @param[in] command_queue , non-empty OpenCL queue for provided context and device
 */
bool init(void **command_queue);

void close();
}