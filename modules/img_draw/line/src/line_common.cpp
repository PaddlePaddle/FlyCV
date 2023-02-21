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

#include "modules/img_draw/line/include/line_common.h"

#include <algorithm>
#include <cmath>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define CV_AA 16

enum {
    XY_SHIFT = 16,
    XY_ONE = 1 << XY_SHIFT,
    DRAWING_STORAGE_BLOCK = (1 << 12) - 256
};

static bool clip_line(
        Size2l img_size,
        Point2l& pt1,
        Point2l& pt2) {
    if (img_size.width() <= 0 || img_size.height() <= 0) {
        return false;
    }

    int c1 = 0;
    int c2 = 0;
    int64_t right = img_size.width() - 1;
    int64_t bottom = img_size.height() - 1;

    c1 = (pt1.x() < 0) + (pt1.x() > right) * 2
            + (pt1.y() < 0) * 4 + (pt1.y() > bottom) * 8;
    c2 = (pt2.x() < 0) + (pt2.x() > right) * 2
            + (pt2.y() < 0) * 4 + (pt2.y() > bottom) * 8;

    if ((c1 & c2) == 0 && (c1 | c2) != 0) {
        int64_t a = 0;
        if (c1 & 12) {
            a = c1 < 8 ? 0 : bottom;
            pt1.set_x(pt1.x() + (int64_t)((double)(a - pt1.y()) *
                    (pt2.x() - pt1.x()) / (pt2.y() - pt1.y())));
            pt1.set_y(a);
            c1 = (pt1.x() < 0) + (pt1.x() > right) * 2;
        }

        if (c2 & 12) {
            a = c2 < 8 ? 0 : bottom;
            pt2.set_x(pt2.x() + (int64_t)((double)(a - pt2.y()) *
                    (pt2.x() - pt1.x()) / (pt2.y() - pt1.y())));
            pt2.set_y(a);
            c2 = (pt2.x() < 0) + (pt2.x() > right) * 2;
        }

        if ((c1 & c2) == 0 && (c1 | c2) != 0) {
            if (c1) {
                a = c1 == 1 ? 0 : right;
                pt1.set_y(pt1.y() + (int64_t)((double)(a - pt1.x()) *
                        (pt2.y() - pt1.y()) / (pt2.x() - pt1.x())));
                pt1.set_x(a);
                c1 = 0;
            }

            if (c2) {
                a = c2 == 1 ? 0 : right;
                pt2.set_y(pt2.y() + (int64_t)((double)(a - pt2.x()) *
                        (pt2.y() - pt1.y()) / (pt2.x() - pt1.x())));
                pt2.set_x(a);
                c2 = 0;
            }
        }
    }

    return (c1 | c2) == 0;
}

static bool clip_line(
        Size img_size,
        Point& pt1,
        Point& pt2) {
    Point2l p1(pt1.x(), pt1.y());
    Point2l p2(pt2.x(), pt2.y());
    bool inside = clip_line(Size2l(img_size.width(), img_size.height()), p1, p2);
    pt1.set_x((int)p1.x());
    pt1.set_y((int)p1.y());
    pt2.set_x((int)p2.x());
    pt2.set_y((int)p2.y());

    return inside;
}

void line_connection(
        Mat& img,
        Point pt1,
        Point pt2,
        const void* color,
        LineType line_type) {
    int connectivity = 8;

    if (line_type == LineType::LINE_8) {
        connectivity = 8;
    } else if (line_type == LineType::LINE_4) {
        connectivity = 4;
    }

    LineIterator iterator(img, pt1, pt2, connectivity, true);
    int count = iterator.count;
    int pix_size = (int)img.channels() * img.type_byte_size();
    const unsigned char* color_arr = (const unsigned char*)color;

    if (pix_size == 3) {
        for (int i = 0; i < count; i++, ++iterator) {
            unsigned char* ptr = *iterator;
            ptr[0] = color_arr[0];
            ptr[1] = color_arr[1];
            ptr[2] = color_arr[2];
        }
    } else {
        for (int i = 0; i < count; i++, ++iterator) {
            unsigned char* ptr = *iterator;
            if (pix_size == 1) {
                ptr[0] = color_arr[0];
            } else {
                memcpy(*iterator, color_arr, pix_size);
            }
        }
    }
}

/* Correction table depent on the slope */
static const unsigned char slope_corr_table[] = {
    181, 181, 181, 182, 182, 183, 184, 185,
    187, 188, 190, 192, 194, 196, 198, 201,
    203, 206, 209, 211, 214, 218, 221, 224,
    227, 231, 235, 238, 242, 246, 250, 254};

/* Gaussian for antialiasing filter */
static const int filter_table[] = {
    168, 177, 185, 194, 202, 210, 218, 224, 231, 236, 241, 246, 249,
    252, 254, 254, 254, 254, 252, 249, 246, 241, 236, 231, 224, 218,
    210, 202, 194, 185, 177, 168, 158, 149, 140, 131, 122, 114, 105,
    97,  89,  82,  75,  68,  62,  56,  50,  45,  40,  36,  32,  28,
    25,  22,  19,  16,  14,  12,  11,  9,   8,   7,   5,   5};

