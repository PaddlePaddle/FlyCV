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
template<class T>
Size_<T>::Size_() : _width(static_cast<T>(0)), _height(static_cast<T>(0)) {}

template<class T>
Size_<T>::Size_(T width, T height) :
        _width(width),
        _height(height) {}

template<class T>
Size_<T>::Size_(const Size_& sz) :
        _width(static_cast<T>(sz._width)),
        _height(static_cast<T>(sz._height)) {}

template<class T>
void Size_<T>::set_width(T width) {
    _width = width;
}

template<class T>
void Size_<T>::set_height(T height) {
    _height = height;
}

template<class T>
T Size_<T>::width() const {
    return _width;
}

template<class T>
T Size_<T>::height() const {
    return _height;
}

template class FCV_API Size_<int>;
template class FCV_API Size_<int64_t>;
template class FCV_API Size_<float>;
template class FCV_API Size_<double>;

// Class rect implement
template<class T>
Rect_<T>::Rect_() :
        _x(static_cast<T>(0)),
        _y(static_cast<T>(0)),
        _width(static_cast<T>(0)),
        _height(static_cast<T>(0)) {}

template<class T>
Rect_<T>::Rect_(T x, T y, T width, T height) :
        _x(static_cast<T>(x)),
        _y(static_cast<T>(y)),
        _width(static_cast<T>(width)),
        _height(static_cast<T>(height)) {}

template<class T>
void Rect_<T>::set_x(T x) {
    _x = x;
}

template<class T>
void Rect_<T>::set_y(T y) {
    _y = y;
}

template<class T>
void Rect_<T>::set_width(T width) {
    _width = width;
}

template<class T>
void Rect_<T>::set_height(T height) {
    _height = height;
}

template<class T>
T Rect_<T>::x() const {
    return _x;
}

template<class T>
T Rect_<T>::y() const {
    return _y;
}

template<class T>
T Rect_<T>::width() const {
    return _width;
}

template<class T>
T Rect_<T>::height() const {
    return _height;
}

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
template<class T>
inline Scalar_<T>::Scalar_() {
    _val[0] = _val[1] = _val[2] = _val[3] = 0;
}

template<class T>
inline Scalar_<T>::Scalar_(T v0, T v1, T v2, T v3) {
    _val[0] = v0;
    _val[1] = v1;
    _val[2] = v2;
    _val[3] = v3;
}

template<class T>
inline Scalar_<T>::Scalar_(const Scalar_& s) {
    _val[0] = s[0];
    _val[1] = s[1];
    _val[2] = s[2];
    _val[3] = s[3];
}

template<class T>
inline Scalar_<T>::Scalar_(T v0) {
    _val[0] = v0;
    _val[1] = _val[2] = _val[3] = 0;
}

template<class T> inline
Scalar_<T>& Scalar_<T>::operator=(const Scalar_<T>& s) {
    _val[0] = s.val()[0];
    _val[1] = s.val()[1];
    _val[2] = s.val()[2];
    _val[3] = s.val()[3];
    return *this;
}

template<class T>
T& Scalar_<T>::operator [] (int index) {
    return _val[index < 0 ? 0 : (index > 4 ? 4 : index)];
}

template<class T>
const T& Scalar_<T>::operator [] (int index) const{
    return _val[index < 0 ? 0 : (index > 4 ? 4 : index)];
}

template<class T>
int Scalar_<T>::set_val(int index, T val) {
    if (index < 0 || index > 3) {
        return -1;
    }

    _val[index] = val;
    return 0;
}

template<class T>
const T* Scalar_<T>::val() const {
    return _val;
}

template<class T>
inline Scalar_<T> Scalar_<T>::all(T v0) {
    return Scalar_<T>(v0, v0, v0, v0);
}

template class FCV_API Scalar_<int>;
template class FCV_API Scalar_<float>;
template class FCV_API Scalar_<double>;

G_FCV_NAMESPACE1_END()
