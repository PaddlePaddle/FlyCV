/***************************************************************************
 *
 * Copyright (c) 2022 Baidu.com, Inc. All Rights Reserved
 *
 **************************************************************************/

/**
 * @contributor     litonghui.huweidong01
 * @created         2022-08-31 16:44:00
 * @brief
 */

#ifndef MARLIN_INCLUDE_UTILS_LOG_UTILS_H_
#define MARLIN_INCLUDE_UTILS_LOG_UTILS_H_

#define DEFAULT_TAG "MARLIN"

#ifdef _WIN32
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

// Log
#ifdef __ANDROID__
#include <android/log.h>
#define LOGDT(fmt, tag, ...)                                                                                           \
    __android_log_print(ANDROID_LOG_DEBUG, tag, ("%s [File %s][Line %d] " fmt), __PRETTY_FUNCTION__, __FILE__,         \
                        __LINE__, ##__VA_ARGS__)
#define LOGIT(fmt, tag, ...)                                                                                           \
    __android_log_print(ANDROID_LOG_INFO, tag, ("%s [File %s][Line %d] " fmt), __PRETTY_FUNCTION__, __FILE__,          \
                        __LINE__, ##__VA_ARGS__)
#define LOGET(fmt, tag, ...)                                                                                           \
    __android_log_print(ANDROID_LOG_ERROR, tag, ("%s [File %s][Line %d] " fmt), __PRETTY_FUNCTION__, __FILE__,         \
                        __LINE__, ##__VA_ARGS__);                                                                      \
    fprintf(stderr, ("E/%s: %s [File %s][Line %d] " fmt), tag, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOGDT(fmt, tag, ...)                                                                                           \
    fprintf(stdout, ("D/%s: %s [File %s][Line %d] " fmt), tag, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGIT(fmt, tag, ...)                                                                                           \
    fprintf(stdout, ("I/%s: %s [File %s][Line %d] " fmt), tag, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOGET(fmt, tag, ...)                                                                                           \
    fprintf(stderr, ("E/%s: %s [File %s][Line %d] " fmt), tag, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#endif  //__ANDROID__

#define LOGD(fmt, ...) LOGDT(fmt, DEFAULT_TAG, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOGIT(fmt, DEFAULT_TAG, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOGET(fmt, DEFAULT_TAG, ##__VA_ARGS__)

// Assert
#include <cassert>
#define ASSERT(x)                                                                                                      \
    {                                                                                                                  \
        int res = (x);                                                                                                 \
        if (!res) {                                                                                                    \
            LOGE("Error: assert failed\n");                                                                              \
            assert(res);                                                                                               \
        }                                                                                                              \
    }

#ifndef DEBUG
#undef LOGDT
#undef LOGD
#define LOGDT(fmt, tag, ...)
#define LOGD(fmt, ...)
#undef ASSERT
#define ASSERT(x)
#endif  // DEBUG


#endif //MARLIN_INCLUDE_UTILS_LOG_UTILS_H_