void line_AA(
        Mat& img,
        Point2l pt1,
        Point2l pt2,
        const void* color) {
    int64_t dx = 0;
    int64_t dy = 0;
    int ecount = 0;
    int scount = 0;
    int slope = 0;
    int64_t ax = 0;
    int64_t ay = 0;
    int64_t x_step = 0;
    int64_t y_step = 0;
    int64_t i = 0;
    int64_t j = 0;
    int ep_table[9];

    int cb = ((unsigned char*)color)[0];
    int cg = ((unsigned char*)color)[1];
    int cr = ((unsigned char*)color)[2];
    int ca = ((unsigned char*)color)[3];

    int _cb = 0, _cg = 0, _cr = 0, _ca = 0;

    int nch = img.channels();
    unsigned char* ptr = (unsigned char*)img.data();
    size_t step = img.stride();
    Size2l size0(img.size().width(), img.size().height());
    Size2l size = size0;

    if (!((nch == 1 || nch == 3 || nch == 4)
            && img.type_byte_size() == 1)) {
        line_connection(img, Point((int)(pt1.x() >> XY_SHIFT), (int)(pt1.y() >> XY_SHIFT)),
                Point((int)(pt2.x() >> XY_SHIFT), (int)(pt2.y() >> XY_SHIFT)), color);
        return;
    }

    size.set_width(size.width() << XY_SHIFT);
    size.set_height(size.height() << XY_SHIFT);

    if (!clip_line(size, pt1, pt2)) return;

    dx = pt2.x() - pt1.x();
    dy = pt2.y() - pt1.y();

    j = dx < 0 ? -1 : 0;
    ax = (dx ^ j) - j;
    i = dy < 0 ? -1 : 0;
    ay = (dy ^ i) - i;

    if (ax > ay) {
        dy = (dy ^ j) - j;
        pt1.set_x(pt1.x() ^ (pt2.x() & j));
        pt2.set_x(pt2.x() ^ (pt1.x() & j));
        pt1.set_x(pt1.x() ^ (pt2.x() & j));
        pt1.set_y(pt1.y() ^ (pt2.y() & j));
        pt2.set_y(pt2.y() ^ (pt1.y() & j));
        pt1.set_y(pt1.y() ^ (pt2.y() & j));

        x_step = XY_ONE;
        y_step = (dy << XY_SHIFT) / (ax | 1);
        pt2.set_x(pt2.x() + XY_ONE);
        ecount = (int)((pt2.x() >> XY_SHIFT) - (pt1.x() >> XY_SHIFT));
        j = -(pt1.x() & (XY_ONE - 1));
        pt1.set_y(pt1.y() + ((y_step * j) >> XY_SHIFT) + (XY_ONE >> 1));
        slope = (y_step >> (XY_SHIFT - 5)) & 0x3f;
        slope ^= (y_step < 0 ? 0x3f : 0);

        /* Get 4-bit fractions for end-point adjustments */
        i = (pt1.x() >> (XY_SHIFT - 7)) & 0x78;
        j = (pt2.x() >> (XY_SHIFT - 7)) & 0x78;
    } else {
        dx = (dx ^ i) - i;
        pt1.set_x(pt1.x() ^ (pt2.x() & i));
        pt2.set_x(pt2.x() ^ (pt1.x() & i));
        pt1.set_x(pt1.x() ^ (pt2.x() & i));
        pt1.set_y(pt1.y() ^ (pt2.y() & i));
        pt1.set_y(pt2.y() ^ (pt1.y() & i));
        pt1.set_y(pt1.y() ^ (pt2.y() & i));

        x_step = (dx << XY_SHIFT) / (ay | 1);
        y_step = XY_ONE;
        pt2.set_y(pt2.y() + XY_ONE);
        ecount = (int)((pt2.y() >> XY_SHIFT) - (pt1.y() >> XY_SHIFT));
        j = -(pt1.y() & (XY_ONE - 1));
        pt1.set_x(pt1.x() + ((x_step * j) >> XY_SHIFT) + (XY_ONE >> 1));
        slope = (x_step >> (XY_SHIFT - 5)) & 0x3f;
        slope ^= (x_step < 0 ? 0x3f : 0);

        /* Get 4-bit fractions for end-point adjustments */
        i = (pt1.y() >> (XY_SHIFT - 7)) & 0x78;
        j = (pt2.y() >> (XY_SHIFT - 7)) & 0x78;
    }

    slope = (slope & 0x20) ? 0x100 : slope_corr_table[slope];

    /* Calc end point correction table */
    {
        int t0 = slope << 7;
        int t1 = ((0x78 - (int)i) | 4) * slope;
        int t2 = ((int)j | 4) * slope;

        ep_table[0] = 0;
        ep_table[8] = slope;
        ep_table[1] = ep_table[3] =
            ((((j - i) & 0x78) | 4) * slope >> 8) & 0x1ff;
        ep_table[2] = (t1 >> 8) & 0x1ff;
        ep_table[4] = ((((j - i) + 0x80) | 4) * slope >> 8) & 0x1ff;
        ep_table[5] = ((t1 + t0) >> 8) & 0x1ff;
        ep_table[6] = (t2 >> 8) & 0x1ff;
        ep_table[7] = ((t2 + t0) >> 8) & 0x1ff;
    }

    if (nch == 3) {
#define ICV_PUT_POINT(x, y)                   \
    {                                         \
        unsigned char* tptr = ptr + (x)*3 + (y)*step; \
        _cb = tptr[0];                        \
        _cb += ((cb - _cb) * a + 127) >> 8;   \
        _cb += ((cb - _cb) * a + 127) >> 8;   \
        _cg = tptr[1];                        \
        _cg += ((cg - _cg) * a + 127) >> 8;   \
        _cg += ((cg - _cg) * a + 127) >> 8;   \
        _cr = tptr[2];                        \
        _cr += ((cr - _cr) * a + 127) >> 8;   \
        _cr += ((cr - _cr) * a + 127) >> 8;   \
        tptr[0] = (unsigned char)_cb;                 \
        tptr[1] = (unsigned char)_cg;                 \
        tptr[2] = (unsigned char)_cr;                 \
    }
        if (ax > ay) {
            int x = (int)(pt1.x() >> XY_SHIFT);

            for (; ecount >= 0; x++, pt1.set_y(pt1.y() + y_step), scount++, ecount--) {
                if ((unsigned char)x >= (unsigned char)size0.width()) continue;

                int y = (int)((pt1.y() >> XY_SHIFT) - 1);

                int ep_corr = ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                        (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.y() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)y < (unsigned)size0.height()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(y + 1) < (unsigned)size0.height()) {
                    ICV_PUT_POINT(x, y + 1)
                }

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(y + 2) < (unsigned)size0.height()) {
                    ICV_PUT_POINT(x, y + 2)
                }
            }
        } else {
            int y = (int)(pt1.y() >> XY_SHIFT);

            for (; ecount >= 0; y++, pt1.set_x(pt1.x() + x_step), scount++, ecount--) {
                if ((unsigned)y >= (unsigned)size0.height()) continue;
                int x = (int)((pt1.x() >> XY_SHIFT) - 1);
                int ep_corr =
                    ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                             (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.x() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)x < (unsigned)size0.width()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(x + 1) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 1, y)

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(x + 2) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 2, y)
            }
        }
