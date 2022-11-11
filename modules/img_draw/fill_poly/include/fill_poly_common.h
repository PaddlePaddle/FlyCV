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

class PolyEdge {
public:
    PolyEdge() : y0(0), y1(0), x(0), dx(0), next(0) {}

    int y0;
    int y1;
    int64_t x;
    int64_t dx;
    PolyEdge* next;
};

void collect_poly_edges(
        Mat& img,
        const Point2l* v,
        int count,
        std::vector<PolyEdge>& edges,
        const void* color,
        LineTypes line_type,
        int shift,
        Point offset = Point());

void fill_edge_collection(
        Mat& img,
        std::vector<PolyEdge>& edges,
        const void* color);

int fill_poly_common(
        Mat& img,
        const Point** pts,
        const int* npts,
        int ncontours,
        const Scalar& color,
        LineTypes line_type = LineTypes::LINE_8,
        int shift = 0,
        Point offset = Point(0, 0));

G_FCV_NAMESPACE1_END()