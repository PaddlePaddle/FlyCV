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

#include "modules/core/parallel/include/thread_pool.h"

#include <thread>

#include "flycv_namespace.h"
#include "modules/core/basic_math/interface/basic_math.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/cpu/interface/cpu.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class ParallelJob {
public:
    ParallelJob(
            const ThreadPool& thread_pool,
            const Range& range,
            const ParallelTask& body,
            int nstripes)
            : cur_position(0),
            active_thread_num(0),
            completed_thread_num(0), 
            completed(false),
            _thread_pool(thread_pool),
            _range(range),
            _body(body),
            _nstripes(nstripes) {}

    int run() {
        int total_cnt = _range.size();
        int step = 1;

        if (_nstripes <= 1) {
            step = (total_cnt - cur_position) / (_thread_pool.get_thread_num() * 2);
        } else {
            step = _nstripes;
        }

        for (;;) {
            int start = cur_position.fetch_add(step, std::memory_order_seq_cst);
            if (start >= total_cnt) {
                break;
            }

            int end = std::min(total_cnt, start + step);

            _body(Range(_range.start() + start, _range.start() + end));
        }

        return 0;
    }

    const Range& range() {
        return _range;
    }

    ~ParallelJob() {}

    std::atomic<int> cur_position;
    int64_t placeholder1[8];

    std::atomic<int> active_thread_num;
    int64_t placeholder2[8];

    std::atomic<int> completed_thread_num;
    int64_t placeholder3[8];  

    std::atomic<bool> completed;

private:
    const ThreadPool& _thread_pool;
    const Range _range;
    const ParallelTask& _body;
    const int _nstripes;
};

void WorkerThread::loop_body() {
    bool allow_active_wait = true;

    while (!_stop) {
        if (allow_active_wait > 0) {
            allow_active_wait = false;
            for (int i = 0; i < _active_wait; i++) {
                if (awake) {
                    break;
                }
                
                std::this_thread::yield();
            }
        }

        std::unique_lock<std::mutex> lock(thread_mutex);
        while (!awake) {
            is_active = false;
            cond_thread_wake.wait(lock);
            is_active = true;
        }

        allow_active_wait = true;

        std::shared_ptr<ParallelJob> j_ptr;
        swap(j_ptr, job);
        awake = false;
        lock.unlock();

        if (_stop || !j_ptr || j_ptr->cur_position >= j_ptr->range().size()) {
            continue;
        }

        j_ptr->active_thread_num.fetch_add(1, std::memory_order_seq_cst);
        j_ptr->run();
                
        int completed = j_ptr->completed_thread_num.fetch_add(1, std::memory_order_seq_cst) + 1;
        int active = j_ptr->active_thread_num.load(std::memory_order_acquire);
        
        if (active == completed) {
            bool need_notify = !j_ptr->completed;
            j_ptr->completed = true;
            j_ptr = nullptr;
            if (need_notify) {
                std::unique_lock<std::mutex> tlock(_thread_pool.mutex_notify);
                tlock.unlock();
                _thread_pool.job_complete.notify_all();
            }
        }
    }
}

ThreadPool::ThreadPool()
        : _active_wait(10000) {
    _thread_num = get_cpu_num() > 1 ? 2 : 1;
}

bool ThreadPool::_set_work_threads(unsigned int num) {
    if (num == _work_threads.size()) {
        return false;
    }

    if (num < _work_threads.size()) {
        std::vector< std::shared_ptr<WorkerThread> > release_threads(_work_threads.size() - num);
        for (size_t i = num; i < _work_threads.size(); ++i) {
            std::unique_lock<std::mutex> lock(_work_threads[i]->thread_mutex);
            _work_threads[i]->stop();
            _work_threads[i]->awake = true;
            lock.unlock();
            _work_threads[i]->cond_thread_wake.notify_all();
            std::swap(_work_threads[i], release_threads[i - num]);
        }
        _work_threads.resize(num);
        release_threads.clear();
        return false;
    } else {
        for (size_t i = _work_threads.size(); i < num; ++i) {
            _work_threads.push_back(std::shared_ptr<WorkerThread>(
                    new WorkerThread(*this, (unsigned)i)));
        }
    }
    return false;
}

ThreadPool::~ThreadPool() {
    _set_work_threads(0);
}

void ThreadPool::run(
        const Range& range,
        const ParallelTask& body,
        int nstripes) {
    if (_thread_num <= 1 || _job || nstripes == 1) {
        body(range);
        return;
    }

    std::unique_lock<std::mutex> lock(_pool_mutex);
    _set_work_threads(_thread_num - 1);
    _job = std::shared_ptr<ParallelJob>(
            new ParallelJob(*this, range, body, nstripes));
    lock.unlock();

    for (size_t i = 0; i < _work_threads.size(); ++i) {
        WorkerThread& thread = *(_work_threads[i].get());
        if (thread.is_active || thread.awake || thread.job) {
            std::unique_lock<std::mutex> lock(thread.thread_mutex);
            thread.job = _job;
            bool is_active = thread.is_active;
            thread.awake = true;
            lock.unlock();
            if (!is_active) {
                thread.cond_thread_wake.notify_all();
            }
        } else {
            thread.job = _job;
            thread.awake = true;
            thread.cond_thread_wake.notify_all();
        }
    }

    _job->run();

    if (_job->completed || _job->active_thread_num == 0) {
        _job->completed = true;
    } else {
        if (_active_wait > 0) {
            for (int i = 0; i < _active_wait; i++)  {
                if (_job->completed) {
                    break;
                }
                
                std::this_thread::yield();
            }
        }

        if (!_job->completed) {
            std::unique_lock<std::mutex> tlock(mutex_notify);
            for (;;) {
                if (_job->completed) {
                    break;
                }
                job_complete.wait(tlock);
            }
        }
    }

    if (_job) {
        std::unique_lock<std::mutex> lock(_pool_mutex);
        _job = nullptr;
    }
}

unsigned int ThreadPool::get_thread_num() const {
    return _thread_num;
}

void ThreadPool::set_thread_num(unsigned int num) {
    if (num == _thread_num) return;

    std::unique_lock<std::mutex> lock(_pool_mutex);
    _thread_num = FCV_MAX(num, 1);
    _set_work_threads(_thread_num - 1);
}

G_FCV_NAMESPACE1_END()