#undef ICV_PUT_POINT
    } else if (nch == 1) {
#define ICV_PUT_POINT(x, y)                 \
    {                                       \
        unsigned char* tptr = ptr + (x) + (y)*step; \
        _cb = tptr[0];                      \
        _cb += ((cb - _cb) * a + 127) >> 8; \
        _cb += ((cb - _cb) * a + 127) >> 8; \
        tptr[0] = (unsigned char)_cb;               \
    }

        if (ax > ay) {
            int x = (int)(pt1.x() >> XY_SHIFT);

            for (; ecount >= 0;
                 x++, pt1.set_y(pt1.y() + y_step), scount++, ecount--) {
                if ((unsigned)x >= (unsigned)size0.width()) continue;
                int y = (int)((pt1.y() >> XY_SHIFT) - 1);

                int ep_corr =
                    ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                             (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.y() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)y < (unsigned)size0.height()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(y + 1) < (unsigned)size0.height())
                    ICV_PUT_POINT(x, y + 1)

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(y + 2) < (unsigned)size0.height())
                    ICV_PUT_POINT(x, y + 2)
            }
        } else {
            int y = (int)(pt1.y() >> XY_SHIFT);

            for (; ecount >= 0;
                 y++, pt1.set_x(pt1.x() + x_step), scount++, ecount--) {
                if ((unsigned)y >= (unsigned)size0.height()) continue;
                int x = (int)((pt1.x() >> XY_SHIFT) - 1);
                int ep_corr =
                    ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                             (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.x() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)x < (unsigned)size0.width()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(x + 1) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 1, y)

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(x + 2) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 2, y)
            }
        }
