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

#ifndef WIN32
#include <unistd.h>
#endif

#include <thread>
#include <queue>
#include <mutex>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <condition_variable>

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/macro_utils.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class WorkerThread;
class ParallelJob;

class ThreadPool {
public:
    static ThreadPool& get_instance() {
        static ThreadPool* instance = new ThreadPool;
        return *instance;
    }

    void run(const Range& range, const ParallelTask& body, int nstripes);
    unsigned int get_thread_num() const;
    void set_thread_num(unsigned int num);

    void stop() {
        _set_work_threads(0);
    }

    ThreadPool();
    ~ThreadPool();

private:
    bool _set_work_threads(unsigned int num);

public:
    std::mutex mutex_notify;
    std::condition_variable job_complete;

private:
    std::mutex _pool_mutex;
    unsigned int _thread_num;
    std::vector<std::shared_ptr<WorkerThread>> _work_threads;
    std::shared_ptr<ParallelJob> _job;
    const int _active_wait;
};

class WorkerThread {
public:
    WorkerThread(ThreadPool& thread_pool, unsigned int thread_id)
            : awake(false),
            is_active(true),
            _thread_pool(thread_pool),
            _thread_id(thread_id),
            _stop(false),
            _active_wait(2000) {
        UN_USED(_thread_id);
        _thread = std::thread([](void* work_thread) {
            ((WorkerThread*)work_thread)->loop_body();
        }, this);
    }

    ~WorkerThread() {
        if (!_stop) {
            std::unique_lock<std::mutex> lock(thread_mutex);
            _stop = true;
            lock.unlock();
        }

        if (_thread.joinable()) {
            _thread.join();
        }
    }

    void stop() {
        _stop = true;
    }

    void loop_body();

public:
    std::atomic<bool> awake;
    std::shared_ptr<ParallelJob> job;
    std::condition_variable cond_thread_wake;
    volatile bool is_active;
    std::mutex thread_mutex;

private:
    ThreadPool& _thread_pool;
    const unsigned int _thread_id;
    std::thread _thread;
    std::atomic<bool> _stop;
    const int _active_wait;
};

G_FCV_NAMESPACE1_END()