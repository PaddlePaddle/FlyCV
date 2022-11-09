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

#pragma once

#include <memory>
#include <vector>

#include "modules/core/base/interface/basic_types.h"
#include "modules/core/base/interface/log.h"
#include "modules/core/base/interface/macro_export.h"
#include "modules/core/base/interface/macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief class for base allocator
 */
class FCV_API BaseAllocator {
public:
    BaseAllocator() {}
    virtual ~BaseAllocator() {}

    /**
     * @brief Get the allocated data address，return false on failure
     * @param[in] ptr_data_addr，points to an allocated address
     */
    virtual bool get_data(void** ptr_data_addr) const = 0;
    virtual bool get_data_complex(
            std::vector<uint64_t*>& phy_addrs,
            std::vector<uint64_t*>& vir_addrs) const = 0;
};

/**
 * @brief the implementation of allocating platform data
 * @param[in] size，input data size
 * @param[in] platform, input computing platform，supported cpu
 */
FCV_API std::shared_ptr<BaseAllocator> get_allocator_from_platform(
        size_t size,
        PlatformType platform);

G_FCV_NAMESPACE1_END()