#undef ICV_PUT_POINT
    } else {
#define ICV_PUT_POINT(x, y)                   \
    {                                         \
        unsigned char* tptr = ptr + (x)*4 + (y)*step; \
        _cb = tptr[0];                        \
        _cb += ((cb - _cb) * a + 127) >> 8;   \
        _cb += ((cb - _cb) * a + 127) >> 8;   \
        _cg = tptr[1];                        \
        _cg += ((cg - _cg) * a + 127) >> 8;   \
        _cg += ((cg - _cg) * a + 127) >> 8;   \
        _cr = tptr[2];                        \
        _cr += ((cr - _cr) * a + 127) >> 8;   \
        _cr += ((cr - _cr) * a + 127) >> 8;   \
        _ca = tptr[3];                        \
        _ca += ((ca - _ca) * a + 127) >> 8;   \
        _ca += ((ca - _ca) * a + 127) >> 8;   \
        tptr[0] = (unsigned char)_cb;                 \
        tptr[1] = (unsigned char)_cg;                 \
        tptr[2] = (unsigned char)_cr;                 \
        tptr[3] = (unsigned char)_ca;                 \
    }
        if (ax > ay) {
            int x = (int)(pt1.x() >> XY_SHIFT);

            for (; ecount >= 0;
                 x++, pt1.set_y(pt1.y() + y_step), scount++, ecount--) {
                if ((unsigned)x >= (unsigned)size0.width()) continue;
                int y = (int)((pt1.y() >> XY_SHIFT) - 1);

                int ep_corr =
                    ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                             (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.y() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)y < (unsigned)size0.height()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(y + 1) < (unsigned)size0.height())
                    ICV_PUT_POINT(x, y + 1)

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(y + 2) < (unsigned)size0.height())
                    ICV_PUT_POINT(x, y + 2)
            }
        } else {
            int y = (int)(pt1.y() >> XY_SHIFT);

            for (; ecount >= 0;
                 y++, pt1.set_x(pt1.x() + x_step), scount++, ecount--) {
                if ((unsigned)y >= (unsigned)size0.height()) continue;
                int x = (int)((pt1.x() >> XY_SHIFT) - 1);
                int ep_corr =
                    ep_table[(((scount >= 2) + 1) & (scount | 2)) * 3 +
                             (((ecount >= 2) + 1) & (ecount | 2))];
                int a = 0, dist = (pt1.x() >> (XY_SHIFT - 5)) & 31;

                a = (ep_corr * filter_table[dist + 32] >> 8) & 0xff;
                if ((unsigned)x < (unsigned)size0.width()) ICV_PUT_POINT(x, y)

                a = (ep_corr * filter_table[dist] >> 8) & 0xff;
                if ((unsigned)(x + 1) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 1, y)

                a = (ep_corr * filter_table[63 - dist] >> 8) & 0xff;
                if ((unsigned)(x + 2) < (unsigned)size0.width())
                    ICV_PUT_POINT(x + 2, y)
            }
        }
#undef ICV_PUT_POINT
    }
}

void line2(
        Mat& img,
        Point2l pt1,
        Point2l pt2,
        const void* color) {
    int64_t dx = 0;
    int64_t dy = 0;
    int ecount = 0;
    int64_t ax = 0;
    int64_t ay = 0;
    int64_t x_step = 0;
    int64_t y_step = 0;
    int64_t i = 0;
    int64_t j = 0;
    int x = 0;
    int y = 0;

    int cb = ((unsigned char*)color)[0];
    int cg = ((unsigned char*)color)[1];
    int cr = ((unsigned char*)color)[2];

    int pix_size = (int)img.channels() * img.type_byte_size();
    unsigned char* ptr = (unsigned char*)img.data();
    unsigned char* tptr = nullptr;
    size_t step = img.stride();
    Size size = img.size();
    Size2l size_scaled(((int64_t)size.width()) << XY_SHIFT,
            ((int64_t)size.height()) << XY_SHIFT);
    if (!clip_line(size_scaled, pt1, pt2)) return;

    dx = pt2.x() - pt1.x();
    dy = pt2.y() - pt1.y();

    j = dx < 0 ? -1 : 0;
    ax = (dx ^ j) - j;
    i = dy < 0 ? -1 : 0;
    ay = (dy ^ i) - i;

    if (ax > ay) {
        dy = (dy ^ j) - j;
        pt1.set_x(pt1.x() ^ (pt2.x() & j));
        pt2.set_x(pt2.x() ^ (pt1.x() & j));
        pt1.set_x(pt1.x() ^ (pt2.x() & j));
        pt1.set_y(pt1.y() ^ (pt2.y() & j));
        pt2.set_y(pt2.y() ^ (pt1.y() & j));
        pt1.set_y(pt1.y() ^ (pt2.y() & j));

        x_step = XY_ONE;
        y_step = (dy << XY_SHIFT) / (ax | 1);
        ecount = (int)((pt2.x() - pt1.x()) >> XY_SHIFT);
    } else {
        dx = (dx ^ i) - i;
        pt1.set_x(pt1.x() ^ (pt2.x() & i));
        pt2.set_x(pt2.x() ^ (pt1.x() & i));
        pt1.set_x(pt1.x() ^ (pt2.x() & i));
        pt1.set_y(pt1.y() ^ (pt2.y() & i));
        pt2.set_y(pt2.y() ^ (pt1.y() & i));
        pt1.set_y(pt1.y() ^ (pt2.y() & i));

        x_step = (dx << XY_SHIFT) / (ay | 1);
        y_step = XY_ONE;
        ecount = (int)((pt2.y() - pt1.y()) >> XY_SHIFT);
    }

    pt1.set_x(pt1.x() + (XY_ONE >> 1));
    pt1.set_y(pt1.y() + (XY_ONE >> 1));

    if (pix_size == 3) {
#define ICV_PUT_POINT(_x, _y)                                        \
    x = (_x);                                                        \
    y = (_y);                                                        \
    if (0 <= x && x < size.width() && 0 <= y && y < size.height()) { \
        tptr = ptr + y * step + x * 3;                               \
        tptr[0] = (unsigned char)cb;                                         \
        tptr[1] = (unsigned char)cg;                                         \
        tptr[2] = (unsigned char)cr;                                         \
    }

        ICV_PUT_POINT((int)((pt2.x() + (XY_ONE >> 1)) >> XY_SHIFT),
                      (int)((pt2.y() + (XY_ONE >> 1)) >> XY_SHIFT));

        if (ax > ay) {
            pt1.set_x(pt1.x() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x()), (int)(pt1.y() >> XY_SHIFT));
                pt1.set_x(pt1.x() + 1);
                pt1.set_y(pt1.y() + y_step);
                ecount--;
            }
        } else {
            pt1.set_y(pt1.y() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x() >> XY_SHIFT), (int)(pt1.y()));
                pt1.set_x(pt1.x() + x_step);
                pt1.set_y(pt1.y() + 1);
                ecount--;
            }
        }

