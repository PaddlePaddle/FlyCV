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

#include "modules/core/base/interface/basic_types.h"
#include "modules/core/basic_math/interface/basic_math.h"
#include <math.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

// Class size implement
template class FCV_API Size_<int>;
template class FCV_API Size_<int64_t>;
template class FCV_API Size_<float>;
template class FCV_API Size_<double>;

// Class rect implement
template class FCV_API Rect_<int>;
template class FCV_API Rect_<float>;
template class FCV_API Rect_<double>;

// class RotatedRect implement
void RotatedRect::points(std::vector<Point2f>& pts) {
    double angle = _angle * FCV_PI / 180.;
    float b = (float)cos(angle) * 0.5f;
    float a = (float)sin(angle) * 0.5f;
    Size2f p_size = size();
    Point2f point = center();
    pts.resize(4);
    pts[0].set_x(point.x() - a * p_size.height() - b * p_size.width());
    pts[0].set_y(point.y() + b * p_size.height() - a * p_size.width());
    pts[1].set_x(point.x() + a * p_size.height() - b * p_size.width());
    pts[1].set_y(point.y() - b * p_size.height() - a * p_size.width());
    pts[2].set_x(2 * point.x() - pts[0].x());
    pts[2].set_y(2 * point.y() - pts[0].y());
    pts[3].set_x(2 * point.x() - pts[1].x());
    pts[3].set_y(2 * point.y() - pts[1].y());
}

// class Scalar implement
template class FCV_API Scalar_<int>;
template class FCV_API Scalar_<float>;
template class FCV_API Scalar_<double>;

G_FCV_NAMESPACE1_END()
