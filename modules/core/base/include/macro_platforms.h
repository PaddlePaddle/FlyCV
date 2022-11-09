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

#include "flycv_namespace.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef __ANDROID__
#define FCV_OS_ANDROID

#elif __APPLE__
    #define FCV_OS_APPLE
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE
    #define FCV_OS_IPHONE
    #elif TARGET_OS_MAC
    #define FCV_OS_MAC
    #else
    #error "Unknown Apple platform"
    #endif

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__NT__)
    #define FCV_OS_WINDOWS
    #ifdef _WIN64
    #define FCV_OS_WIN64
    #else
    #define FCV_OS_WIN32
    #endif

#elif defined(__linux__) || defined(linux) || defined(_linux_)
#define FCV_OS_LINUX

#elif __unix__
#define FCV_OS_UNIX

#endif

G_FCV_NAMESPACE1_END()
