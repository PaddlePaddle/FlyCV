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

#include "modules/img_transform/flip/include/flip_common.h"
#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

/*
 1 2 3
 4 5 6
 7 8 9
---------- flip_x
 7 8 9
 4 5 6
 1 2 3
*/
template<typename T>
class FlipXCTask : public ParallelTask {
public:
    FlipXCTask(const T * src,
            int src_h,
            int src_w,
            int sc,
            int sstep,
            T* dst,
            int dstep) :
            _src(src),
            _src_h(src_h),
            _src_w(src_w),
            _sc(sc),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep) {}

    void operator() (const Range & range) const {    
        const T* ptr_src = _src + range.start() * _sstep ;
        T * ptr_dst = _dst + (_src_h - 1 - range.start()) * _dstep ;
        int size = range.size();
        int width = _src_w * _sc;
        int i = 0, j=0;
        for (; i < size; i++) {
            const T * src_row = ptr_src;
            T* dst_row = ptr_dst;
            for (j=0; j < width; j++) {
                *(dst_row++) = *(src_row++);
            }
            ptr_dst -= _dstep;
            ptr_src += _sstep;
        }
    }
private:
    const T * _src;  
    int _src_h;
    int _src_w;
    int _sc;
    int _sstep;
    T * _dst;
    int _dstep;
};

/*
 1 2 3  | 3 2 1
 4 5 6  | 6 5 4
 7 8 9  | 9 8 7
*****flip_y*****
*/
template<typename T>
class FlipYCTask : public ParallelTask {
public:
    FlipYCTask(const T* src,
            int src_h,
            int src_w,
            int sc,
            int sstep,
            T* dst,
            int dstep) :
            _src(src),
            _src_h(src_h),
            _src_w(src_w),
            _sc(sc),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep) {}
    
    void operator() (const Range & range) const {
        const T* ptr_src = _src + range.start() * _sstep;
        T* ptr_dst = _dst + range.start() * _dstep;
        int size = range.size();
        int width = _src_w;
        int i = 0, j=0;

        if (1 == _sc) {
                
            for (i = 0; i < size; i++) {
                const T* src_row = ptr_src;
                T* dst_row = ptr_dst + _dstep - _sc;

                for (j = 0; j < width; j++) {
                    *(dst_row--) = *(src_row++);
                }

                ptr_dst += _dstep;
                ptr_src += _sstep;
            }

        } else if (3 == _sc) {
            for (i = 0; i < size; i++) {
                const T* src_row = ptr_src;
                T* dst_row = ptr_dst + _dstep - _sc;

                for (j = 0; j < width; j++) {
                    *(dst_row++) = *(src_row++);
                    *(dst_row++) = *(src_row++);
                    *(dst_row++) = *(src_row++);

                    dst_row -= 6;
                }

                ptr_dst += _dstep;
                ptr_src += _sstep;
            }
        } else if (4 == _sc) {
            for (; i < size; i++) {
                const T* src_row = ptr_src;
                T* dst_row = ptr_dst + _dstep - _sc;

                for (j = 0; j < width; j++) {
                    *(dst_row++) = *(src_row++);
                    *(dst_row++) = *(src_row++);
                    *(dst_row++) = *(src_row++);
                    *(dst_row++) = *(src_row++);

                    dst_row -= 8;
                }

                ptr_dst += _dstep;
                ptr_src += _sstep;
            }
        } else {
            LOG_ERR( "flip y not support the channel number yet !");
        }
    }

private:
    const T * _src;  
    int _src_h;
    int _src_w;
    int _sc;
    int _sstep;
    T * _dst;
    int _dstep;
};

/*
 u0 v0 u1 v1 u2 v2
 u3 v3 u4 v4 u5 v5
 u6 v6 u7 v7 u8 v8
---------- flip_x
 u6 v6 u7 v7 u8 v8
 u3 v3 u4 v4 u5 v5
 u0 v0 u1 v1 u2 v2
*/
template<typename T>
class FlipUVXTask : public ParallelTask {
public:
    FlipUVXTask(const T* src,
            int src_h,
            int src_w,
            int sstep,
            T* dst,
            int dstep) :
            _src(src),
            _src_h(src_h),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){}

    void operator() (const Range & range) const {
        const T* ptr_src = _src + range.start() * _sstep;
        T * ptr_dst = _dst + (_src_h - 1 - range.start()) * _sstep;
        int width = range.size();
        int i = 0, j = 0;

        for(; i < width; i++) {
            const T * src_row = ptr_src;
            T* dst_row = ptr_dst;

            for (j = 0; j < _src_w; j++) {
                dst_row[0] = src_row[0];
                dst_row[1] = src_row[1];
                dst_row += 2;
                src_row += 2;
            }

            ptr_dst -= _dstep;
            ptr_src += _sstep;
        }
    }        

