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

#include "modules/img_draw/poly_lines/include/poly_lines_common.h"
#include "modules/img_draw/line/include/line_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

int poly_lines_common(
        Mat& img,
        const Point2l* v,
        int count,
        bool is_closed,
        const void* color,
        int thickness,
        LineType line_type,
        int shift) {
    if (!v || count <= 0) {
        return -1;
    }

    int i = is_closed ? count - 1 : 0;
    int flags = 2 + !is_closed;
    Point p0(v[i].x(), v[i].y());

    for (i = !is_closed; i < count; i++ ) {
        Point p = Point(v[i].x(), v[i].y());
        draw_line(img, p0, p, (unsigned char*)color, thickness, line_type, flags, shift);
        p0 = p;
        flags = 2;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
