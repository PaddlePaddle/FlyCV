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

#include "modules/core/allocator/include/cpu_allocator.h"

#include <stdlib.h>
#include <cstdint>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

cpu_allocator::cpu_allocator(size_t size)
        : _data_addr(nullptr) {
    _data_addr = malloc(size);

    if (!_data_addr) {
#if (SIZE_MAX == 0xFFFFFFFF) /// 32bit
        LOG_ERR("Failed to allocate %zu bytes memory from CPU platform", size);
#elif (SIZE_MAX == 0xFFFFFFFFFFFFFFFF) /// 64bit
        LOG_ERR("Failed to allocate %zu bytes memory from CPU platform", size);
#else
        LOG_ERR("Failed to allocate %zu bytes memory from CPU platform", size);
#endif
    }
}

cpu_allocator::~cpu_allocator() {
    if (_data_addr) {
        free(_data_addr);
        _data_addr = nullptr;
    }
}

bool cpu_allocator::get_data(void** ptr_data_addr) const {
    if (ptr_data_addr) {
        *ptr_data_addr = _data_addr;
        return true;
    }
    return false;
}

bool cpu_allocator::get_data_complex(
        std::vector<uint64_t*>& phy_addrs,
        std::vector<uint64_t*>& vir_addrs) const {
    phy_addrs = _phy_addrs;
    vir_addrs = _vir_addrs;

    return true;
}

G_FCV_NAMESPACE1_END()
