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

#include "modules/fusion_api/bgr_to_rgba_with_mask/interface/bgr_to_rgba_with_mask.h"

EMSCRIPTEN_BINDINGS(bgr_to_rgba_with_mask) {
    emscripten::function("bgrToRgbaWithMask", &g_fcv_ns::bgr_to_rgba_with_mask);
}