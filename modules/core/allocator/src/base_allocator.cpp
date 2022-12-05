// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include "modules/core/allocator/interface/base_allocator.h"
#include "modules/core/allocator/include/cpu_allocator.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

std::shared_ptr<BaseAllocator> get_allocator_from_platform(
        size_t size,
        PlatformType platform,
        int flag) {
    std::shared_ptr<BaseAllocator> result = nullptr;

    switch (platform) {
    case PlatformType::CPU:
        result = std::make_shared<cpu_allocator>(size);
        break;
    default:
        break;
    };

    return result;
}

G_FCV_NAMESPACE1_END()