private:
    const T* _src;
    int _src_h;
    int _src_w;
    int _sstep;
    T* _dst;
    int _dstep;
};

/*
 u0 v0 u1 v1 u2 v2 | u2 v2 u1 v1 u0 v0
 u3 v3 u4 v4 u5 v5 | u5 v5 u4 v4 u3 v3
 u6 v6 u7 v7 u8 v8 | u8 v8 u7 v7 u6 v6
*****uv_flip_y*****
*/
template<typename T>
class FlipUVYTask : public ParallelTask {
public:
    FlipUVYTask(const T* src,
            int src_h,
            int src_w,
            int sstep,
            T* dst,
            int dstep) :
            _src(src),
            _src_h(src_h),
            _src_w(src_w),
            _sstep(sstep),
            _dst(dst),
            _dstep(dstep){}

    void operator() (const Range & range) const {
        const T* ptr_src = _src + range.start() * _sstep;
        T* ptr_dst = _dst + range.start() * _dstep;
        int width = range.size();
        int i=0, j=0;

        for(; i< width; i++){
            const T* src_row = ptr_src;
            T* dst_row = ptr_dst + _dstep - 2;
            
            for (j=0; j < _src_w; j++) {
                dst_row[0] = src_row[0];
                dst_row[1] = src_row[1];
                dst_row -= 2;
                src_row += 2;
            }
            ptr_dst += _dstep;
            ptr_src += _sstep;
        }
    }        

private:
    const T* _src;
    int _src_h;
    int _src_w;
    int _sstep;
    T* _dst;
    int _dstep;
};

template<typename T>
void flip_c(
        const T* src,
        int src_h,
        int src_w,
        int sc,
        int sstep,
        T* dst,
        int dstep,
        FlipType type) {
    if (FlipType::X == type) {
        FlipXCTask<T> task(src, src_h, src_w, sc, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else if (FlipType::Y == type) {
        FlipYCTask<T> task(src, src_h, src_w, sc, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else {
        LOG_ERR( "flip type not support yet !");
    }
    return;
}

template<typename T>
void flip_uv_c(
        const T* src,
        int src_h,
        int src_w,
        int sstep,
        T* dst,
        int dstep,
        FlipType type) {
    if (FlipType::X == type) {
        FlipUVXTask<T> task(src, src_h, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);        
    } else if (FlipType::Y == type) {
        FlipUVYTask<T> task(src, src_h, src_w, sstep, dst, dstep);
        parallel_run(Range(0, src_h), task);
    } else {
        LOG_ERR( "flip type not support yet !");
    }
    return;
}

int flip_common(const Mat& src, Mat& dst, FlipType type) {
    const int src_w = src.width();
    const int src_h = src.height();
    const void *src_ptr = (const void *)src.data();
    void*dst_ptr = (void *)dst.data();

    int sc = src.channels();
    const int s_stride = src.stride();
    const int d_stride = dst.stride();

    switch (src.type()) {
    case FCVImageType::GRAY_U8:
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        flip_c((unsigned char*)src_ptr, src_h, src_w, sc, s_stride, 
        (unsigned char*)dst_ptr, d_stride, type);
        break;
    case FCVImageType::GRAY_F32:
    case FCVImageType::PKG_RGB_F32:
    case FCVImageType::PKG_BGR_F32:
    case FCVImageType::PKG_RGBA_F32:
    case FCVImageType::PKG_BGRA_F32:
        flip_c((float*)src_ptr, src_h, src_w, sc, s_stride / sizeof(float),
                (float*)dst_ptr, d_stride / sizeof(float), type);
        break;

    case FCVImageType::NV12:
    case FCVImageType::NV21:
        flip_c((unsigned char*)src_ptr, src_h, src_w, 1,
                s_stride, (unsigned char *)dst_ptr, d_stride, type);
        flip_uv_c(((unsigned char*)src_ptr + src_w * src_h), (src_h >> 1),
                (src_w >> 1), s_stride, ((unsigned char *)dst_ptr + src_w * src_h), d_stride, type);
        break;
    default:
        LOG_ERR("flip type not support yet!");
        break;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
