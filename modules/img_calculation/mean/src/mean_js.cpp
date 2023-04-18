// Copyright (c) 2023 FlyCV Authors. All Rights Reserved.
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

#include <emscripten/bind.h>

#include "modules/img_calculation/mean/interface/mean.h"

EMSCRIPTEN_BINDINGS(mean) {
     emscripten::function("mean", emscripten::select_overload<g_fcv_ns::Scalar(const g_fcv_ns::Mat&)>(&g_fcv_ns::mean));
     emscripten::function("mean", emscripten::select_overload<g_fcv_ns::Scalar(const g_fcv_ns::Mat&, const g_fcv_ns::Rect&)>(&g_fcv_ns::mean));
}