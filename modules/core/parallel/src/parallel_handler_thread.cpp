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

#include "modules/core/parallel/include/parallel_handler_thread.h"

#include <future>

#include "modules/core/parallel/include/thread_pool.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int ThreadParallelHandler::get_thread_num() const {
    return ThreadPool::get_instance().get_thread_num();
}

void ThreadParallelHandler::set_thread_num(int num) {
    ThreadPool::get_instance().set_thread_num(num);
}

size_t ThreadParallelHandler::get_thread_id() const {
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

int ThreadParallelHandler::run(
        const Range& range,
        const ParallelTask& task,
        int nstripes) {
    ThreadPool::get_instance().run(range, task, nstripes);
    return 0;
}

G_FCV_NAMESPACE1_END()
