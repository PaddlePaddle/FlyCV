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

#include "modules/fusion_api/y420sp_to_resize_to_bgr/interface/y420sp_to_resize_to_bgr.h"

EMSCRIPTEN_BINDINGS(y420sp_to_resize_to_bgr) {
     emscripten::function("nv12ToResizeToBgr", &g_fcv_ns::nv12_to_resize_to_bgr);
     emscripten::function("nv21ToResizeToBgr", &g_fcv_ns::nv21_to_resize_to_bgr);
}