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

#include <stdio.h>

#include "flycv_namespace.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/base/interface/macro_export.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Specify a continuous subinterval
 */
class FCV_API Range {
public:
    Range() : _start(0), _end(0) {}

    Range(int start, int end) :
            _start(start),
            _end(end) {}

    int start() const {
        return _start;
    }

    int end() const {
        return _end;
    }

    void set_start(int start) {
        _start = start;
    }

    void set_end(int end) {
        _end = end;
    }

    int size() const {
        return _end - _start;
    }

    bool empty() const {
        return _start == _end;
    }

private:
    int _start;
    int _end;
};

/**
 * @brief Virtual base class for thread tasks.
 * To use paralell_run interface, you need to implement a subclass that inherits from this,
 * and override the invoke function.
 */
class FCV_API ParallelTask {
public:
    virtual ~ParallelTask() {}
    virtual void operator() (const Range& range) const = 0;
};

/**
 * @brief Set the thread num of thread pool.
 * @param[in] num the thread num expected to be set
 */
EXTERN_C FCV_API void set_thread_num(int num);

/**
 * @brief Get the thread num of thread pool.
 * @return the thread num
 */
EXTERN_C FCV_API int get_thread_num();

/**
 * @brief This function is used to improve performance through multi-threading.
 * @param[in] range The range of data that needs to be processed.
 * @param[in] body Functions that require thread-safe multi-threaded iteration.
 * @param[in] nstripes Step size processed by each thread.
 * @return excute status, zero : sucess, non-zero: failed.
 */
FCV_API int parallel_run(
        const Range& range,
        const ParallelTask& body,
        int nstripes = -1);

G_FCV_NAMESPACE1_END()
