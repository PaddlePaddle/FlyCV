// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#ifdef USE_C_API
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

#ifdef USE_FCV_DLL
#ifdef FCV_DLL_EXPORT
#define FCV_API __declspec(dllexport)
#else
#define FCV_API __declspec(dllimport)
#endif
#else
#define FCV_API
#endif