#undef ICV_PUT_POINT
    } else if (pix_size == 1) {
#define ICV_PUT_POINT(_x, _y)                                        \
    x = (_x);                                                        \
    y = (_y);                                                        \
    if (0 <= x && x < size.width() && 0 <= y && y < size.height()) { \
        tptr = ptr + y * step + x;                                   \
        tptr[0] = (unsigned char)cb;                                         \
    }

        ICV_PUT_POINT((int)((pt2.x() + (XY_ONE >> 1)) >> XY_SHIFT),
                      (int)((pt2.y() + (XY_ONE >> 1)) >> XY_SHIFT));

        if (ax > ay) {
            pt1.set_x(pt1.x() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x()), (int)(pt1.y() >> XY_SHIFT));
                pt1.set_x(pt1.x() + 1);
                pt1.set_y(pt1.y() + y_step);
                ecount--;
            }
        } else {
            pt1.set_y(pt1.y() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x() >> XY_SHIFT), (int)(pt1.y()));
                pt1.set_x(pt1.x() + x_step);
                pt1.set_y(pt1.y() + 1);
                ecount--;
            }
        }

#undef ICV_PUT_POINT
    } else {
#define ICV_PUT_POINT(_x, _y)                                        \
    x = (_x);                                                        \
    y = (_y);                                                        \
    if (0 <= x && x < size.width() && 0 <= y && y < size.height()) { \
        tptr = ptr + y * step + x * pix_size;                        \
        for (j = 0; j < pix_size; j++) {                             \
            tptr[j] = ((unsigned char*)color)[j];                            \
        }                                                            \
    }

        ICV_PUT_POINT((int)((pt2.x() + (XY_ONE >> 1)) >> XY_SHIFT),
                      (int)((pt2.y() + (XY_ONE >> 1)) >> XY_SHIFT));

        if (ax > ay) {
            pt1.set_x(pt1.x() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x()), (int)(pt1.y() >> XY_SHIFT));
                pt1.set_x(pt1.x() + 1);
                pt1.set_y(pt1.y() + y_step);
                ecount--;
            }
        } else {
            pt1.set_y(pt1.y() >> XY_SHIFT);

            while (ecount >= 0) {
                ICV_PUT_POINT((int)(pt1.x() >> XY_SHIFT), (int)(pt1.y()));
                pt1.set_x(pt1.x() + x_step);
                pt1.set_y(pt1.y() + 1);
                ecount--;
            }
        }

#undef ICV_PUT_POINT
    }
}

static inline void icv_hline_x(
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
    } else {
        if (hline_min_ptr < hline_end_ptr) {
            memcpy(hline_ptr, color, pix_size);
            hline_ptr += pix_size;
        }

        size_t size_to_copy = pix_size;

        while (hline_ptr < hline_end_ptr) {
            memcpy(hline_ptr, hline_min_ptr, size_to_copy);
            hline_ptr += size_to_copy;
            size_to_copy = std::min(2 * size_to_copy,
                    static_cast<size_t>(hline_end_ptr - hline_ptr));
        }
    }
}

static inline void icv_hline(
        unsigned char* ptr,
        int xl,
        int xr,
        const void* color,
        int pix_size) {
    icv_hline_x(ptr, xl, xr, reinterpret_cast<const unsigned char*>(color), pix_size);
}

