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

#include "modules/fusion_api/y420sp_to_resize_to_bgr/include/y420sp_to_resize_to_bgr_common.h"

#include <stdlib.h>

#include <iostream>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"
#include "modules/img_transform/resize/include/resize_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class Y420spToResizeBilinearToBgrCommon : public ParallelTask {
public:
    Y420spToResizeBilinearToBgrCommon(
            unsigned char* src, int src_w, int src_h, unsigned char* dst, int dst_w, int dst_h, bool is_nv12)
            : _src_ptr(src),
              _dst_ptr(dst),
              _src_w(src_w),
              _src_h(src_h),
              _dst_w(dst_w),
              _dst_h(dst_h),
              _dst_s(dst_w),
              _is_nv12(is_nv12) {
        _buf_size = (_dst_w + _dst_h) << 3;  //(dst_w + dst_h) * 2 * sizeof(int)
        _scale_x = (double)_src_w / _dst_w;
        _scale_y = (double)_src_h / _dst_h;

        _buf = (int*)malloc(_buf_size);
        memset(_buf, 0, _buf_size);

        get_resize_bilinear_buf(_src_w, _src_h, _dst_w, _dst_h, 1, &_buf);
        _xofs = _buf;
        _yofs = _buf + _dst_w;
        _alpha = (unsigned short*)(_yofs + _dst_h);
        _beta = (unsigned short*)(_alpha + _dst_w + _dst_w);
    }

    ~Y420spToResizeBilinearToBgrCommon() {
        if (_buf != nullptr) {
            free(_buf);
            _buf = nullptr;
        }
    }

    void operator()(const Range& range) const {
        unsigned char* src_ptr = _src_ptr;
        unsigned char* dst_ptr = _dst_ptr;
        unsigned char* src_uv_ptr = src_ptr + _src_w * _src_h;

        const int d1_stride = _dst_s;
        const int d2_stride = d1_stride << 1;
        const int d3_stride = d1_stride + d2_stride;

        int rows_size = d1_stride << 3;
        unsigned short* rows = (unsigned short*)malloc(rows_size);
        memset(rows, 0, rows_size);

        unsigned short* rows00 = nullptr;
        unsigned short* rows01 = nullptr;
        unsigned short* rows10 = nullptr;
        unsigned short* rows11 = nullptr;
        unsigned char* ptr_dst0 = nullptr;
        unsigned char* ptr_dst1 = nullptr;

        int dy = range.start(), dx = 0, sy0 = 0, sy1 = 0, channel = 3;

        // std::cout << "range start: " << range.start()  << "end " << range.end() << std::endl;
        for (; dy < range.end(); dy += 2) {
            const int dst_offset = d3_stride * dy;
            rows00 = rows;
            rows01 = rows + d1_stride;
            rows10 = rows + d2_stride;
            rows11 = rows + d3_stride;
            sy0 = *(_yofs + dy);
            sy1 = *(_yofs + dy + 1);

            unsigned short* alphap = _alpha;
            ptr_dst0 = dst_ptr + dst_offset;
            ptr_dst1 = dst_ptr + dst_offset + d3_stride;

            hresize_bn_one_row(src_ptr, _xofs, sy0, _src_w, _dst_w, 1, alphap, rows00, rows01);
            hresize_bn_one_row(src_ptr, _xofs, sy1, _src_w, _dst_w, 1, alphap, rows10, rows11);

            unsigned int b0 = *(_beta + dy * 2);
            unsigned int b1 = *(_beta + dy * 2 + 1);

            unsigned int b2 = *(_beta + dy * 2 + 2);
            unsigned int b3 = *(_beta + dy * 2 + 3);

            int src_y = (int)((dy >> 1) * _scale_y + 0.5);
            unsigned char uv0 = 0, uv1 = 0, y00 = 0, y10 = 0, y01 = 0, y11 = 0;
            unsigned char* uv_ptr = src_uv_ptr + src_y * _src_w;

            for (dx = 0; dx < _dst_w; dx += 2) {
                int src_x = (int)((dx / 2) * _scale_x + 0.5) * 2;
                // calculate y
                y00 = static_cast<unsigned char>((*(rows00++) * b0 + *(rows01++) * b1 + (1 << 17)) >> 18);
                y01 = static_cast<unsigned char>((*(rows00++) * b0 + *(rows01++) * b1 + (1 << 17)) >> 18);
                y10 = static_cast<unsigned char>((*(rows10++) * b2 + *(rows11++) * b3 + (1 << 17)) >> 18);
                y11 = static_cast<unsigned char>((*(rows10++) * b2 + *(rows11++) * b3 + (1 << 17)) >> 18);

                // calculate uv, nearest interpolation
                uv0 = _is_nv12 ? uv_ptr[src_x] : uv_ptr[src_x + 1];  // u
                uv1 = _is_nv12 ? uv_ptr[src_x + 1] : uv_ptr[src_x];  // v

                int vr = uv1 * 102 - 14216;
                int ub = 129 * uv0 - 17672;
                int uvg = 8696 - 52 * uv1 - 25 * uv0;

                convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
                convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
            }
        }

        if (rows != nullptr) {
            free(rows);
            rows = nullptr;
        }
    }

private:
    unsigned char* _src_ptr;
    unsigned char* _dst_ptr;
    int _src_w;
    int _src_h;
    int _dst_w;
    int _dst_h;
    int _dst_s;
    bool _is_nv12;
    int _buf_size;
    int* _buf;
    double _scale_x;
    double _scale_y;
    int* _xofs;
    int* _yofs;
    unsigned short* _alpha;
    unsigned short* _beta;
};

