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

#include <mutex>
#include <memory>

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/parallel/include/parallel_handler.h"

// todo: use macro to control
#include "modules/core/parallel/include/parallel_handler_thread.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief   paralell manager
 */
class ParallelManager {
public:
    static ParallelManager& get_instance() {
        std::call_once(first_init_flag, [] {
            _instance->create_handler();
        });

        return *_instance;
    }

    int create_handler();

    int destroy_handler();

    void set_thread_num(int thread_num);

    int get_thread_num();

    int run(const Range& range, const ParallelTask& task, int nstripes);

    ~ParallelManager() = default;

private:
    ParallelManager() = default;

private:
    static std::once_flag first_init_flag;
    static std::unique_ptr<ParallelManager> _instance;

    std::shared_ptr<ParallelHandler> _parallel_hld;
};

G_FCV_NAMESPACE1_END()
