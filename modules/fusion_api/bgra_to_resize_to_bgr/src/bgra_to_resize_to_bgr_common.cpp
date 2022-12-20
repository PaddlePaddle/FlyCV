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
#include "modules/core/parallel/interface/parallel.h"
#include <stdlib.h>

#include "modules/img_transform/resize/include/resize_common.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class BgraToResizeBilinearToBgrCommonParallelTask : public ParallelTask {
public:
    BgraToResizeBilinearToBgrCommonParallelTask(
            unsigned short *rows,
            unsigned char* src_ptr,
            unsigned char* dst_ptr,
            unsigned short* alpha,
            unsigned short* beta,
            int* xofs,
            int* yofs,
            int dst_bgra_stride,
            int src_stride,
            int dst_stride,
            int dst_w) : 
            _rows(rows),
            _src_ptr(src_ptr),
            _dst_ptr(dst_ptr),
            _alpha(alpha),
            _beta(beta),
            _xofs(xofs),
            _yofs(yofs),
            _dst_bgra_stride(dst_bgra_stride),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _dst_w(dst_w) {}

    void operator()(const Range& range) const override {
        for (int dy = range.start(); dy < range.end(); dy++) {
            unsigned short *rows0 = _rows;
            unsigned short *rows1 = _rows + _dst_bgra_stride; // the stride of rows should calculate according to bgra
            int sy0 = *(_yofs + dy);
            unsigned short *alphap = _alpha;
            unsigned char *ptr_dst = _dst_ptr + _dst_stride * dy;

            hresize_bn_one_row(_src_ptr, _xofs, sy0, _src_stride,
                    _dst_w, 4, alphap, rows0, rows1);

            unsigned int b0 = _beta[dy * 2];
            unsigned int b1 = _beta[dy * 2 + 1];
            for (int dx = 0; dx < _dst_w; dx++) {
                ptr_dst[0] = fcv_cast_u8((rows0[0] * b0 + rows1[0] * b1 + (1 << 17)) >> 18);
                ptr_dst[1] = fcv_cast_u8((rows0[1] * b0 + rows1[1] * b1 + (1 << 17)) >> 18);
                ptr_dst[2] = fcv_cast_u8((rows0[2] * b0 + rows1[2] * b1 + (1 << 17)) >> 18);

                ptr_dst += 3;
                rows0 += 4;
                rows1 += 4;
            }
        }
    }

private:
    unsigned short *_rows;
    unsigned char* _src_ptr;
    unsigned char* _dst_ptr;
    unsigned short* _alpha;
    unsigned short* _beta;
    int* _xofs;
    int* _yofs;
    int _dst_bgra_stride;
    int _src_stride;
    int _dst_stride;
    int _dst_w;
};

int bgra_to_resize_bilinear_to_bgr_common(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int dst_bgra_stride = (dst_stride / 3) << 2;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = dst_bgra_stride << 2; // dst_w * 2 * 4 * sizeof(short);
    int* buf = (int *)malloc(buf_size);
    unsigned short *rows = (unsigned short*)malloc(rows_size);

    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);

    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    BgraToResizeBilinearToBgrCommonParallelTask task(
            rows,
            src_ptr,
            dst_ptr,
            alpha,
            beta,
            xofs,
            yofs,
            dst_bgra_stride,
            src_stride,
            dst_stride,
            dst_w);

    parallel_run(Range(0, dst_h), task);

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }

    return 0;
}

class BgraToResizeNearestToBgrCommonParallelTask : public ParallelTask {
public:
    BgraToResizeNearestToBgrCommonParallelTask(
            unsigned char* src_ptr,
            unsigned char* dst_ptr,
            int* xofs,
            int* yofs,
            int src_stride,
            int dst_stride,
            int dst_w) : 
            _src_ptr(src_ptr),
            _dst_ptr(dst_ptr),
            _xofs(xofs),
            _yofs(yofs),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _dst_w(dst_w) {}

    void operator()(const Range& range) const override {
        for (int dy = range.start(); dy < range.end(); dy++) {
            const unsigned char* src0 = _src_ptr + (_yofs[dy] >> 16) * _src_stride;
            unsigned char * ptr_dst0 = _dst_ptr + _dst_stride * dy;

            for (int dx = 0; dx < _dst_w; dx++) {
                int idx0 = (_xofs[dx] >> 16) << 2; // (<< 16) << 2;

                ptr_dst0[dx * 3] = src0[idx0];
                ptr_dst0[dx * 3 + 1] = src0[idx0 + 1];
                ptr_dst0[dx * 3 + 2] = src0[idx0 + 2];
            }
        }
    }

private:
    unsigned char* _src_ptr;
    unsigned char* _dst_ptr;
    int* _xofs;
    int* _yofs;
    int _src_stride;
    int _dst_stride;
    int _dst_w;
};

int bgra_to_resize_nearest_to_bgr_common(
        Mat& src,
        Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    int buf_size = (dst_w + dst_h) << 2; //(dst_w + dst_h) * sizeof(int)
    int* buf = (int *)malloc(buf_size);

    int* xofs = buf;
    int* yofs = buf + dst_w;

    nearest_cal_offset(xofs, dst_w, src_w);
    nearest_cal_offset(yofs, dst_h, src_h);

    BgraToResizeNearestToBgrCommonParallelTask task(
            src_ptr,
            dst_ptr,
            xofs,
            yofs,
            src_stride,
            dst_stride,
            dst_w);

    parallel_run(Range(0, dst_h), task);

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()