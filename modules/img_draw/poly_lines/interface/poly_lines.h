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

#include "modules/core/mat/interface/mat.h"
#include "modules/img_draw/line/interface/line.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief draw polygons
 * @param[inout] img The image on which polygons is to be drawn, only support PKG_BGR_U8 or PKG_RGB_U8 format now
 * @param[in] v pts Array of polygons where each polygon is represented as an array of points.
 * @param[in] count ncontours All Polygon number.
 * @param[in] is_closed
 * @param[in] color polygons color.
 * @param[in] thickness Thickness of the circle outline, if positive. Negative values, like #FILLED,
 * @param[in] line_type Type of the circle boundary.
 * @param[in] shift shift Number of fractional bits in the coordinates of the center and in the radius value.
 * @return
 */
FCV_API int poly_lines(
        Mat& img,
        const Point2l* v,
        int count,
        bool is_closed,
        const void* color,
        int thickness,
        LineType line_type,
        int shift);

G_FCV_NAMESPACE1_END()
