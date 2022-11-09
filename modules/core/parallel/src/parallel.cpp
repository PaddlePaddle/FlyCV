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

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/parallel/include/parallel_manager.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

void set_thread_num(int num) {
    ParallelManager::get_instance().set_thread_num(num);
}

int get_thread_num() {
    return ParallelManager::get_instance().get_thread_num();
}

int parallel_run(
        const Range& range,
        const ParallelTask& task,
        int nstripes) {
    if (range.end() <= range.start()) {
        return -1;
    }

    return ParallelManager::get_instance().run(range, task, nstripes);
}

G_FCV_NAMESPACE1_END()
