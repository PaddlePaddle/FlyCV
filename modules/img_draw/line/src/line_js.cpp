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

#include "modules/img_draw/line/interface/line.h"

using g_fcv_ns::LineType;

EMSCRIPTEN_BINDINGS(line) {
    emscripten::enum_<LineType>("LineType")
        .value("FILLED", LineType::FILLED)
        .value("LINE_4", LineType::LINE_4)
        .value("LINE_8", LineType::LINE_8)
        .value("LINE_AA", LineType::LINE_AA)
        ;

     emscripten::function("line", &g_fcv_ns::line);
}
