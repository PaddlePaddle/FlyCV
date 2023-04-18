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
#include <stdint.h>

#include "flycv_namespace.h"
#include "modules/core/allocator/interface/base_allocator.h"
#include "modules/core/base/interface/basic_types.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/base/interface/log.h"
#include "modules/core/basic_math/interface/basic_math.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class FCV_API Mat {
public:
    Mat();

    Mat(int width,
            int height,
            FCVImageType type,
            void* data,
            int stride = 0);

    Mat(int width,
            int height,
            FCVImageType type,
            std::vector<uint64_t*>& phy_addrs,
            std::vector<uint64_t*>& vir_addrs,
            int stride = 0);

    Mat(int width,
            int height,
            FCVImageType type,
            int stride = 0,
            PlatformType platform = PlatformType::CPU);

    Mat(Size size,
            FCVImageType type,
            int stride = 0,
            PlatformType platform = PlatformType::CPU);

    ~Mat();

    int width() const;
    int height() const;
    Size2i size() const;
    int channels() const;
    int stride() const;
    FCVImageType type() const;
    int type_byte_size() const;
    uint64_t total_byte_size() const;
    bool empty() const;

    void* data() const;
    uint64_t* phy_data() const;
    uint64_t* vir_data() const;

    Mat clone() const;

    template<typename T>
    T& at(int x, int y, int c = 0) {
        return *reinterpret_cast<T*>(_get_pixel_address(x, y, c));
    }

    template<typename T>
    const T& at(int x, int y, int c = 0) const {
        return *reinterpret_cast<T*>(_get_pixel_address(x, y, c));
    }

    template<typename T>
    T* ptr(int x, int y, int c = 0) {
        return reinterpret_cast<T*>(_get_pixel_address(x, y, c));
    }

    template<typename T>
    const T* ptr(int x, int y, int c = 0) const {
        return reinterpret_cast<T*>(_get_pixel_address(x, y, c));
    }

    /** @brief Converts an array to another data type with optional scaling.
    The method converts source pixel values to the target data type. saturate_cast\<\> is applied at
    the end to avoid possible overflows:
    \f[m(x,y) = saturate \_ cast<rType>( \scale (*this)(x,y) +  \shift )\f]
    @param dst output mat; if it does not have a proper size or type before the operation, it is
    reallocated.
    @param rtype desired output matrix type
    @param scale optional scale factor.
    @param shift optional delta added to the scaled values.
     */

    int convert_to(Mat& dst, FCVImageType dst_type, double scale = 1.0, double shift = 0.0) const;

    /** @brief Copies the Mat to another one.
    @param dst Destination matrix. If it does not have a proper size or type before the operation, it is
    reallocated.
     */
    void copy_to(Mat& dst) const;

    /** @overload
    @param dst Destination matrix. If it does not have a proper size or type before the operation, it is
    reallocated.
    @param mask Operation mask of the same size as \*this. Its non-zero elements indicate which matrix
    elements need to be copied. The mask has to be of type unsigned char and can have 1 or multiple channels.
    */
    int copy_to(Mat& dst, Mat& mask) const;

     /** @overload
      * copy src to the area oriented of dst, so the size of dst cannot samller than src's.
    @param dst Destination matrix. If it does not have a proper size or type before the operation, it is
    return.
    @param rect dst rect Rect_(T x, T y, T width, T height).
    */
    int copy_to(Mat& dst, Rect& rect) const;

    /** @brief Computes a dot-product of two vectors.
    The method computes a dot-product of two matrices. The vectors must have the same size and type. If the matrices have more than one channel,
    the dot products from all the channels are summed together.
    @param m another dot-product operand.
     */
    double dot(Mat& m) const;

    /** @brief Compute the inverse of a matrix.
    The method performs a matrix inversion by means of matrix expressions. This means that a temporary
    matrix inversion object is returned by the method and can be used further as a part of more complex
    matrix expressions or can be assigned to a matrix.
     */
    bool invert(Mat& dst) const;

private:
    int _initialize();
    void* _get_pixel_address(int x, int y, int c) const;

private:
    int _width;
    int _height;
    int _channels;
    int _stride;
    uint64_t _total_byte_size;
    FCVImageType _type;
    int _type_byte_size;
    PlatformType _platform;
    void* _data;
    std::vector<uint64_t*> _phy_addrs;
    std::vector<uint64_t*> _vir_addrs;
    std::shared_ptr<BaseAllocator> _allocator;
    int _pixel_offset;
    int _channel_offset;
};

G_FCV_NAMESPACE1_END()