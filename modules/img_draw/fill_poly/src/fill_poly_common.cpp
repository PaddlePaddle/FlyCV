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

#include "modules/img_draw/fill_poly/include/fill_poly_common.h"

#include <algorithm>

#include "modules/img_draw/line/include/line_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

enum {
    XY_SHIFT = 16,
    XY_ONE = 1 << XY_SHIFT,
    DRAWING_STORAGE_BLOCK = (1 << 12) - 256
};

static inline void ICV_HLINE_X(
        unsigned char* ptr,
        int xl,
        int xr,
        const unsigned char* color,
        int pix_size) {
    unsigned char* hline_min_ptr = (unsigned char*)(ptr) + (xl) * (pix_size);
    unsigned char* hline_end_ptr = (unsigned char*)(ptr) + (xr + 1) * (pix_size);
    unsigned char* hline_ptr = hline_min_ptr;

    if (pix_size == 1) {
        memset(hline_min_ptr, *color, hline_end_ptr - hline_min_ptr);
    } else  {
        if (hline_min_ptr < hline_end_ptr) {
            memcpy(hline_ptr, color, pix_size);
            hline_ptr += pix_size;
        }  // end if (hline_min_ptr < hline_end_ptr)
        size_t sizeToCopy = pix_size;
        while (hline_ptr < hline_end_ptr) {
            memcpy(hline_ptr, hline_min_ptr, sizeToCopy);
            hline_ptr += sizeToCopy;
            sizeToCopy = std::min(
                2 * sizeToCopy, static_cast<size_t>(hline_end_ptr - hline_ptr));
        }  // end while(hline_ptr < hline_end_ptr)
    }      // end if (pix_size != 1)
}
// end ICV_HLINE_X()

static inline void ICV_HLINE(unsigned char* ptr, int xl, int xr, const void* color,
                             int pix_size) {
    ICV_HLINE_X(ptr, xl, xr, reinterpret_cast<const unsigned char*>(color), pix_size);
}
// end ICV_HLINE()

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

void collect_poly_edges(
        Mat& img,
        const Point2l* v,
        int count,
        std::vector<PolyEdge>& edges,
        const void* color,
        LineTypes line_type,
        int shift,
        Point offset) {
    int i = 0;
    int delta = offset.y() + ((1 << shift) >> 1);
    Point2l pt0 = v[count - 1];
    Point2l pt1;
    pt0.set_x((pt0.x() + offset.x()) << (XY_SHIFT - shift));
    pt0.set_y((pt0.y() + delta) >> shift);

    edges.reserve(edges.size() + count);

    for (i = 0; i < count; i++, pt0 = pt1) {
        Point2l t0, t1;
        PolyEdge edge;

        pt1 = v[i];
        pt1.set_x((pt1.x() + offset.x()) << (XY_SHIFT - shift));
        pt1.set_y((pt1.y() + delta) >> shift);

        if (line_type != LineTypes::LINE_AA) {
            t0.set_y(pt0.y());
            t1.set_y(pt1.y());
            t0.set_x((pt0.x() + (XY_ONE >> 1)) >> XY_SHIFT);
            t1.set_x((pt1.x() + (XY_ONE >> 1)) >> XY_SHIFT);
            line_connection(img, Point(t0.x(), t0.y()), Point(t1.x(),
                    t1.y()), color, line_type);
        } else {
            t0.set_x(pt0.x());
            t1.set_x(pt1.x());
            t0.set_y(pt0.y() << XY_SHIFT);
            t1.set_y(pt1.y() << XY_SHIFT);
            line_AA(img, t0, t1, color);
        }

        if (pt0.y() == pt1.y()) continue;

        if (pt0.y() < pt1.y()) {
            edge.y0 = (int)(pt0.y());
            edge.y1 = (int)(pt1.y());
            edge.x = pt0.x();
        } else {
            edge.y0 = (int)(pt1.y());
            edge.y1 = (int)(pt0.y());
            edge.x = pt1.x();
        }
        edge.dx = (pt1.x() - pt0.x()) / (pt1.y() - pt0.y());
        edges.push_back(edge);
    }
}

struct CmpEdges {
    bool operator()(const PolyEdge& e1, const PolyEdge& e2) {
        return e1.y0 - e2.y0 ? e1.y0 < e2.y0
                : e1.x - e2.x ? e1.x < e2.x : e1.dx < e2.dx;
    }
};

