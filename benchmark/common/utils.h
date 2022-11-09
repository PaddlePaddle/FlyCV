/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     taotianran
 * @created         2022-07-22 11:40
 */

#pragma once

template<class T>
inline void construct_data(size_t len, int feed_num, void* data) {
    T* src_data = reinterpret_cast<T*>(data);

    for (size_t i = 0; i < len; ++i) {
        src_data[i] = (feed_num + i) % 256;
    }
}