/* filling convex polygon. v - array of vertices, ntps - number of points */
void fill_convex_poly(
        Mat& img,
        const Point2l* v,
        int npts,
        const void* color,
        LineType line_type,
        int shift) {
    struct {
        int idx = 0;
        int di = 0;
        int64_t x = 0;
        int64_t dx = 0;
        int ye = 0;
    } edge[2];

    int delta = 1 << shift >> 1;
    int i = 0, y = 0, imin = 0;
    int edges = npts;
    int64_t xmin = 0, xmax = 0, ymin = 0, ymax = 0;
    unsigned char* ptr = (unsigned char*)img.data();
    Size size = img.size();
    int pix_size = (int)img.channels() * img.type_byte_size();
    Point2l p0;
    int delta1 = 0, delta2 = 0;

    if (line_type != LineType::LINE_AA) {
        delta1 = delta2 = XY_ONE >> 1;
    } else {
        delta1 = XY_ONE - 1, delta2 = 0;
    }

    p0 = v[npts - 1];
    p0.set_x(p0.x() << (XY_SHIFT - shift));
    p0.set_y(p0.y() << (XY_SHIFT - shift));

    xmin = xmax = v[0].x();
    ymin = ymax = v[0].y();

    for (i = 0; i < npts; i++) {
        Point2l p = v[i];
        if (p.y() < ymin) {
            ymin = p.y();
            imin = i;
        }

        ymax = std::max(ymax, p.y());
        xmax = std::max(xmax, p.x());
        xmin = std::min(xmin, p.x());

        p.set_x(p.x() << (XY_SHIFT - shift));
        p.set_y(p.y() << (XY_SHIFT - shift));

        //if (line_type <= 8) {
        if (line_type != LineType::LINE_AA) {
            if (shift == 0) {
                Point pt0, pt1;
                pt0.set_x((int)(p0.x() >> XY_SHIFT));
                pt0.set_y((int)(p0.y() >> XY_SHIFT));
                pt1.set_x((int)(p.x() >> XY_SHIFT));
                pt1.set_y((int)(p.y() >> XY_SHIFT));
                line_connection(img, pt0, pt1, color, line_type);
            } else {
                line2(img, p0, p, color);
            }
        } else {
            line_AA(img, p0, p, color);
        }
        p0 = p;
    }

    xmin = (xmin + delta) >> shift;
    xmax = (xmax + delta) >> shift;
    ymin = (ymin + delta) >> shift;
    ymax = (ymax + delta) >> shift;

    if (npts < 3 || (int)xmax < 0
            || (int)ymax < 0
            || (int)xmin >= size.width()
            || (int)ymin >= size.height()) {
        return;
    }

    ymax = std::min(ymax, int64_t(size.height() - 1));
    edge[0].idx = edge[1].idx = imin;

    edge[0].ye = edge[1].ye = y = (int)ymin;
    edge[0].di = 1;
    edge[1].di = npts - 1;

    edge[0].x = edge[1].x = -XY_ONE;
    edge[0].dx = edge[1].dx = 0;

    ptr += img.stride() * y;

    do {
        //if (line_type < CV_AA || y < (int)ymax || y == (int)ymin) {
        if (line_type != LineType::LINE_AA || y < (int)ymax || y == (int)ymin) {
            for (i = 0; i < 2; i++) {
                if (y >= edge[i].ye) {
                    int idx0 = edge[i].idx;
                    int di = edge[i].di;
                    int idx = idx0 + di;

                    if (idx >= npts) {
                        idx -= npts;
                    }

                    int ty = 0;

                    for (; edges-- > 0;) {
                        ty = (int)((v[idx].y() + delta) >> shift);
                        if (ty > y) {
                            int64_t xs = v[idx0].x();
                            int64_t xe = v[idx].x();
                            if (shift != XY_SHIFT) {
                                xs <<= XY_SHIFT - shift;
                                xe <<= XY_SHIFT - shift;
                            }

                            edge[i].ye = ty;
                            edge[i].dx = ((xe - xs) * 2 + (ty - y)) / (2 * (ty - y));
                            edge[i].x = xs;
                            edge[i].idx = idx;
                            break;
                        }
                        idx0 = idx;
                        idx += di;
                        if (idx >= npts) idx -= npts;
                    }
                }
            }
        }

        if (edges < 0) break;

        if (y >= 0) {
            int left = 0, right = 1;
            if (edge[0].x > edge[1].x) {
                left = 1, right = 0;
            }

            int xx1 = (int)((edge[left].x + delta1) >> XY_SHIFT);
            int xx2 = (int)((edge[right].x + delta2) >> XY_SHIFT);

            if (xx2 >= 0 && xx1 < size.width()) {
                if (xx1 < 0) xx1 = 0;
                if (xx2 >= size.width()) xx2 = size.width() - 1;
                icv_hline(ptr, xx1, xx2, color, pix_size);
            }
        } else {
            // TODO optimize scan for negative y
        }

        edge[0].x += edge[0].dx;
        edge[1].x += edge[1].dx;
        ptr += img.stride();
    } while (++y <= (int)ymax);
}