int y420sp_to_resize_bilinear_to_bgr_common(Mat& src, Mat& dst, bool is_nv12) {
    Y420spToResizeBilinearToBgrCommon task((unsigned char*)src.data(),
            src.width(),
            src.height(),
            (unsigned char*)dst.data(),
            dst.width(),
            dst.height(),
            is_nv12);

    parallel_run(Range(0, dst.height()), task, dst.height() / 2);

    return 0;
}

class Y420spToResizeNearestToBgrCommon : public ParallelTask {
public:
    Y420spToResizeNearestToBgrCommon(unsigned char* src,
                                     int src_w,
                                     int src_h,
                                     int src_s,
                                     unsigned char* dst,
                                     int dst_w,
                                     int dst_h,
                                     int dst_s,
                                     bool is_nv12)
            : _src_ptr(src),
              _dst_ptr(dst),
              _src_w(src_w),
              _src_h(src_h),
              _src_s(src_s),
              _dst_w(dst_w),
              _dst_h(dst_h),
              _dst_s(dst_s),
              _is_nv12(is_nv12) {
        _buf_size = (dst_w + dst_h) << 3;  //(dst_w + dst_h + (dst_w + 1) / 2 +
                                           //(dst_h + 1) / 2) * sizeof(int)
        _buf = (int*)malloc(_buf_size);

        _y_xofs = _buf;
        _y_yofs = _buf + dst_w;

        _uv_xofs = _y_yofs + dst_h;
        _uv_yofs = _uv_xofs + (dst_w + 1) / 2;

        nearest_cal_offset(_y_xofs, dst_w, src_w);
        nearest_cal_offset(_y_yofs, dst_h, src_h);

        nearest_cal_offset(_uv_xofs, (dst_w + 1) / 2, (src_w + 1) / 2);
        nearest_cal_offset(_uv_yofs, (dst_h + 1) / 2, (src_h + 1) / 2);
    }

    ~Y420spToResizeNearestToBgrCommon() {
        if (_buf != nullptr) {
            free(_buf);
            _buf = nullptr;
        }
    }

    void operator()(const Range& range) const {
        const int src_w = _src_w;
        const int src_h = _src_h;
        const int dst_w = _dst_w;
        const int dst_h = _dst_h;
        const int src_stride = _src_s;
        const int dst_stride = _dst_s;
        unsigned char* src_ptr = _src_ptr;
        unsigned char* dst_ptr = _dst_ptr;
        unsigned char* src_uv_ptr = src_ptr + src_w * src_h;

        int dy = range.start(), dx = 0, channel = 3;
        // std::cout << "start: " << range.start() << " end: " << range.end() << std::endl;
        for (; dy < range.end(); dy += 2) {
            const unsigned char* src0 = src_ptr + (_y_yofs[dy] >> 16) * src_stride;
            const unsigned char* src1 = src_ptr + (_y_yofs[dy + 1] >> 16) * src_stride;
            int uv_dy = _uv_yofs[dy >> 1] >> 16;
            unsigned char* uv_ptr = src_uv_ptr + uv_dy * src_stride;

            unsigned char* ptr_dst0 = dst_ptr + dst_stride * dy;
            unsigned char* ptr_dst1 = ptr_dst0 + dst_stride;

            unsigned char uv0 = 0, uv1 = 0, y00 = 0, y10 = 0, y01 = 0, y11 = 0;
            for (dx = 0; dx < dst_w; dx += 2) {
                int idx0 = _y_xofs[dx] >> 16;
                int idx1 = _y_xofs[dx + 1] >> 16;

                int uv_idx = _uv_xofs[dx >> 1] >> 15;  // (x >> 16) << 1
                // calculate y, nearest interpolation
                y00 = src0[idx0];
                y01 = src0[idx1];

                y10 = src1[idx0];
                y11 = src1[idx1];

                // calculate uv
                uv0 = _is_nv12 ? uv_ptr[uv_idx] : uv_ptr[uv_idx + 1];  // u
                uv1 = _is_nv12 ? uv_ptr[uv_idx + 1] : uv_ptr[uv_idx];  // v

                // calculate uv coefficients
                int vr = uv1 * 102 - 14216;
                int ub = 129 * uv0 - 17672;
                int uvg = 8696 - 52 * uv1 - 25 * uv0;

                convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
                convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
            }
        }
    }

private:
    unsigned char* _src_ptr;
    unsigned char* _dst_ptr;
    int _src_w;
    int _src_h;
    int _src_s;
    int _dst_w;
    int _dst_h;
    int _dst_s;
    bool _is_nv12;
    int _buf_size;
    int* _buf;
    int* _y_xofs;
    int* _y_yofs;
    int* _uv_xofs;
    int* _uv_yofs;
};

int y420sp_to_resize_nearest_to_bgr_common(Mat& src, Mat& dst, bool is_nv12) {
    Y420spToResizeNearestToBgrCommon task((unsigned char*)src.data(),
            src.width(),
            src.height(),
            src.stride(),
            (unsigned char*)dst.data(),
            dst.width(),
            dst.height(),
            dst.stride(),
            is_nv12);

    parallel_run(Range(0, dst.height()), task, dst.height() / 2);

    return 0;
}

G_FCV_NAMESPACE1_END()
