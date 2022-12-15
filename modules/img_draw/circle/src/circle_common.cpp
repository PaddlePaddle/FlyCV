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

/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this
//  license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without
// modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright
// notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote
// products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is"
// and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any
// direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#include "modules/img_draw/circle/include/circle_common.h"
#include "modules/img_draw/line/include/line_common.h"
#include "modules/img_draw/fill_poly/include/fill_poly_common.h"
#include "modules/img_draw/poly_lines/interface/poly_lines.h"
#include "modules/core/base/include/macro_utils.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

enum {
    XY_SHIFT = 16,
    XY_ONE = 1 << XY_SHIFT,
    DRAWING_STORAGE_BLOCK = (1 << 12) - 256
};

static void scalar_to_raw_data(
        const Scalar& s,
        void* const buf,
        const int cn,
        const int unroll_to) {
    int i = 0;
    unsigned char* out = (unsigned char*)buf;
    for (; i < cn; i++) {
        out[i] = s[i];
    }

    for (; i < unroll_to; i++) {
        out[i] = out[i - cn];
    }
}

static void ellipse_ex(
        Mat& img,
        Point2l center,
        Size2l axes,
        int angle,
        int arc_start,
        int arc_end,
        const void* color,
        int thickness,
        LineType line_type ) {
    UN_USED(angle);

    axes.set_width(std::abs(axes.width()));
    axes.set_height(std::abs(axes.height()));
    int delta = (int)((std::max(axes.width(), axes.height()) + (XY_ONE >> 1)) >> XY_SHIFT);
    delta = delta < 3 ? 90 : delta < 10 ? 30 : delta < 15 ? 18 : 5;
    std::vector<Point2l> v;
    Point2l prevPt(0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
    v.resize(0);

    // If there are no points, it's a zero-size polygon
    if (v.size() == 1) {
        v.assign(2, center);
    }

    if (thickness >= 0) {
        poly_lines(img, &v[0], (int)v.size(), false, color, thickness, line_type, XY_SHIFT);
    } else if(arc_end - arc_start >= 360) {
        fill_convex_poly(img, &v[0], (int)v.size(), color, line_type, XY_SHIFT);
    } else {
        v.push_back(center);
        std::vector<PolyEdge> edges;
        collect_poly_edges(img, &v[0], (int)v.size(), edges, color, line_type, XY_SHIFT);
        fill_edge_collection(img, edges, color);
    }
}

int circle_common(
        Mat& img,
        Point center,
        int radius,
        const Scalar& color,
        int thickness,
        LineType line_type,
        int shift) {
    if (line_type == LineType::LINE_AA && img.type_byte_size() == 1) {
        line_type = LineType::LINE_8;
    }

    double buf[4];
    scalar_to_raw_data(color, buf, img.channels(), 0);

    if (thickness > 1 || line_type != LineType::LINE_8 || shift > 0) {
        Point2l center_pts(center);
        int64_t radius_tmp(radius);
        center_pts.set_x(center_pts.x() << (XY_SHIFT - shift));
        center_pts.set_y(center_pts.y() << (XY_SHIFT - shift));
        radius_tmp <<= XY_SHIFT - shift;
        ellipse_ex(img, center_pts, Size2l(radius_tmp, radius_tmp),
                0, 0, 360, buf, thickness, line_type);
    } else {
        draw_circle(img, center, radius, buf, thickness < 0);
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