/* draws simple or filled circle */
void draw_circle(
        Mat& img,
        Point center,
        int radius,
        const void* color,
        int fill) {
    Size size = img.size();
    size_t step = img.stride();
    int pix_size = (int)img.type_byte_size() * img.channels();
    unsigned char* ptr = (unsigned char*)img.data();
    int err = 0;
    int dx = radius;
    int dy = 0;
    int plus = 1;
    int minus = (radius << 1) - 1;
    int inside = center.x() >= radius && center.x() < size.width() - radius &&
            center.y() >= radius && center.y() < size.height() - radius;

    #define ICV_PUT_POINT( ptr, x )     \
        memcpy( ptr + (x)*pix_size, color, pix_size );

    while (dx >= dy) {
        int mask = 0;
        int y11 = center.y() - dy;
        int y12 = center.y() + dy;
        int y21 = center.y() - dx;
        int y22 = center.y() + dx;
        int x11 = center.x() - dx;
        int x12 = center.x() + dx;
        int x21 = center.x() - dy;
        int x22 = center.x() + dy;

        if (inside) {
            unsigned char *tptr0 = ptr + y11 * step;
            unsigned char *tptr1 = ptr + y12 * step;

            if (!fill) {
                ICV_PUT_POINT(tptr0, x11);
                ICV_PUT_POINT(tptr1, x11);
                ICV_PUT_POINT(tptr0, x12);
                ICV_PUT_POINT(tptr1, x12);
            } else {
                icv_hline(tptr0, x11, x12, color, pix_size);
                icv_hline(tptr1, x11, x12, color, pix_size);
            }

            tptr0 = ptr + y21 * step;
            tptr1 = ptr + y22 * step;

            if (!fill) {
                ICV_PUT_POINT(tptr0, x21);
                ICV_PUT_POINT(tptr1, x21);
                ICV_PUT_POINT(tptr0, x22);
                ICV_PUT_POINT(tptr1, x22);
            } else {
                icv_hline(tptr0, x21, x22, color, pix_size);
                icv_hline(tptr1, x21, x22, color, pix_size);
            }
        } else if (x11 < size.width() && x12 >= 0
                && y21 < size.height() && y22 >= 0) {
            if (fill) {
                x11 = std::max(x11, 0);
                x12 = std::min(x12, size.width() - 1);
            }

            if ((unsigned)y11 < (unsigned)size.height()) {
                unsigned char *tptr = ptr + y11 * step;

                if (!fill) {
                    if (x11 >= 0) {
                        ICV_PUT_POINT(tptr, x11);
                    }

                    if (x12 < size.width()) {
                        ICV_PUT_POINT(tptr, x12);
                    }
                } else {
                    icv_hline(tptr, x11, x12, color, pix_size);
                }
            }

            if ((unsigned)y12 < (unsigned)size.height()) {
                unsigned char *tptr = ptr + y12 * step;

                if (!fill) {
                    if (x11 >= 0) {
                        ICV_PUT_POINT(tptr, x11);
                    }

                    if (x12 < size.width()) {
                        ICV_PUT_POINT(tptr, x12);
                    }
                } else {
                    icv_hline(tptr, x11, x12, color, pix_size);
                }
            }

            if (x21 < size.width() && x22 >= 0) {
                if (fill) {
                    x21 = std::max(x21, 0);
                    x22 = std::min(x22, size.width() - 1);
                }

                if ((unsigned)y21 < (unsigned)size.height()) {
                    unsigned char *tptr = ptr + y21 * step;

                    if (!fill) {
                        if (x21 >= 0) {
                            ICV_PUT_POINT(tptr, x21);
                        }

                        if (x22 < size.width()) {
                            ICV_PUT_POINT(tptr, x22);
                        }
                    } else {
                        icv_hline(tptr, x21, x22, color, pix_size);
                    }
                }

                if ((unsigned)y22 < (unsigned)size.height()) {
                    unsigned char *tptr = ptr + y22 * step;

                    if (!fill) {
                        if (x21 >= 0) {
                            ICV_PUT_POINT(tptr, x21);
                        }

                        if (x22 < size.width()) {
                            ICV_PUT_POINT(tptr, x22);
                        }
                    } else {
                        icv_hline(tptr, x21, x22, color, pix_size);
                    }
                }
            }
        }

        dy++;
        err += plus;
        plus += 2;

        mask = (err <= 0) - 1;

        err -= minus & mask;
        dx += mask;
        minus -= mask & 2;
    }

    #undef ICV_PUT_POINT
}

void draw_line(
        Mat& img,
        Point& p0,
        Point& p1,
        unsigned char* color,
        int thickness,
        LineType line_type,
        int flags,
        int shift) {
    static const double INV_XY_ONE = 1. / XY_ONE;

    Point2l pt_a(p0.x(), p0.y());
    Point2l pt_b(p1.x(), p1.y());

    pt_a.set_x(pt_a.x() << (XY_SHIFT - shift));
    pt_a.set_y(pt_a.y() << (XY_SHIFT - shift));
    pt_b.set_x(pt_b.x() << (XY_SHIFT - shift));
    pt_b.set_y(pt_b.y() << (XY_SHIFT - shift));

    if (thickness <= 1) {
        if (line_type != LineType::LINE_AA) {
            if (line_type == LineType::FILLED
                    || line_type == LineType::LINE_4 || shift == 0) {
                pt_a.set_x((pt_a.x() + (XY_ONE >> 1)) >> XY_SHIFT);
                pt_a.set_y((pt_a.y() + (XY_ONE >> 1)) >> XY_SHIFT);
                pt_b.set_x((pt_b.x() + (XY_ONE >> 1)) >> XY_SHIFT);
                pt_b.set_y((pt_b.y() + (XY_ONE >> 1)) >> XY_SHIFT);
                line_connection(img, Point(pt_a.x(), pt_a.y()),
                        Point(pt_b.x(), pt_b.y()), color, line_type);
            } else {
                line2(img, p0, p1, color);
            }
        } else {
            LOG_ERR("line type is not support");
        }
    } else {
        Point2l pt[4];
        Point2l dp = Point2l(0, 0);
        double dx = (pt_a.x() - pt_b.x()) * INV_XY_ONE;
        double dy = (pt_b.y() - pt_a.y()) * INV_XY_ONE;
        double r = dx * dx + dy * dy;
        int i = 0;
        int odd_thickness = thickness & 1;
        thickness <<= XY_SHIFT - 1;

        if (std::fabs(r) > FCV_DBL_EPSILON) {
            r = (thickness + odd_thickness * XY_ONE * 0.5) / std::sqrt(r);
            dp.set_x(fcv_round(dy * r));
            dp.set_y(fcv_round(dx * r));

            pt[0].set_x(pt_a.x() + dp.x());
            pt[0].set_y(pt_a.y() + dp.y());
            pt[1].set_x(pt_a.x() - dp.x());
            pt[1].set_y(pt_a.y() - dp.y());
            pt[2].set_x(pt_b.x() - dp.x());
            pt[2].set_y(pt_b.y() - dp.y());
            pt[3].set_x(pt_b.x() + dp.x());
            pt[3].set_y(pt_b.y() + dp.y());

            fill_convex_poly(img, pt, 4, color, line_type, XY_SHIFT);
        }

        for (i = 0; i < 2; i++) {
            if (flags & (i + 1)) {
                if (line_type != LineType::LINE_AA) {
                    Point center;
                    center.set_x((int)((pt_a.x() + (XY_ONE >> 1)) >> XY_SHIFT));
                    center.set_y((int)((pt_a.y() + (XY_ONE >> 1)) >> XY_SHIFT));
                    draw_circle(img, center, (thickness + (XY_ONE >> 1)) >> XY_SHIFT,
                           color, 1);
                } else {
                    LOG_ERR("line type is not support");
                }
            }

            p0 = p1;
        }
    }
}

