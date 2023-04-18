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

#include "modules/core/cmat/interface/cmat.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief The algorithm for line generation
 */
enum CLineType {
    FILLED = -1,
    LINE_4 = 4,   //!< 4-connected line
    LINE_8 = 8,   //!< 8-connected line
    LINE_AA = 16  //!< antialiased line, not support now
};

/** 
 * @brief Draws a line segment connecting two points.
 * The function line draws the line segment between pt1 and pt2 points in the
 * image. The line is clipped by the image boundaries. For non-antialiased lines
 * with integer coordinates, the 8-connected or 4-connected Bresenham algorithm is
 * used. Thick lines are drawn with rounding endings. Antialiased lines are drawn using Gaussian filtering.
 * @param[inout] img Image only support PKG_BGR_U8 or PKG_RGB_U8 format now.
 * @param[in] pt1 First point of the line segment.
 * @param[in] pt2 Second point of the line segment.
 * @param[in] color Line color.
 * @param[in] thickness Line thickness.
 * @param[in] line_type Type of the line.
 * @param[in] shift Number of fractional bits in the point coordinates.
 */
EXTERN_C FCV_API int fcvLine(
        CMat* img,
        CPoint pt1,
        CPoint pt2,
        CScalar* color,
        int thickness,
        CLineType line_type,
        int shift);

G_FCV_NAMESPACE1_END()