void fill_edge_collection(
        Mat& img,
        std::vector<PolyEdge>& edges,
        const void* color) {
    PolyEdge tmp;
    int i = 0, y = 0, total = (int)edges.size();
    Size size = img.size();
    PolyEdge* e = nullptr;
    int y_max = INT_MIN;
    int y_min = INT_MAX;
    int64_t x_max = 0xFFFFFFFFFFFFFFFF;
    int64_t x_min = 0x7FFFFFFFFFFFFFFF;
    int pix_size = (int)img.channels() * img.type_byte_size();

    if (total < 2) return;

    for (i = 0; i < total; i++) {
        PolyEdge& e1 = edges[i];
        // Determine x-coordinate of the end of the edge.
        // (This is not necessary x-coordinate of any vertex in the array.)
        int64_t x1 = e1.x + (e1.y1 - e1.y0) * e1.dx;
        y_min = std::min(y_min, e1.y0);
        y_max = std::max(y_max, e1.y1);
        x_min = std::min(x_min, e1.x);
        x_max = std::max(x_max, e1.x);
        x_min = std::min(x_min, x1);
        x_max = std::max(x_max, x1);
    }

    if (y_max < 0 || y_min >= size.height()
            || x_max < 0
            || x_min >= ((int64_t)size.width() << XY_SHIFT)) {
        return;
    }

    std::sort(edges.begin(), edges.end(), CmpEdges());

    // start drawing
    tmp.y0 = INT_MAX;
    edges.push_back(tmp);  // after this point we do not add
                           // any elements to edges, thus we can use pointers
    i = 0;
    tmp.next = 0;
    e = &edges[i];
    y_max = std::min(y_max, size.height());

    for (y = e->y0; y < y_max; y++) {
        PolyEdge* last = nullptr;
        PolyEdge* prelast = nullptr;
        PolyEdge* keep_prelast = nullptr;
        int sort_flag = 0;
        int draw = 0;
        int clipline = y < 0;

        prelast = &tmp;
        last = tmp.next;
        while (last || e->y0 == y) {
            if (last && last->y1 == y) {
                // exclude edge if y reaches its lower point
                prelast->next = last->next;
                last = last->next;
                continue;
            }
            keep_prelast = prelast;
            if (last && (e->y0 > y || last->x < e->x)) {
                // go to the next edge in active list
                prelast = last;
                last = last->next;
            } else if (i < total) {
                // insert new edge into active list if y reaches its upper point
                prelast->next = e;
                e->next = last;
                prelast = e;
                e = &edges[++i];
            } else
                break;

            if (draw) {
                if (!clipline) {
                    // convert x's from fixed-point to image coordinates
                    unsigned char* timg = img.ptr<unsigned char>(0, y);
                    int x1 = 0, x2 = 0;

                    if (keep_prelast->x > prelast->x) {
                        x1 = (int)((prelast->x + XY_ONE - 1) >> XY_SHIFT);
                        x2 = (int)(keep_prelast->x >> XY_SHIFT);
                    } else {
                        x1 = (int)((keep_prelast->x + XY_ONE - 1) >> XY_SHIFT);
                        x2 = (int)(prelast->x >> XY_SHIFT);
                    }

                    // clip and draw the line
                    if (x1 < size.width() && x2 >= 0) {
                        if (x1 < 0) x1 = 0;
                        if (x2 >= size.width()) x2 = size.width() - 1;
                        ICV_HLINE(timg, x1, x2, color, pix_size);
                    }
                }
                keep_prelast->x += keep_prelast->dx;
                prelast->x += prelast->dx;
            }
            draw ^= 1;
        }

        // sort edges (using bubble sort)
        keep_prelast = 0;

        do {
            prelast = &tmp;
            last = tmp.next;

            while (last != keep_prelast && last->next != 0) {
                PolyEdge* te = last->next;

                // swap edges
                if (last->x > te->x) {
                    prelast->next = te;
                    last->next = te->next;
                    te->next = last;
                    prelast = te;
                    sort_flag = 1;
                } else {
                    prelast = last;
                    last = te;
                }
            }
            keep_prelast = prelast;
        } while (sort_flag && keep_prelast != tmp.next && keep_prelast != &tmp);
    }
}

int fill_poly_common(
        Mat& img,
        const Point** pts,
        const int* npts,
        int ncontours,
        const Scalar& color,
        LineTypes line_type,
        int shift,
        Point offset) {
    double buf[4];
    scalar_to_raw_data(color, buf, img.channels(), 0);

    std::vector<PolyEdge> edges;

    int i = 0;
    int total = 0;

    for (i = 0; i < ncontours; i++) {
        total += npts[i];
    }

    edges.reserve(total + 1);
    for (i = 0; i < ncontours; i++) {
        std::vector<Point2l> _pts(pts[i], pts[i] + npts[i]);
        collect_poly_edges(img, _pts.data(), npts[i],
                edges, buf, line_type, shift, offset);
    }

    fill_edge_collection(img, edges, buf);

    return 0;
}

G_FCV_NAMESPACE1_END()
