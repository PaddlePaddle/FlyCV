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

#pragma once

#include <vector>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <ostream>

#include "flycv_namespace.h"
#include "macro_export.h"
#include "macro_ns.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/**
 * @brief Image types.
 * U8  - unsigned 1 byte integer
 * U16 - unsigned 2 byte integer
 * S16 - signed 2 byte integer
 * S32 - signed 4 byte integer
 * F32 - 4 byte float
 * F64 - 8 byte float
 */
enum class FCVImageType {
    GRAY_U8 = 0,
    GRAY_U16,
    GRAY_S16,
    GRAY_S32,
    GRAY_F32,
    GRAY_F64,
    PLA_BGR_U8 = 20,
    PLA_RGB_U8,
    PKG_BGR_U8,
    PKG_RGB_U8,
    PLA_BGRA_U8,
    PLA_RGBA_U8,
    PKG_BGRA_U8,
    PKG_RGBA_U8,
    PLA_BGR_F32 = 40,
    PLA_RGB_F32,
    PKG_BGR_F32,
    PKG_RGB_F32,
    PLA_BGRA_F32,
    PLA_RGBA_F32,
    PKG_BGRA_F32,
    PKG_RGBA_F32,
    PKG_BGR_F64,
    PKG_RGB_F64,
    PKG_BGRA_F64,
    PKG_RGBA_F64,
    PKG_BGR565_U8,
    PKG_RGB565_U8,
    NV12 = 60,
    NV21,
    I420,
};

/**
 * @brief Computing platform types.
 */
enum class PlatformType {
    CPU = 0,
    METAL,
    CUDA,
    CUDA_POOL,
    OPENCL,
    RV1109,
    AMBA_CV25,
    HISI3516D
};

/**
 * @brief interpolation types.
 */
enum class InterpolationType {
    INTER_NEAREST = 0,      // nearest interpolation
    INTER_LINEAR,           // bilinear interpolation
    INTER_CUBIC,            // bicubic interpolation
    INTER_AREA,             // area interpolation
    WARP_INVERSE_MAP = 16   // inverse transformation interpolation, the type means that M matrix is the inverse transformation
};

/**
 * @brief make border types, image boundaries are denoted with
 */
enum class BorderType {
    BORDER_CONSTANT    = 0, //!< `iiiiii|abcdefgh|iiiiiii`  with some specified `i`
    BORDER_REPLICATE   = 1, //!< `aaaaaa|abcdefgh|hhhhhhh`
    BORDER_REFLECT     = 2, //!< `fedcba|abcdefgh|hgfedcb`
    BORDER_WRAP        = 3, //!< `cdefgh|abcdefgh|abcdefg`
    BORDER_REFLECT_101 = 4, //!< `gfedcb|abcdefgh|gfedcba`
    BORDER_TRANSPARENT = 5  //!< `uvwxyz|abcdefgh|ijklmno`
};

/**
 * @brief calculate norm types
 */
enum class NormType {
    NORM_INF = 0,   // the max value of absolute value of src, for example: max =(|-1|, |2|) = 2
    NORM_L1,        // the sum of absolute value of src, for example: sum = |-1| + |2| = 3
    NORM_L2         // the Euclidean distance of src, for example: sum = sqrt((-1)^2 + (2)^2) = 5
};

/**
 * @brief template class for Size
 */
template<class T>
class FCV_CLASS Size_ {
public:
    Size_();
    Size_(T width, T height);
    Size_(const Size_& sz);

    void set_width(T width);
    void set_height(T height);

    T width() const;
    T height() const;

private:
    T _width;
    T _height;
};

typedef Size_<int> Size2i;
typedef Size_<int64_t> Size2l;
typedef Size_<float> Size2f;
typedef Size_<double> Size2d;
typedef Size2i Size;

/**
 * @brief template class for Rect
 */
template<class T>
class FCV_CLASS Rect_ {
public:
    Rect_();
    Rect_(T x, T y, T width, T height);
    Rect_(const Rect_& rectangle) = default;
    ~Rect_() = default;

    Rect_& operator=(const Rect_& rectangle) = default;

    void set_x(T x);
    void set_y(T y);
    void set_width(T width);
    void set_height(T height);

    T x() const;
    T y() const;
    T width() const;
    T height() const;

private:
    T _x;
    T _y;
    T _width;
    T _height;
};

typedef Rect_<int> RectI;
typedef Rect_<float> RectF;
typedef Rect_<double> RectD;
typedef RectI Rect;

/**
 * @brief template class for 2d points
 */
template<class T>
class FCV_CLASS Point_ {
public:
    Point_();
    Point_(T x, T y);
    Point_(const Point_& p) = default;

    Point_& operator=(const Point_& p) = default;
    template<class _Tp2> operator Point_<_Tp2>() const {
        return Point_<_Tp2>(_Tp2(_x), _Tp2(_y));
    }
    ~Point_() = default;

    void set_x(T x);
    void set_y(T y);

    T x() const;
    T y() const;

private:
    T _x;
    T _y;
};

typedef Point_<int> Point;
typedef Point_<int64_t> Point2l;
typedef Point Point2i;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;

/**
 * @brief  class for rotated rectangle on a plan
 */
class FCV_CLASS RotatedRect {
public:
    RotatedRect();

    RotatedRect(
            const float& center_x,
            const float& center_y,
            const float& width,
            const float& height,
            const float& angle);

    RotatedRect(const Point2f& center, const Size2f& size, const float& angle);

    RotatedRect(const RotatedRect&) = default;
    ~RotatedRect() = default;

    void set_center(const Point2f& center);
    void set_center_x(const float& center_x);
    void set_center_y(const float& center_y);
    void set_size(const Size2f& size);
    void set_width(const float& width);
    void set_height(const float& height);
    void set_angle(const float& angle);
    void points(std::vector<Point2f>& pts);

    Point2f center() const;
    float center_x() const;
    float center_y() const;
    Size2f size() const;
    float width() const;
    float height() const;
    float angle() const;

private:
    float _center_x;
    float _center_y;
    float _width;
    float _height;
    float _angle;
};

/**
 * @brief class for hold an array of values.
 */
template<class T>
class FCV_CLASS Scalar_ {
public:
    //! default constructor
    Scalar_();
    Scalar_(T v0, T v1, T v2 = 0, T v3 = 0);
    Scalar_(T v0);

    Scalar_(const Scalar_& s);

    Scalar_& operator= (const Scalar_& s);

    T& operator[] (int index);
    const T& operator[] (int index) const;

    int set_val(int index, T val);
    const T* val() const;

    //! returns a scalar with all elements set to v0
    static Scalar_<T> all(T v0);

private:
    T _val[4];
};

typedef Scalar_<double> Scalar;

template<typename T>
static inline std::ostream& operator << (std::ostream& out_stream, const Size_<T>& size) {
    return out_stream << "<width = " << size.width() << ", height = " << size.height() << ">";
}

G_FCV_NAMESPACE1_END()
