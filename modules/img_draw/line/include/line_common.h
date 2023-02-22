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

#include <algorithm>

#include "modules/core/mat/interface/mat.h"
#include "modules/img_draw/line/interface/line.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/** @brief Line iterator
The class is used to iterate over all the pixels on the raster line
segment connecting two specified points.

The class LineIterator is used to get each pixel of a raster line. It
can be treated as versatile implementation of the Bresenham algorithm
where you can stop at each pixel and do some extra processing, for
example, grab pixel values along the line or draw a line with an effect
(for example, with XOR operation).

The number of pixels along the line is stored in LineIterator::count.
The method LineIterator::pos returns the current position in the image:

@code{.cpp}
// grabs pixels along the line (pt1, pt2)
// from 8-bit 3-channel image to the buffer
LineIterator it(img, pt1, pt2, 8);
LineIterator it2 = it;
vector<Vec3b> buf(it.count);

for(int i = 0; i < it.count; i++, ++it)
    buf[i] = *(const Vec3b*)*it;

// alternative way of iterating through the line
for(int i = 0; i < it2.count; i++, ++it2)
{
    Vec3b val = img.at<Vec3b>(it2.pos());
}
@endcode
*/
class LineIterator {
public:
    /** @brief initializes the iterator
     * creates iterators for the line connecting pt1 and pt2
     * the line will be clipped on the image boundaries
     * the line is 8-connected or 4-connected
     * If left_to_right=true, then the iteration is always done
     * from the left-most point to the right most,
     * not to depend on the ordering of pt1 and pt2 parameters;
     */
    LineIterator(
            const Mat& img,
            Point pt1,
            Point pt2,
            int connectivity = 8,
            bool left_to_right = false) {
        init(&img, Rect(0, 0, img.width(), img.height()),
                pt1, pt2, connectivity, left_to_right);
        ptmode = false;
    }

    LineIterator(
            Point pt1,
            Point pt2,
            int connectivity = 8,
            bool left_to_right = false) {
        init(0, Rect(std::min(pt1.x(), pt2.x()), std::min(pt1.y(), pt2.y()),
                std::max(pt1.x(), pt2.x()) - std::min(pt1.x(), pt2.x()) + 1,
                std::max(pt1.y(), pt2.y()) - std::min(pt1.y(), pt2.y()) + 1),
                pt1, pt2, connectivity, left_to_right);
        ptmode = true;
    }

    LineIterator(
            Size bounding_area_size,
            Point pt1,
            Point pt2,
            int connectivity = 8,
            bool left_to_right = false) {
        init(0, Rect(0, 0, bounding_area_size.width(), bounding_area_size.height()),
                pt1, pt2, connectivity, left_to_right);
        ptmode = true;
    }

    LineIterator(
            Rect bounding_area_rect,
            Point pt1, Point pt2,
            int connectivity = 8,
            bool left_to_right = false) {
        init(0, bounding_area_rect, pt1, pt2, connectivity, left_to_right);
        ptmode = true;
    }

    void init(
            const Mat* img,
            Rect bounding_area_rect,
            Point pt1,
            Point pt2,
            int connectivity,
            bool left_to_right);

    /** @brief returns pointer to the current pixel
     */
    unsigned char* operator*() {
        return ptmode ? 0 : ptr;
    }

    /** @brief prefix increment operator (++it). shifts iterator to the next
     * pixel
     */
    LineIterator& operator++() {
        int mask = err < 0 ? -1 : 0;
        err += minus_delta + (plus_delta & mask);
        if (!ptmode) {
            ptr += minus_step + (plus_step & mask);
        } else {
            p.set_x(p.x() + minus_shift + (plus_shift & mask));
            p.set_y(p.y() + minus_step + (plus_step & mask));
        }
        return *this;
    }

    /** @brief postfix increment operator (it++). shifts iterator to the next
     * pixel
     */
    LineIterator operator++(int) {
        LineIterator it = *this;
        ++(*this);
        return it;
    }

    /** @brief returns coordinates of the current pixel
     */
    Point pos() const {
        if (!ptmode) {
            size_t offset = (size_t)(ptr - ptr0);
            int y = (int)(offset / step);
            int x = (int)((offset - (size_t)y * step) / elem_size);
            return Point(x, y);
        }
        return p;
    }

    unsigned char* ptr;
    const unsigned char* ptr0;
    int step;
    int elem_size;
    int err;
    int count;
    int minus_delta;
    int plus_delta;
    int minus_step;
    int plus_step;
    int minus_shift;
    int plus_shift;
    Point p;
    bool ptmode;
};

void line_connection(
        Mat& img,
        Point pt1,
        Point pt2,
        const void* color,
        LineType line_type = LineType::LINE_8);

void line2(
        Mat& img,
        Point2l pt1,
        Point2l pt2,
        const void* color);

void line_AA(
        Mat& img,
        Point2l pt1,
        Point2l pt2,
        const void* color);

void draw_line(
        Mat& img,
        Point& p0,
        Point& p1,
        unsigned char* color,
        int thickness,
        LineType line_type,
        int flags,
        int shift);

void fill_convex_poly(
        Mat& img,
        const Point2l* v,
        int npts,
        const void* color,
        LineType line_type,
        int shift);

void draw_circle(
        Mat& img,
        Point center,
        int radius,
        const void* color,
        int fill);

int line_common(
        Mat& img,
        Point& pt1,
        Point& pt2,
        const Scalar& color,
        int thickness = 1,
        LineType line_type = LineType::LINE_8,
        int shift = 0);

G_FCV_NAMESPACE1_END()
