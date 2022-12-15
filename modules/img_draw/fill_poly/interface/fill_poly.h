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
 * @brief Fills the area bounded by one or more polygons.
 * The function cv::fillPoly fills an area bounded by several polygonal contours.
 * The function can fill complex areas, for example, areas with holes, contours
 * with self-intersections (some of their parts), and so forth.
 * @param[inout] img Image only support PKG_BGR_U8 or PKG_RGB_U8 format now.
 * @param[in] pts Array of polygons where each polygon is represented as an array of points.
 * @param[in] npts Array of each polygons pts data.
 * @param[in] ncontours All Polygon number.
 * @param[in] color Polygon color.
 * @param[in] line_type Type of the polygon boundaries. See #line_types
 * @param[in] shift Number of fractional bits in the vertex coordinates.
 * @param[in] offset Optional offset of all points of the contours.
 */
FCV_API int fill_poly(
        Mat& img,
        const Point** pts,
        const int* npts,
        int ncontours,
        const Scalar& color,
        LineType line_type = LineType::LINE_8,
        int shift = 0,
        Point offset = Point(0, 0));

G_FCV_NAMESPACE1_END()
