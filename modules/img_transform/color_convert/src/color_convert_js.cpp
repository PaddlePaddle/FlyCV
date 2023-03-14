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

#include "modules/img_transform/color_convert/interface/color_convert.h"

using g_fcv_ns::Mat;
using g_fcv_ns::ColorConvertType;

EMSCRIPTEN_BINDINGS(cvt_color_enum_types) {
    emscripten::enum_<ColorConvertType>("ColorConvertType")
        .value("CVT_PA_BGR2GRAY", ColorConvertType::CVT_PA_BGR2GRAY)
        .value("CVT_PA_RGB2GRAY", ColorConvertType::CVT_PA_RGB2GRAY)
        .value("CVT_PA_BGR2PA_RGB", ColorConvertType::CVT_PA_BGR2PA_RGB)
        .value("CVT_PA_RGB2PA_BGR", ColorConvertType::CVT_PA_RGB2PA_BGR)
        .value("CVT_PA_BGR2PA_BGRA", ColorConvertType::CVT_PA_BGR2PA_BGRA)
        .value("CVT_PA_RGB2PA_RGBA", ColorConvertType::CVT_PA_RGB2PA_RGBA)
        .value("CVT_PA_BGR2PA_RGBA", ColorConvertType::CVT_PA_BGR2PA_RGBA)
        .value("CVT_PA_RGB2PA_BGRA", ColorConvertType::CVT_PA_RGB2PA_BGRA)
        .value("CVT_PA_BGRA2PA_BGR", ColorConvertType::CVT_PA_BGRA2PA_BGR)
        .value("CVT_PA_RGBA2PA_RGB", ColorConvertType::CVT_PA_RGBA2PA_RGB)
        .value("CVT_PA_RGBA2PA_BGR", ColorConvertType::CVT_PA_RGBA2PA_BGR)
        .value("CVT_PA_BGRA2PA_RGB", ColorConvertType::CVT_PA_BGRA2PA_RGB)
        .value("CVT_PA_BGRA2PA_RGBA", ColorConvertType::CVT_PA_BGRA2PA_RGBA)
        .value("CVT_PA_RGBA2PA_BGRA", ColorConvertType::CVT_PA_RGBA2PA_BGRA)
        .value("CVT_GRAY2PA_RGB", ColorConvertType::CVT_GRAY2PA_RGB)
        .value("CVT_GRAY2PA_BGR", ColorConvertType::CVT_GRAY2PA_BGR)
        .value("CVT_GRAY2PA_BGRA", ColorConvertType::CVT_GRAY2PA_BGRA)
        .value("CVT_GRAY2PA_RGBA", ColorConvertType::CVT_GRAY2PA_RGBA)
        .value("CVT_PA_BGR2NV12", ColorConvertType::CVT_PA_BGR2NV12)
        .value("CVT_PA_BGR2NV21", ColorConvertType::CVT_PA_BGR2NV21)
        .value("CVT_PA_RGB2NV12", ColorConvertType::CVT_PA_RGB2NV12)
        .value("CVT_PA_RGB2NV21", ColorConvertType::CVT_PA_RGB2NV21)
        .value("CVT_PA_BGRA2NV12", ColorConvertType::CVT_PA_BGRA2NV12)
        .value("CVT_PA_BGRA2NV21", ColorConvertType::CVT_PA_BGRA2NV21)
        .value("CVT_PA_RGBA2NV12", ColorConvertType::CVT_PA_RGBA2NV12)
        .value("CVT_PA_RGBA2NV21", ColorConvertType::CVT_PA_RGBA2NV21)
        .value("CVT_NV122PA_RGB", ColorConvertType::CVT_NV122PA_RGB)
        .value("CVT_NV212PA_RGB", ColorConvertType::CVT_NV212PA_RGB)
        .value("CVT_NV122PA_BGR", ColorConvertType::CVT_NV122PA_BGR)
        .value("CVT_NV212PA_BGR", ColorConvertType::CVT_NV212PA_BGR)
        .value("CVT_I4202PA_BGR", ColorConvertType::CVT_I4202PA_BGR)
        .value("CVT_NV122PA_BGRA", ColorConvertType::CVT_NV122PA_BGRA)
        .value("CVT_NV212PA_BGRA", ColorConvertType::CVT_NV212PA_BGRA)
        .value("CVT_NV122PA_RGBA", ColorConvertType::CVT_NV122PA_RGBA)
        .value("CVT_NV212PA_RGBA", ColorConvertType::CVT_NV212PA_RGBA)
        .value("CVT_PA_BGR2PL_BGR", ColorConvertType::CVT_PA_BGR2PL_BGR)
        .value("CVT_PL_BGR2PA_BGR", ColorConvertType::CVT_PL_BGR2PA_BGR)
        .value("CVT_GRAY2PA_BGR565", ColorConvertType::CVT_GRAY2PA_BGR565)
        .value("CVT_PA_BGR2PA_BGR565", ColorConvertType::CVT_PA_BGR2PA_BGR565)
        .value("CVT_PA_RGB2PA_BGR565", ColorConvertType::CVT_PA_RGB2PA_BGR565)
        .value("CVT_PA_BGRA2PA_BGR565", ColorConvertType::CVT_PA_BGRA2PA_BGR565)
        .value("CVT_PA_RGBA2PA_BGR565", ColorConvertType::CVT_PA_RGBA2PA_BGR565)
        .value("CVT_PA_RGBA2PA_mRGBA", ColorConvertType::CVT_PA_RGBA2PA_mRGBA)
        ;
}

EMSCRIPTEN_BINDINGS(color_convert) {
     emscripten::function("cvtColor",
            emscripten::select_overload<int(const Mat&, Mat&, ColorConvertType)>(&g_fcv_ns::cvt_color));
}