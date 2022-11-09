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

#include "modules/core/parallel/include/parallel_manager.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

std::once_flag ParallelManager::first_init_flag;
std::unique_ptr<ParallelManager> ParallelManager::_instance(new ParallelManager);

int ParallelManager::create_handler() {
    // todo: add more paralell handler in the future
    _parallel_hld = std::make_shared<ThreadParallelHandler>();
    return 0;
}

int ParallelManager::destroy_handler() {
    return 0;
}

void ParallelManager::set_thread_num(int thread_num) {
    _parallel_hld->set_thread_num(thread_num);
}

int ParallelManager::get_thread_num() {
    return _parallel_hld->get_thread_num();
}

int ParallelManager::run(
        const Range& range,
        const ParallelTask& task,
        int nstripes) {
    _parallel_hld->run(range, task, nstripes);
    return 0;
}

G_FCV_NAMESPACE1_END()
