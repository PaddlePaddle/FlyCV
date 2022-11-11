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

// the thread num for all benchmark test
const int G_THREAD_NUM = 1;

/**
 * @brief mock test data
 * @param len data size
 * @param feed_num feed num to generator data
 * @param data data pointer, the memory needs to be allocated outside
 */
template<class T>
inline void construct_data(size_t len, int feed_num, void* data) {
    T* src_data = reinterpret_cast<T*>(data);

    for (size_t i = 0; i < len; ++i) {
        src_data[i] = (feed_num + i) % 256;
    }
}