void LineIterator::init(
        const Mat* img,
        Rect rect,
        Point pt1_,
        Point pt2_,
        int connectivity,
        bool left_to_right) {
    count = -1;
    p = Point(0, 0);
    ptr0 = ptr = 0;
    step = elem_size = 0;
    ptmode = !img;

    Point pt1;
    pt1.set_x(pt1_.x() - rect.x());
    pt1.set_y(pt1_.y() - rect.y());
    Point pt2;
    pt2.set_x(pt2_.x() - rect.x());
    pt2.set_y(pt2_.y() - rect.y());

    if ((unsigned)pt1.x() >= (unsigned)(rect.width())
            || (unsigned)pt2.x() >= (unsigned)(rect.width())
            || (unsigned)pt1.y() >= (unsigned)(rect.height())
            || (unsigned)pt2.y() >= (unsigned)(rect.height())) {
        if (!clip_line(Size(rect.width(), rect.height()), pt1, pt2)) {
            err = plus_delta = minus_delta = plus_step = minus_step =
                    plus_shift = minus_shift = count = 0;
            return;
        }
    }

    pt1.set_x(pt1.x() + rect.x());
    pt1.set_y(pt1.y() + rect.y());
    pt2.set_x(pt2.x() + rect.x());
    pt2.set_y(pt2.y() + rect.y());

    int delta_x = 1, delta_y = 1;
    int dx = pt2.x() - pt1.x();
    int dy = pt2.y() - pt1.y();

    if (dx < 0) {
        if (left_to_right) {
            dx = -dx;
            dy = -dy;
            pt1 = pt2;
        } else {
            dx = -dx;
            delta_x = -1;
        }
    }

    if (dy < 0) {
        dy = -dy;
        delta_y = -1;
    }

    bool vert = dy > dx;
    if (vert) {
        std::swap(dx, dy);
        std::swap(delta_x, delta_y);
    }

    if (connectivity == 8) {
        err = dx - (dy + dy);
        plus_delta = dx + dx;
        minus_delta = -(dy + dy);
        minus_shift = delta_x;
        plus_shift = 0;
        minus_step = 0;
        plus_step = delta_y;
        count = dx + 1;
    } else /* connectivity == 4 */ {
        err = 0;
        plus_delta = (dx + dx) + (dy + dy);
        minus_delta = -(dy + dy);
        minus_shift = delta_x;
        plus_shift = -delta_x;
        minus_step = 0;
        plus_step = delta_y;
        count = dx + dy + 1;
    }

    if (vert) {
        std::swap(plus_step, plus_shift);
        std::swap(minus_step, minus_shift);
    }

    p = pt1;
    if (!ptmode) {
        ptr0 = (const unsigned char*)img->data();
        step = (int)img->stride();
        elem_size = (int)img->channels() * img->type_byte_size();
        ptr = (unsigned char*)ptr0 + (size_t)p.y() * step + (size_t)p.x() * elem_size;
        plus_step = plus_step * step + plus_shift * elem_size;
        minus_step = minus_step * step + minus_shift * elem_size;
    }
}

int line_common(
        Mat& img,
        Point& pt1,
        Point& pt2,
        const Scalar& color,
        int thickness,
        LineType line_type,
        int shift) {
    unsigned char buf[4];

    for (int i = 0; i < 4; ++i) {
        buf[i] = fcv_cast_u8(color[i]);
    }

    draw_line(img, pt1, pt2, buf, thickness, line_type, 3, shift);

    return 0;
}


G_FCV_NAMESPACE1_END()
