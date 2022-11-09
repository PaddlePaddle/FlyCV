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

#include <vector>

#include "modules/core/allocator/interface/base_allocator.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class cpu_allocator : public BaseAllocator {
public:
    cpu_allocator(size_t size);
    ~cpu_allocator() override;

    bool get_data(void** ptr_data_addr) const override;
    bool get_data_complex(
            std::vector<uint64_t*>& phy_addrs,
            std::vector<uint64_t*>& vir_addrs) const override;

private:
    void* _data_addr;
    std::vector<uint64_t*> _phy_addrs;
    std::vector<uint64_t*> _vir_addrs;
};

G_FCV_NAMESPACE1_END()
