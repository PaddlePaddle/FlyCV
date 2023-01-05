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

#include "modules/img_transform/resize/include/resize_arm.h"
#include "modules/img_transform/resize/include/resize_common.h"
#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/utils.h"

#include <math.h>
#include <stdlib.h>
#include <cmath>

#include <arm_neon.h>

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define FCV_ALIGNED(x) __attribute__ ((aligned (x)))

//static inline uint8x8x4_t v_cubic_horizonal_elems(
//        const unsigned char* tab,
//        const int* idx) {
//    unsigned char FCV_ALIGNED(32) elems[32] = {
//        tab[idx[0]],
//        tab[idx[0] + 1],
//        tab[idx[0] + 2],
//        tab[idx[0] + 3],
//        tab[idx[1]],
//        tab[idx[1] + 1],
//        tab[idx[1] + 2],
//        tab[idx[1] + 3],
//        tab[idx[2]],
//        tab[idx[2] + 1],
//        tab[idx[2] + 2],
//        tab[idx[2] + 3],
//        tab[idx[3]],
//        tab[idx[3] + 1],
//        tab[idx[3] + 2],
//        tab[idx[3] + 3],
//        tab[idx[4]],
//        tab[idx[4] + 1],
//        tab[idx[4] + 2],
//        tab[idx[4] + 3],
//        tab[idx[5]],
//        tab[idx[5] + 1],
//        tab[idx[5] + 2],
//        tab[idx[5] + 3],
//        tab[idx[6]],
//        tab[idx[6] + 1],
//        tab[idx[6] + 2],
//        tab[idx[6] + 3],
//        tab[idx[7]],
//        tab[idx[7] + 1],
//        tab[idx[7] + 2],
//        tab[idx[7] + 3]
//    };
//
//    return uint8x8x4_t(vld4_u8(elems));
//}

static void vertical_resize_bilinear_u16(
        const unsigned short *row0,
        const unsigned short *row1,
        int src_width_align8,
        int src_w,
        unsigned short b0,
        unsigned short b1,
        unsigned char *dst) {
    int dx = 0;
    for (; dx < src_width_align8; dx += 8) {
        uint16x8_t v_s00_u16 = vld1q_u16(row0);
        uint16x8_t v_s01_u16 = vld1q_u16(row1);

        uint32x4_t rows_u32_lo = vmull_n_u16(vget_low_u16(v_s00_u16), b0);
        uint32x4_t rows_u32_hi = vmull_n_u16(vget_high_u16(v_s00_u16), b0);

        rows_u32_lo = vmlal_n_u16(rows_u32_lo, vget_low_u16(v_s01_u16), b1);
        rows_u32_hi = vmlal_n_u16(rows_u32_hi, vget_high_u16(v_s01_u16), b1);

        uint16x4_t rows_u16_lo = vrshrn_n_u32(rows_u32_lo, 16);
        uint16x4_t rows_u16_hi = vrshrn_n_u32(rows_u32_hi, 16);

        vst1_u8(dst, vrshrn_n_u16(vcombine_u16(rows_u16_lo, rows_u16_hi), 2));

        dst  += 8;
        row0 += 8;
        row1 += 8;
    }

    for (; dx < src_w; dx++) {
        *(dst++) = (*(row0++) * b0 + *(row1++) * b1 + (1 << 17)) >> 18;
    }
}

static void vertical_resize_bilinear_u16(
        const uint8_t *row0,
        const uint8_t *row1,
        int src_width_align8,
        int src_w,
        uint8_t b0,
        uint8_t b1,
        unsigned char *dst) {
    int dx = 0;
    uint8x8_t vb0 = vdup_n_u8(b0);
    uint8x8_t vb1 = vdup_n_u8(b1);
    for (; dx < src_width_align8; dx += 16) {
        uint8x16_t v_s00_u8 = vld1q_u8(row0);
        uint8x16_t v_s10_u8 = vld1q_u8(row1);

        uint16x8_t rows0_u16 = vmull_u8(vget_low_u8(v_s00_u8), vb0);
        uint16x8_t rows1_u16 = vmull_u8(vget_high_u8(v_s00_u8), vb0);

        rows0_u16 = vmlal_u8(rows0_u16, vget_low_u8(v_s10_u8), vb1);
        rows1_u16 = vmlal_u8(rows1_u16, vget_high_u8(v_s10_u8), vb1);

        vst1q_u8(dst, vcombine_u8(vshrn_n_u16(rows0_u16, 7), vshrn_n_u16(rows1_u16, 7)));

        dst  += 16;
        row0 += 16;
        row1 += 16;
    }

    for (; dx < src_w; dx++) {
        *(dst++) = (*(row0++) * b0 + *(row1++) * b1) >> 7;
    }
}

void resize_bilinear_c1_neon_impl(
        unsigned char *src_ptr,
        unsigned char *dst_ptr,
        const int src_w,
        const int src_h,
        const int dst_w,
        const int dst_h,
        const int s_stride,
        const int d_stride) {
    const int dst_width_align8 = dst_w & (~15);

    int* buf = nullptr;
    uint8_t *rows = nullptr;
    int buf_size  = (dst_w + dst_h) << 3;
    int rows_size = (d_stride << 3);
    buf = (int *)malloc(buf_size);
    rows = (uint8_t*)malloc(rows_size);
    get_resize_bilinear_buf_c1(src_w, src_h, dst_w, dst_h, 1, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    uint8_t* alpha = (uint8_t*)(yofs + dst_h);
    uint8_t* beta = (uint8_t*)(alpha + dst_w + dst_w);

    uint8_t *rows0 = nullptr;
    uint8_t *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + dst_w;

    int prev_sy = -1;
    for (int dy = 0; dy < dst_h; dy++) {
        int sy0 = yofs[dy];
        const int sy_off = sy0 * s_stride;

        uint8x8x2_t v_row0_u8, v_row1_u8, v_row2_u8, v_row3_u8;
        uint16x8_t v_y0l_u16, v_y1l_u16, v_y2l_u16, v_y3l_u16;

        uint8_t b0 = beta[dy * 2];
        uint8_t b1 = beta[dy * 2 + 1];

        unsigned char *ptr_dst = dst_ptr + d_stride * dy;

        if (sy0 == prev_sy) {
            uint8_t *alphap = alpha;
            uint8_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;

            const unsigned char *src1 = src_ptr + (sy0 + 1) * s_stride;

            uint8_t* rows1p  = rows1;

            for (int dx = 0; dx < dst_width_align8; dx += 16) {
                const int* cx = &xofs[dx];
                uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                // int32x4_t idx0 = vldq_s32(cx);
                // int32x4_t idx1 = vldq_s32(cx + 4);

                //prefetch_l1(src1, 128);
                //prefetch_l1(S01, 256);
                // v_row1_u8 = v_bilinear_horizonal_elems(src1, cx);
                // v_row3_u8 = v_bilinear_horizonal_elems(src1, cx + 8);

                // v_row1_u8 = vld2_u8(src1);
                // v_row3_u8 = vld2_u8(src1 + 8);

                v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                rows1p += 16;
                alphap += 32;
                //src1 += 16;
            }

            for (int dl = dst_width_align8; dl < dst_w; dl++) {
                int sx   = xofs[dl];
                uint8_t a0 = *(alphap++);
                uint8_t a1 = *(alphap++);

                *(rows1p++) = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);
            }
        } else if (sy0 > prev_sy) {
            uint8_t *alphap = alpha;
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            uint8_t *rows0p = rows0;
            uint8_t *rows1p = rows1;

            for (int dx = 0; dx < dst_width_align8; dx += 16) {
                const int* cx = &xofs[dx];
                uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                // v_row0_u8 = v_bilinear_horizonal_elems(src0, cx);
                // v_row1_u8 = v_bilinear_horizonal_elems(src1, cx);
                // v_row2_u8 = v_bilinear_horizonal_elems(src0, cx + 8);
               // v_row3_u8 = v_bilinear_horizonal_elems(src1, cx + 8);

                v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                v_row0_u8 = vld2_lane_u8(src0 + cx[0], v_row0_u8, 0);
                v_row0_u8 = vld2_lane_u8(src0 + cx[1], v_row0_u8, 1);
                v_row0_u8 = vld2_lane_u8(src0 + cx[2], v_row0_u8, 2);
                v_row0_u8 = vld2_lane_u8(src0 + cx[3], v_row0_u8, 3);
                v_row0_u8 = vld2_lane_u8(src0 + cx[4], v_row0_u8, 4);
                v_row0_u8 = vld2_lane_u8(src0 + cx[5], v_row0_u8, 5);
                v_row0_u8 = vld2_lane_u8(src0 + cx[6], v_row0_u8, 6);
                v_row0_u8 = vld2_lane_u8(src0 + cx[7], v_row0_u8, 7);

                v_row2_u8 = vld2_lane_u8(src0 + cx[8],  v_row2_u8, 0);
                v_row2_u8 = vld2_lane_u8(src0 + cx[9],  v_row2_u8, 1);
                v_row2_u8 = vld2_lane_u8(src0 + cx[10], v_row2_u8, 2);
                v_row2_u8 = vld2_lane_u8(src0 + cx[11], v_row2_u8, 3);
                v_row2_u8 = vld2_lane_u8(src0 + cx[12], v_row2_u8, 4);
                v_row2_u8 = vld2_lane_u8(src0 + cx[13], v_row2_u8, 5);
                v_row2_u8 = vld2_lane_u8(src0 + cx[14], v_row2_u8, 6);
                v_row2_u8 = vld2_lane_u8(src0 + cx[15], v_row2_u8, 7);

                v_y0l_u16 = vmull_u8(v_row0_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));

                v_y0l_u16 = vmlal_u8(v_y0l_u16, v_row0_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));

                v_y2l_u16 = vmull_u8(v_row2_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));
                v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                v_y2l_u16 = vmlal_u8(v_y2l_u16, v_row2_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));
                v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                vst1q_u8(rows0p, vcombine_u8(vshrn_n_u16(v_y0l_u16, 7), vshrn_n_u16(v_y2l_u16, 7)));
                vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                rows0p += 16;
                rows1p += 16;
                alphap += 32;
            }

            for (int dl = dst_width_align8; dl < dst_w; dl++) {
                int sx = xofs[dl];
                uint8_t a0 = *(alphap++);
                uint8_t a1 = *(alphap++);

                uint8_t rows0_res = (uint8_t)((src0[sx] * a0 + src0[sx + 1] * a1) >> 7);
                uint8_t rows1_res = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);

                *(rows0p++) = rows0_res;
                *(rows1p++) = rows1_res;
            }
        }

        prev_sy = sy0 + 1;
        vertical_resize_bilinear_u16(rows0,
                 rows1, dst_width_align8, dst_w, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_bilinear_c1_comm_neon_old(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    resize_bilinear_c1_neon_impl(src_ptr, dst_ptr, src_w,
            src_h, dst_w, dst_h, s_stride, d_stride);
}

class ResizeBilinearC1NeonParallelTask : public ParallelTask {
public:
    ResizeBilinearC1NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align16 = _dst_stride & (~15);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint8_t* alpha = (const uint8_t*)(yofs + _dst_h);
        const uint8_t* beta  = (const uint8_t*)(alpha + _dst_w + _dst_w);

        uint8_t* rows = (uint8_t*)malloc(_dst_stride << 3);
        uint8_t* rows0 = rows;
        uint8_t* rows1 = rows + _dst_stride;

        int prev_sy1 = (range.start() == 0) ? -1 : (yofs[range.start() - 1] + 1);
        bool valid_rows1 = false;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint8_t* cur_beta = beta + (range.start() << 1);
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint8_t *alphap = alpha;

            uint8x8x2_t v_row0_u8, v_row1_u8, v_row2_u8, v_row3_u8;
            uint16x8_t v_y0l_u16, v_y1l_u16, v_y2l_u16, v_y3l_u16;

            if (sy0 == prev_sy1 && valid_rows1) {
                uint8_t* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;

                uint8_t* rows1p  = rows1;
                int dx = 0;
                for (; dx < dst_width_align16; dx += 16) {
                    const int* cx = &xofs[dx];
                    uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                    v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                    v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                    v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                    v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                    v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                    v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                    vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                    rows1p += 16;
                    alphap += 32;
                }
                for (; dx < _dst_w; dx++) {
                    int sx   = xofs[dx];
                    uint8_t a0 = *(alphap++);
                    uint8_t a1 = *(alphap++);

                    *(rows1p++) = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);
                }
            } else {
                // hresize two rows
                const uint8_t* src0 = _src_ptr + sy_off;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;

                uint8_t *rows0p = rows0;
                uint8_t *rows1p = rows1;
                int dx = 0;
                for (; dx < dst_width_align16; dx += 16) {
                    const int* cx = &xofs[dx];
                    uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                    v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                    v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                    v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                    v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                    v_row0_u8 = vld2_lane_u8(src0 + cx[0], v_row0_u8, 0);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[1], v_row0_u8, 1);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[2], v_row0_u8, 2);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[3], v_row0_u8, 3);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[4], v_row0_u8, 4);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[5], v_row0_u8, 5);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[6], v_row0_u8, 6);
                    v_row0_u8 = vld2_lane_u8(src0 + cx[7], v_row0_u8, 7);

                    v_row2_u8 = vld2_lane_u8(src0 + cx[8],  v_row2_u8, 0);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[9],  v_row2_u8, 1);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[10], v_row2_u8, 2);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[11], v_row2_u8, 3);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[12], v_row2_u8, 4);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[13], v_row2_u8, 5);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[14], v_row2_u8, 6);
                    v_row2_u8 = vld2_lane_u8(src0 + cx[15], v_row2_u8, 7);

                    v_y0l_u16 = vmull_u8(v_row0_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                    v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));

                    v_y0l_u16 = vmlal_u8(v_y0l_u16, v_row0_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                    v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));

                    v_y2l_u16 = vmull_u8(v_row2_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));
                    v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                    v_y2l_u16 = vmlal_u8(v_y2l_u16, v_row2_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));
                    v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                    vst1q_u8(rows0p, vcombine_u8(vshrn_n_u16(v_y0l_u16, 7), vshrn_n_u16(v_y2l_u16, 7)));
                    vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                    rows0p += 16;
                    rows1p += 16;
                    alphap += 32;
                }

                for (; dx < _dst_w; dx++) {
                    int sx = xofs[dx];
                    uint8_t a0 = *(alphap++);
                    uint8_t a1 = *(alphap++);

                    uint8_t rows0_res = (uint8_t)((src0[sx] * a0 + src0[sx + 1] * a1) >> 7);
                    uint8_t rows1_res = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);

                    *(rows0p++) = rows0_res;
                    *(rows1p++) = rows1_res;
                }
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            uint16_t b0 = *cur_beta++;
            uint16_t b1 = *cur_beta++;

            vertical_resize_bilinear_u16(rows0,
                 rows1, dst_width_align16, _dst_stride, b0, b1, ptr_dst);

            ptr_dst += _dst_stride;
        }
        free(rows);
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c1_comm_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf_c1(src_w, src_h, dst_w, dst_h, 1, &buf);

    ResizeBilinearC1NeonParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

class Resize_Bilinear_C3_Neon_ParallelTask : public ParallelTask {
public:
    Resize_Bilinear_C3_Neon_ParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint16_t* alpha = (const uint16_t*)(yofs + _dst_h);
        const uint16_t* beta  = (const uint16_t*)(alpha + _dst_w + _dst_w);

        uint16_t* rows = (uint16_t*)malloc(_dst_stride << 3);
        uint16_t* rows0 = rows;
        uint16_t* rows1 = rows + _dst_stride;

        int prev_sy1 = (range.start() == 0) ? -1 : (yofs[range.start() - 1] + 1);
        bool valid_rows1 = false;
        // const uint8_t* ptr_src = _src_ptr;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint16_t* cur_beta = beta + (range.start() << 1);
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint16_t *alphap = alpha;
            if (sy0 == prev_sy1 && valid_rows1) {
                uint16_t* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                int dx = 0;
                for (; dx < _dst_w - 1; ++dx) {
                    int idx = dx * 3;
                    int cx = xofs[dx];

                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;

                    const uint8_t* S1p = src1 + cx;
                    uint16x4_t v_a0 = vdup_n_u16(a0);
                    uint16x4_t v_a1 = vdup_n_u16(a1);
                    uint8x8_t v_s1 = uint8x8_t();

                    v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                    v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                    v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                    v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                    v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                    v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                    uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                    uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                    uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                    uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                    v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                    uint16x4_t rows1_s16= vshrn_n_u32(v_rows1, 4);

                    vst1_u16(rows1 + idx, rows1_s16);
                }
                int idx = dx + (dx << 1);
                int cx = xofs[dx];

                uint16_t a0 = *alphap++;
                uint16_t a1 = *alphap++;

                const uint8_t* S1p = src1 + cx;

                rows1[idx] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            } else {
                // hresize two rows
                const uint8_t *src0 = _src_ptr + sy_off;
                const uint8_t *src1 = _src_ptr + sy_off + _src_stride;

                uint16_t* rows0p = rows0;
                uint16_t* rows1p = rows1;
                int dx = 0;
                for (; dx < _dst_w - 1; dx++) {
                    int idx = dx + (dx << 1);
                    int cx = xofs[dx];

                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;

                    const uint8_t* S0p = src0 + cx;
                    const uint8_t* S1p = src1 + cx;
                    uint16x4_t v_a0 = vdup_n_u16(a0);
                    uint16x4_t v_a1 = vdup_n_u16(a1);
                    uint8x8_t v_s0 = uint8x8_t();
                    uint8x8_t v_s1 = uint8x8_t();

                    v_s0 = vld1_lane_u8(S0p, v_s0, 0);
                    v_s0 = vld1_lane_u8(S0p + 1, v_s0, 1);
                    v_s0 = vld1_lane_u8(S0p + 2, v_s0, 2);
                    v_s0 = vld1_lane_u8(S0p + 3, v_s0, 3);
                    v_s0 = vld1_lane_u8(S0p + 4, v_s0, 4);
                    v_s0 = vld1_lane_u8(S0p + 5, v_s0, 5);

                    v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                    v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                    v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                    v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                    v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                    v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                    uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                    uint16x4_t v_s0_low = vget_low_u16(v_s0_u16);
                    uint16x4_t v_s0_hi  = vext_u16(v_s0_low, vget_high_u16(v_s0_u16), 3);
                    uint32x4_t v_rows0 = vmull_u16(v_s0_low, v_a0);
                    v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                    uint16x4_t rows0_u16= vshrn_n_u32(v_rows0, 4);

                    uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                    uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                    uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                    uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                    v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                    uint16x4_t rows1_u16= vshrn_n_u32(v_rows1, 4);

                    vst1_u16(rows0p + idx, rows0_u16);
                    vst1_u16(rows1p + idx, rows1_u16);
                }
                int idx = dx + (dx << 1);
                int cx = xofs[dx];

                uint16_t a0 = *alphap++;
                uint16_t a1 = *alphap++;

                const uint8_t* S0p = src0 + cx;
                const uint8_t* S1p = src1 + cx;

                rows0p[idx] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
                rows0p[idx + 1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
                rows0p[idx + 2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;

                rows1p[idx] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1p[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1p[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            uint16_t b0 = *cur_beta++;
            uint16_t b1 = *cur_beta++;

            vertical_resize_bilinear_u16(rows0, rows1,
                    dst_width_align8, _dst_stride, b0, b1, ptr_dst);

            ptr_dst += _dst_stride;
        }

        free(rows);
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c3_comm_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 3, &buf);

    Resize_Bilinear_C3_Neon_ParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

void resize_bilinear_c3_comm_neon_old(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = d_stride & (~7);
    int dst_w_1 = dst_w - 1;

    int* buf = nullptr;
    unsigned short *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = d_stride << 3;
    buf = (int *)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 3, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + d_stride;

    unsigned char *ptr_dst = nullptr;
    int dy = 0, dx= 0, sy0 = 0;
    unsigned short b0 = 0, b1 = 0;
    int prev_sy1 = -1;

    for (dy = 0; dy < dst_h; dy++) {
        ptr_dst = dst_ptr + d_stride * dy;

        sy0 = *(yofs + dy);
        const int sy_off = sy0 * s_stride;
        unsigned short *alphap = alpha;

        if (sy0 == prev_sy1) {
            unsigned short *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            for (dx = 0 ; dx < dst_w_1; dx++) {
                int idx = dx * 3;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);
                uint8x8_t v_s1 = uint8x8_t();

                v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t rows1_s16= vshrn_n_u32(v_rows1, 4);

                vst1_u16(rows1 + idx, rows1_s16);
            }

            {
                int idx = dst_w_1 + (dst_w_1 << 1);
                int cx = xofs[dst_w_1];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S1p = src1 + cx;

                rows1[idx] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            }
        } else if (sy0 > prev_sy1) {
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            unsigned short *rows0p = rows0;
            unsigned short *rows1p = rows1;

            for (dx = 0; dx < dst_w_1; dx++) {
                int idx = dx + (dx << 1);
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);
                uint8x8_t v_s0 = uint8x8_t();
                uint8x8_t v_s1 = uint8x8_t();

                v_s0 = vld1_lane_u8(S0p, v_s0, 0);
                v_s0 = vld1_lane_u8(S0p + 1, v_s0, 1);
                v_s0 = vld1_lane_u8(S0p + 2, v_s0, 2);
                v_s0 = vld1_lane_u8(S0p + 3, v_s0, 3);
                v_s0 = vld1_lane_u8(S0p + 4, v_s0, 4);
                v_s0 = vld1_lane_u8(S0p + 5, v_s0, 5);

                v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                uint16x4_t v_s0_low = vget_low_u16(v_s0_u16);
                uint16x4_t v_s0_hi  = vext_u16(v_s0_low, vget_high_u16(v_s0_u16), 3);
                uint32x4_t v_rows0 = vmull_u16(v_s0_low, v_a0);
                v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                uint16x4_t rows0_u16= vshrn_n_u32(v_rows0, 4);

                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t rows1_u16= vshrn_n_u32(v_rows1, 4);

                vst1_u16(rows0p + idx, rows0_u16);
                vst1_u16(rows1p + idx, rows1_u16);
            }

            {
                int idx = dst_w_1 + (dst_w_1 << 1);
                int cx = xofs[dst_w_1];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;

                rows0p[idx] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
                rows0p[idx + 1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
                rows0p[idx + 2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;

                rows1p[idx] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1p[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1p[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            }
        }

        prev_sy1 = sy0 + 1;
        b0 = *(beta++);
        b1 = *(beta++);

        vertical_resize_bilinear_u16(rows0, rows1,
                dst_width_align8, d_stride, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_bilinear_c4_comm_neon_old(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = d_stride & (~7);

    int* buf = nullptr;
    unsigned short *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = d_stride << 3;
    buf = (int*)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + d_stride;

    unsigned char *ptr_dst = nullptr;
    int dy = 0, dx= 0, idx = 0, sy0 = 0, cx = 0;
    unsigned short b0 = 0, b1 = 0;

    int prev_sy1 = -1;
    for (; dy < dst_h; dy++) {
        ptr_dst = dst_ptr + d_stride * dy;
        sy0 = *(yofs + dy);
        const int sy_off = sy0 * s_stride;

        if (sy0 == prev_sy1) {
            unsigned short *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);
            unsigned short *alphap = alpha;

            for (dx = 0; dx < dst_w; dx++) {
                idx = dx << 2;
                cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s1 = vld1_u8(S1p);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows1_res = vrshrn_n_u32(v_rows1, 4);
                vst1_u16(rows1 + idx, v_rows1_res);
            }
        } else if (sy0 > prev_sy1) {
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            unsigned short *rows0p = rows0;
            unsigned short *rows1p = rows1;

            unsigned short *alphap = alpha;
            for (dx = 0; dx < dst_w; dx++) {
                int idx = dx << 2;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s0 = vld1_u8(S0p);
                uint8x8_t v_s1 = vld1_u8(S1p);
                uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s0_lo = vget_low_u16(v_s0_u16);
                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);

                uint16x4_t v_s0_hi = vget_high_u16(v_s0_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows0 = vmull_u16(v_s0_lo, v_a0);
                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);

                v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows0_res = vshrn_n_u32(v_rows0, 4);
                uint16x4_t v_rows1_res = vshrn_n_u32(v_rows1, 4);
                vst1_u16(rows0p + idx, v_rows0_res);
                vst1_u16(rows1p + idx, v_rows1_res);
            }
        }

        prev_sy1 = sy0 + 1;
        b0 = *beta++;
        b1 = *beta++;

        vertical_resize_bilinear_u16(rows0, rows1, dst_width_align8,
                d_stride, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

class ResizeBilinearC4NeonParallelTask : public ParallelTask {
public:
    ResizeBilinearC4NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_h, 
            int dst_stride,
            const int* buf)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_h(dst_h), 
            _dst_stride(dst_stride),
            _buf(buf) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_stride & (~7);
        const int* xofs = _buf;
        const int* yofs = _buf + _dst_w;
        const uint16_t* alpha = (const uint16_t*)(yofs + _dst_h);
        const uint16_t* beta  = (const uint16_t*)(alpha + _dst_w + _dst_w);

        uint16_t* rows = (uint16_t*)malloc(_dst_stride << 3);
        uint16_t* rows0 = rows;
        uint16_t* rows1 = rows + _dst_stride;

        int prev_sy1 = (range.start() == 0) ? -1 : (yofs[range.start() - 1] + 1);
        bool valid_rows1 = false;
        // const uint8_t* ptr_src = _src_ptr;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        const uint16_t* cur_beta = beta + (range.start() << 1);
        for (int i = range.start(); i < range.end(); i++) {
            int sy0 = yofs[i];
            const int sy_off = sy0 * _src_stride;
            const uint16_t *alphap = alpha;
            if (sy0 == prev_sy1 && valid_rows1) {
                uint16_t* rows0_old = rows0;
                rows0 = rows1;
                rows1 = rows0_old;
                const uint8_t* src1 = _src_ptr + sy_off + _src_stride;
                for (int dx = 0; dx < _dst_w; dx++) {
                    int idx = dx << 2;
                    int cx = xofs[dx];

                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;

                    const uint8_t* S1p = src1 + cx;
                    uint16x4_t v_a0 = vdup_n_u16(a0);
                    uint16x4_t v_a1 = vdup_n_u16(a1);

                    uint8x8_t v_s1 = vld1_u8(S1p);
                    uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                    uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);
                    uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                    uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);
                    v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                    uint16x4_t v_rows1_res = vrshrn_n_u32(v_rows1, 4);
                    vst1_u16(rows1 + idx, v_rows1_res);
                }
            } else {
                // hresize two rows
                const uint8_t *src0 = _src_ptr + sy_off;
                const uint8_t *src1 = _src_ptr + sy_off + _src_stride;

                uint16_t* rows0p = rows0;
                uint16_t* rows1p = rows1;
                for (int dx = 0; dx < _dst_w; dx++) {
                    int idx = dx << 2;
                    int cx = xofs[dx];

                    uint16_t a0 = *alphap++;
                    uint16_t a1 = *alphap++;

                    const uint8_t* S0p = src0 + cx;
                    const uint8_t* S1p = src1 + cx;
                    uint16x4_t v_a0 = vdup_n_u16(a0);
                    uint16x4_t v_a1 = vdup_n_u16(a1);

                    uint8x8_t v_s0 = vld1_u8(S0p);
                    uint8x8_t v_s1 = vld1_u8(S1p);
                    uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                    uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                    uint16x4_t v_s0_lo = vget_low_u16(v_s0_u16);
                    uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);

                    uint16x4_t v_s0_hi = vget_high_u16(v_s0_u16);
                    uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                    uint32x4_t v_rows0 = vmull_u16(v_s0_lo, v_a0);
                    uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);

                    v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                    v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                    uint16x4_t v_rows0_res = vshrn_n_u32(v_rows0, 4);
                    uint16x4_t v_rows1_res = vshrn_n_u32(v_rows1, 4);
                    vst1_u16(rows0p + idx, v_rows0_res);
                    vst1_u16(rows1p + idx, v_rows1_res);
                }
            }
            valid_rows1 = true;
            prev_sy1 = sy0 + 1;
            uint16_t b0 = *cur_beta++;
            uint16_t b1 = *cur_beta++;

            vertical_resize_bilinear_u16(rows0, rows1,
                    dst_width_align8, _dst_stride, b0, b1, ptr_dst);

            ptr_dst += _dst_stride;
        }
        free(rows);
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_h;
    int _dst_stride;
    const int* _buf;
};

static void resize_bilinear_c4_comm_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_w = src.width();
    int src_h = src.height();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    int* buf = (int*)malloc((dst_w + dst_h) << 3);
    get_resize_bilinear_buf(src_w, src_h, dst_w, dst_h, 4, &buf);

    ResizeBilinearC4NeonParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_h, dst_stride, buf);
    parallel_run(Range(0, dst_h), task);
    free(buf);
}

void resize_bilinear_c1_dn2x_neon_old(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int dst_width_align8 = dst_w & (~15);
    const int dou_srcw = src_w << 1;

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    int dy = 0, dx = 0;
    for (; dy < dst_h; dy++) {
        const unsigned char *S00 = (unsigned char*)ptr_src;
        const unsigned char *S01 = (unsigned char*)(ptr_src + src_w);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dst_width_align8; dx += 16) {
            prefetch_l1(S00, 256);
            prefetch_l1(S01, 256);
            uint8x16_t src0_u8 = vld1q_u8(S00);
            uint8x16_t src1_u8 = vld1q_u8(S01);

            uint16x8_t src0_u16 = vpaddlq_u8(src0_u8);
            uint16x8_t vsum0_u16 = vpadalq_u8(src0_u16, src1_u8);

            uint8x16_t src2_u8 = vld1q_u8(S00 + 16);
            uint8x16_t src3_u8 = vld1q_u8(S01 + 16);

            uint16x8_t src1_u16 = vpaddlq_u8(src2_u8);
            uint16x8_t vsum1_u16 = vpadalq_u8(src1_u16, src3_u8);

            uint8x8_t res0_u8 = vrshrn_n_u16(vsum0_u16, 2);
            uint8x8_t res1_u8 = vrshrn_n_u16(vsum1_u16, 2);

            vst1q_u8(dst0, vcombine_u8(res0_u8, res1_u8));

            S00 += 32;
            S01 += 32;
            dst0 += 16;
        }

        for (; dx < dst_w; dx++) {
            *(dst0++) = (unsigned char)((S00[0] + S00[1] + S01[0] + S01[1] + 2) >> 2);
            S00 += 2;
            S01 += 2;
        }

        ptr_src += dou_srcw;
        ptr_dst += dst_w;
    }
}

class HalfResizeBilinearC1NeonParallelTask : public ParallelTask {
public:
    HalfResizeBilinearC1NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;

        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;

        const int dst_width_align16 = _dst_w & (~15);
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_width_align16; dx += 16) {
                prefetch_l1(S00, 256);
                prefetch_l1(S01, 256);
                uint8x16_t src0_u8 = vld1q_u8(S00);
                uint8x16_t src1_u8 = vld1q_u8(S01);

                uint16x8_t src0_u16 = vpaddlq_u8(src0_u8);
                uint16x8_t vsum0_u16 = vpadalq_u8(src0_u16, src1_u8);

                uint8x16_t src2_u8 = vld1q_u8(S00 + 16);
                uint8x16_t src3_u8 = vld1q_u8(S01 + 16);

                uint16x8_t src1_u16 = vpaddlq_u8(src2_u8);
                uint16x8_t vsum1_u16 = vpadalq_u8(src1_u16, src3_u8);

                uint8x8_t res0_u8 = vrshrn_n_u16(vsum0_u16, 2);
                uint8x8_t res1_u8 = vrshrn_n_u16(vsum1_u16, 2);

                vst1q_u8(dst0, vcombine_u8(res0_u8, res1_u8));

                S00 += 32;
                S01 += 32;
                dst0 += 16;
            }

            for (; dx < _dst_w; dx++) {
                *(dst0++) = uint8_t((S00[0] + S00[1] + S01[0] + S01[1] + 2) >> 2);
                S00 += 2;
                S01 += 2;
            }
            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c1_dn2x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    HalfResizeBilinearC1NeonParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c1_dn4x_neon_old(Mat& src, Mat& dst) {
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int dst_width_align8 = dst_w & (~7);
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const int doub_src_stride = src_stride << 1;
    const int four_src_stride = doub_src_stride << 1;

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    int dy = 0, dx = 0;
    for (; dy < dst_h; dy++) {
        const unsigned char *S01 = (unsigned char*)(ptr_src + src_stride);
        const unsigned char *S02 = (unsigned char*)(ptr_src + doub_src_stride);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dst_width_align8; dx += 8) {
            uint8x8x4_t src1_u8 = vld4_u8(S01);
            uint8x8x4_t src2_u8 = vld4_u8(S02);

            uint16x8_t h1sum_u16 = vaddl_u8(src1_u8.val[1], src1_u8.val[2]);
            uint16x8_t h2sum_u16 = vaddl_u8(src2_u8.val[1], src2_u8.val[2]);

            uint16x8_t vsum_u16 = vaddq_u16(h1sum_u16, h2sum_u16);
            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_u8(dst0, res_u8);

            S01 += 32;
            S02 += 32;
            dst0 += 8;
        }

        for (; dx < dst_w; dx++) {
            *(dst0++) = (unsigned char)((S01[1] + S01[2] + S02[1] + S02[2] + 2) >> 2);
            S01 += 4;
            S02 += 4;
        }

        ptr_src += four_src_stride;
        ptr_dst += dst_stride;
    }
}

class QuarterResizeBilinearC1NeonParallelTask : public ParallelTask {
public:
    QuarterResizeBilinearC1NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dst_width_align8 = _dst_w & (~7);

        const int dou_src_step = _src_stride << 1;
        const int fou_src_step = dou_src_step << 1;

        const uint8_t* ptr_src = _src_ptr + range.start() * fou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S01 = ptr_src + _src_stride;
            const uint8_t* S02 = ptr_src + dou_src_step;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_width_align8; dx += 8) {
                uint8x8x4_t src1_u8 = vld4_u8(S01);
                uint8x8x4_t src2_u8 = vld4_u8(S02);

                uint16x8_t h1sum_u16 = vaddl_u8(src1_u8.val[1], src1_u8.val[2]);
                uint16x8_t h2sum_u16 = vaddl_u8(src2_u8.val[1], src2_u8.val[2]);

                uint16x8_t vsum_u16 = vaddq_u16(h1sum_u16, h2sum_u16);
                uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

                vst1_u8(dst0, res_u8);

                S01 += 32;
                S02 += 32;
                dst0 += 8;
            }

            for (; dx < _dst_w; dx++) {
                *(dst0++) = uint8_t((S01[1] + S01[2] + S02[1] + S02[2] + 2) >> 2);
                S01 += 4;
                S02 += 4;
            }
            ptr_src += fou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c1_dn4x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    QuarterResizeBilinearC1NeonParallelTask task(src_ptr,
            src_stride, dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

class Half_Resize_Bilinear_C3_Neon_ParallelTask : public ParallelTask {
public:
    Half_Resize_Bilinear_C3_Neon_ParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;

        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;

        uint8x8_t src0_u8, src1_u8;
        uint16x8_t hsum_u16, ext_sum_u16;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < _dst_w - 2; dx++) {
                src0_u8 = vld1_u8(S00);
                src1_u8 = vld1_u8(S01);
                prefetch_l1(S00, 256);
                prefetch_l1(S01, 256);

                hsum_u16 = vaddl_u8(src0_u8, src1_u8);
                ext_sum_u16 = vextq_u16(hsum_u16, hsum_u16, 3);//b1 g1 r1

                uint16x8_t vsum_u16 = vaddq_u16(hsum_u16, ext_sum_u16);
                uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

                vst1_u8(dst0, res_u8);

                S00 += 6;
                S01 += 6;
                dst0 += 3;
            }

            for (; dx < _dst_w; dx++) {
                dst0[0] = uint8_t((S00[0] + S00[3] + S01[0] + S01[3] + 2) >> 2);
                dst0[1] = uint8_t((S00[1] + S00[4] + S01[1] + S01[4] + 2) >> 2);
                dst0[2] = uint8_t((S00[2] + S00[5] + S01[2] + S01[5] + 2) >> 2);
                S00 += 6;
                S01 += 6;
                dst0 += 3;
            }
            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c3_dn2x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    Half_Resize_Bilinear_C3_Neon_ParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c3_dn2x_neon_old(Mat& src, Mat& dst) {
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_step  = src.stride();
    const int d_step  = dst.stride();
    const int dou_src_step = s_step << 1;

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    uint8x8_t src0_u8, src1_u8;
    uint16x8_t hsum_u16, ext_sum_u16;
    int dy = 0, dx = 0;
    for (; dy < dst_h; dy++) {
        const unsigned char *S00 = (unsigned char*)ptr_src;
        const unsigned char *S01 = (unsigned char*)(ptr_src + s_step);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dst_w - 2; dx++) {
            src0_u8 = vld1_u8(S00);
            src1_u8 = vld1_u8(S01);
            prefetch_l1(S00, 256);
            prefetch_l1(S01, 256);

            hsum_u16 = vaddl_u8(src0_u8, src1_u8);
            ext_sum_u16 = vextq_u16(hsum_u16, hsum_u16, 3);//b1 g1 r1

            uint16x8_t vsum_u16 = vaddq_u16(hsum_u16, ext_sum_u16);
            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_u8(dst0, res_u8);

            S00 += 6;
            S01 += 6;
            dst0 += 3;
        }

        for (; dx < dst_w; dx++) {
            dst0[0] = (unsigned char)((S00[0] + S00[3] + S01[0] + S01[3] + 2) >> 2);
            dst0[1] = (unsigned char)((S00[1] + S00[4] + S01[1] + S01[4] + 2) >> 2);
            dst0[2] = (unsigned char)((S00[2] + S00[5] + S01[2] + S01[5] + 2) >> 2);
            S00 += 6;
            S01 += 6;
            dst0 += 3;
        }

        ptr_src += dou_src_step;
        ptr_dst += d_step;
    }
}

class Quarter_Resize_Bilinear_C3_Neon_ParallelTask : public ParallelTask {
public:
    Quarter_Resize_Bilinear_C3_Neon_ParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;
        const int fou_src_step = dou_src_step << 1;

        const uint8_t* ptr_src = _src_ptr + range.start() * fou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S01 = ptr_src + _src_stride;
            const uint8_t* S02 = ptr_src + dou_src_step;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < _dst_w - 2; dx++) {
                uint8x8_t src0_u8 = vld1_u8(S01 + 3); //b1 g1 r1
                uint8x8_t src1_u8 = vld1_u8(S02 + 3);
                prefetch_l1(S01, 256);
                prefetch_l1(S02, 256);

                uint8x8_t r01_ext_u8 = vext_u8(src0_u8, src0_u8, 3);//b2 g2 r2
                uint8x8_t r11_ext_u8 = vext_u8(src1_u8, src1_u8, 3);//b1 g1 r1

                uint16x8_t vsum0_u16 = vaddl_u8((r01_ext_u8), (src0_u8));
                uint16x8_t vsum1_u16 = vaddl_u8((r11_ext_u8), (src1_u8));
                uint16x8_t vsum_u16  = vaddq_u16(vsum0_u16, vsum1_u16);

                uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

                vst1_u8(dst0, res_u8);
                S01  += 12;
                S02  += 12;
                dst0 += 3;
            }

            for (; dx < _dst_w; dx++) {
                uint8x8_t s0_u8 = uint8x8_t();
                uint8x8_t s1_u8 = uint8x8_t();

                s0_u8 = vld1_lane_u8(S01 + 3, s0_u8, 0); //b1 g1 r1
                s0_u8 = vld1_lane_u8(S01 + 4, s0_u8, 1);
                s0_u8 = vld1_lane_u8(S01 + 5, s0_u8, 2);
                s0_u8 = vld1_lane_u8(S01 + 6, s0_u8, 3);
                s0_u8 = vld1_lane_u8(S01 + 7, s0_u8, 4);
                s0_u8 = vld1_lane_u8(S01 + 8, s0_u8, 5);

                s1_u8 = vld1_lane_u8(S02 + 3, s1_u8, 0); //b1 g1 r1
                s1_u8 = vld1_lane_u8(S02 + 4, s1_u8, 1);
                s1_u8 = vld1_lane_u8(S02 + 5, s1_u8, 2);
                s1_u8 = vld1_lane_u8(S02 + 6, s1_u8, 3);
                s1_u8 = vld1_lane_u8(S02 + 7, s1_u8, 4);
                s1_u8 = vld1_lane_u8(S02 + 8, s1_u8, 5);

                uint8x8_t r02_ext_u8 = vext_u8(s0_u8, s0_u8, 3);//b2 g2 r2
                uint8x8_t r12_ext_u8 = vext_u8(s1_u8, s1_u8, 3);//b2 g2 r2

                uint16x8_t vsum0_u16 = vaddl_u8(s0_u8, r02_ext_u8);
                uint16x8_t vsum1_u16 = vaddl_u8(s1_u8, r12_ext_u8);
                uint16x8_t vsum_u16  = vaddq_u16(vsum0_u16, vsum1_u16);

                uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

                vst1_lane_u8(dst0, res_u8, 0);
                vst1_lane_u8(dst0 + 1, res_u8, 1);
                vst1_lane_u8(dst0 + 2, res_u8, 2);

                S01  += 12;
                S02  += 12;
                dst0 += 3;
            }
            ptr_src += fou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c3_dn4x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    Quarter_Resize_Bilinear_C3_Neon_ParallelTask task(src_ptr,
            src_stride, dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c3_dn4x_neon_old(Mat& src, Mat& dst) {
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int d_step  = dst.stride();
    const int s_step  = src.stride();
    const int dou_src_step = s_step << 1;
    const int fou_src_step = dou_src_step << 1;
    const int dst_w_1 = dst_w - 1;

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    uint8x8_t src0_u8, src1_u8;
    int dy = 0, dx = 0;

    for (; dy < dst_h; dy++) {
        const unsigned char *S01 = (unsigned char*)(ptr_src + s_step);
        const unsigned char *S02 = (unsigned char*)(ptr_src + dou_src_step);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dst_w_1; dx++) {
            src0_u8 = vld1_u8(S01 + 3); //b1 g1 r1
            src1_u8 = vld1_u8(S02 + 3);
            prefetch_l1(S01, 256);
            prefetch_l1(S02, 256);

            uint8x8_t r01_ext_u8 = vext_u8(src0_u8, src0_u8, 3);//b2 g2 r2
            uint8x8_t r11_ext_u8 = vext_u8(src1_u8, src1_u8, 3);//b1 g1 r1

            uint16x8_t vsum0_u16 = vaddl_u8((r01_ext_u8), (src0_u8));
            uint16x8_t vsum1_u16 = vaddl_u8((r11_ext_u8), (src1_u8));
            uint16x8_t vsum_u16  = vaddq_u16(vsum0_u16, vsum1_u16);

            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_u8(dst0, res_u8);

            S01  += 12;
            S02  += 12;
            dst0 += 3;
        }

        for (; dx < dst_w; dx++) {
            uint8x8_t s0_u8 = uint8x8_t();
            uint8x8_t s1_u8 = uint8x8_t();

            s0_u8 = vld1_lane_u8(S01 + 3, s0_u8, 0); //b1 g1 r1
            s0_u8 = vld1_lane_u8(S01 + 4, s0_u8, 1);
            s0_u8 = vld1_lane_u8(S01 + 5, s0_u8, 2);
            s0_u8 = vld1_lane_u8(S01 + 6, s0_u8, 3);
            s0_u8 = vld1_lane_u8(S01 + 7, s0_u8, 4);
            s0_u8 = vld1_lane_u8(S01 + 8, s0_u8, 5);

            s1_u8 = vld1_lane_u8(S02 + 3, s1_u8, 0); //b1 g1 r1
            s1_u8 = vld1_lane_u8(S02 + 4, s1_u8, 1);
            s1_u8 = vld1_lane_u8(S02 + 5, s1_u8, 2);
            s1_u8 = vld1_lane_u8(S02 + 6, s1_u8, 3);
            s1_u8 = vld1_lane_u8(S02 + 7, s1_u8, 4);
            s1_u8 = vld1_lane_u8(S02 + 8, s1_u8, 5);

            uint8x8_t r02_ext_u8 = vext_u8(s0_u8, s0_u8, 3);//b2 g2 r2
            uint8x8_t r12_ext_u8 = vext_u8(s1_u8, s1_u8, 3);//b2 g2 r2

            uint16x8_t vsum0_u16 = vaddl_u8(s0_u8, r02_ext_u8);
            uint16x8_t vsum1_u16 = vaddl_u8(s1_u8, r12_ext_u8);
            uint16x8_t vsum_u16  = vaddq_u16(vsum0_u16, vsum1_u16);

            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_lane_u8(dst0, res_u8, 0);
            vst1_lane_u8(dst0 + 1, res_u8, 1);
            vst1_lane_u8(dst0 + 2, res_u8, 2);
        }

        ptr_src += fou_src_step;
        ptr_dst += d_step;
    }
}

void resize_bilinear_c4_dn2x_neon_old(Mat& src, Mat& dst) {
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int dw_align8 = dst_w & (~7);
    const int s_step  = src.stride();
    const int d_step  = dst.stride();
    const int dou_src_step = s_step << 1;

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    uint8x16x4_t src0_u8, src1_u8;
    uint16x8_t s0, s1, s2, s3;
    int dy = 0, dx = 0;
    for (; dy < dst_h; dy++) {
        const unsigned char *S00 = (unsigned char*)ptr_src;
        const unsigned char *S01 = (unsigned char*)(ptr_src + s_step);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dw_align8; dx += 8) {
            prefetch_l1(S00, 256);
            prefetch_l1(S01, 256);

            src0_u8 = vld4q_u8(S00);
            src1_u8 = vld4q_u8(S01);

            s0 = vpaddlq_u8(src0_u8.val[0]);
            s1 = vpaddlq_u8(src0_u8.val[1]);
            s2 = vpaddlq_u8(src0_u8.val[2]);
            s3 = vpaddlq_u8(src0_u8.val[3]);

            s0 = vpadalq_u8(s0, src1_u8.val[0]);
            s1 = vpadalq_u8(s1, src1_u8.val[1]);
            s2 = vpadalq_u8(s2, src1_u8.val[2]);
            s3 = vpadalq_u8(s3, src1_u8.val[3]);

            uint8x8x4_t res_u8;
            res_u8.val[0] = vrshrn_n_u16(s0, 2);
            res_u8.val[1] = vrshrn_n_u16(s1, 2);
            res_u8.val[2] = vrshrn_n_u16(s2, 2);
            res_u8.val[3] = vrshrn_n_u16(s3, 2);

            vst4_u8(dst0, res_u8);

            S00 += 64;
            S01 += 64;
            dst0 += 32;
        }

        for (; dx< dst_w; dx++) {
            *(dst0++) = (unsigned char)((S00[0] + S00[4] + S01[0] + S01[4] + 2) >> 2);
            *(dst0++) = (unsigned char)((S00[1] + S00[5] + S01[1] + S01[5] + 2) >> 2);
            *(dst0++) = (unsigned char)((S00[2] + S00[6] + S01[2] + S01[6] + 2) >> 2);
            *(dst0++) = (unsigned char)((S00[3] + S00[7] + S01[3] + S01[7] + 2) >> 2);
            S00 +=8;
            S01 +=8;
        }

        ptr_src += dou_src_step;
        ptr_dst += d_step;
    }
}

class HalfResizeBilinearC4NeonParallelTask : public ParallelTask {
public:
    HalfResizeBilinearC4NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;

        const uint8_t* ptr_src = _src_ptr + range.start() * dou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;

        const int dst_width_align8 = _dst_w & (~7);
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S00 = ptr_src;
            const uint8_t* S01 = ptr_src + _src_stride;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_width_align8; dx += 8) {
                prefetch_l1(S00, 256);
                prefetch_l1(S01, 256);

                uint8x16x4_t src0_u8 = vld4q_u8(S00);
                uint8x16x4_t src1_u8 = vld4q_u8(S01);

                uint16x8_t s0 = vpaddlq_u8(src0_u8.val[0]);
                uint16x8_t s1 = vpaddlq_u8(src0_u8.val[1]);
                uint16x8_t s2 = vpaddlq_u8(src0_u8.val[2]);
                uint16x8_t s3 = vpaddlq_u8(src0_u8.val[3]);

                s0 = vpadalq_u8(s0, src1_u8.val[0]);
                s1 = vpadalq_u8(s1, src1_u8.val[1]);
                s2 = vpadalq_u8(s2, src1_u8.val[2]);
                s3 = vpadalq_u8(s3, src1_u8.val[3]);

                uint8x8x4_t res_u8;
                res_u8.val[0] = vrshrn_n_u16(s0, 2);
                res_u8.val[1] = vrshrn_n_u16(s1, 2);
                res_u8.val[2] = vrshrn_n_u16(s2, 2);
                res_u8.val[3] = vrshrn_n_u16(s3, 2);

                vst4_u8(dst0, res_u8);

                S00 += 64;
                S01 += 64;
                dst0 += 32;
            }

            for (; dx< _dst_w; dx++) {
                *dst0++ = uint8_t((S00[0] + S00[4] + S01[0] + S01[4] + 2) >> 2);
                *dst0++ = uint8_t((S00[1] + S00[5] + S01[1] + S01[5] + 2) >> 2);
                *dst0++ = uint8_t((S00[2] + S00[6] + S01[2] + S01[6] + 2) >> 2);
                *dst0++ = uint8_t((S00[3] + S00[7] + S01[3] + S01[7] + 2) >> 2);
                S00 +=8;
                S01 +=8;
            }

            ptr_src += dou_src_step;
            ptr_dst += _dst_stride;
        }
    }

private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c4_dn2x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    HalfResizeBilinearC4NeonParallelTask task(src_ptr, src_stride,
            dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

void resize_bilinear_c4_dn4x_neon_old(Mat& src, Mat& dst) {
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_step  = src.stride();
    const int d_step  = dst.stride();
    const int dou_src_step = s_step << 1;
    const int fou_src_step = dou_src_step << 1;
    const int dw_align2 = dst_w & (~1);

    const unsigned char *ptr_src = src_ptr;
    unsigned char *ptr_dst = dst_ptr;

    uint8x8_t src0_u8, src1_u8, src2_u8, src3_u8;
    int dy = 0, dx = 0;
    for (; dy < dst_h; dy++) {
        const unsigned char *S01 = (unsigned char*)(ptr_src + s_step);
        const unsigned char *S02 = (unsigned char*)(ptr_src + dou_src_step);
        unsigned char *dst0 = (unsigned char*)ptr_dst;

        for (dx = 0; dx < dw_align2; dx += 2) {
            src0_u8 = vld1_u8(S01 + 4);
            src1_u8 = vld1_u8(S02 + 4);

            src2_u8 = vld1_u8(S01 + 20);
            src3_u8 = vld1_u8(S02 + 20);

            prefetch_l1(S01, 256);
            prefetch_l1(S02, 256);

            uint16x8_t hsum0_u16  = vaddl_u8(src0_u8, src1_u8); // 4 5 6 7  8 9 10 11
            uint16x8_t hsum1_u16  = vaddl_u8(src2_u8, src3_u8); // 20 21 22 23  24 25 26 27

            uint16x8_t vsum0_u16 = vcombine_u16(vget_low_u16(hsum0_u16), vget_low_u16(hsum1_u16));   // 4 5 6 7   20 21 22 23
            uint16x8_t vsum1_u16 = vcombine_u16(vget_high_u16(hsum0_u16), vget_high_u16(hsum1_u16)); // 8 9 10 11 24 25 26 27

            uint8x8_t res_u8  = vrshrn_n_u16(vaddq_u16(vsum0_u16, vsum1_u16), 2);

            vst1_u8(dst0, res_u8);

            S01 += 32;
            S02 += 32;
            dst0 += 8;
        }

        for (; dx < dst_w; dx++) {
            src0_u8 = vld1_u8(S01 + 4);
            src1_u8 = vld1_u8(S02 + 4);
            prefetch_l1(S01, 256 + 4);
            prefetch_l1(S02, 256 + 4);

            uint16x8_t hsum0_u16  = vaddl_u8(src0_u8, src1_u8);
            uint16x8_t hsum1_u16 = vextq_u16(hsum0_u16, hsum0_u16, 4);//b2 g2 r2
            uint16x8_t vsum_u16  = vaddq_u16(hsum0_u16, hsum1_u16);

            uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

            vst1_u8(dst0, res_u8);

            S01 += 16;
            S02 += 16;
            dst0 += 4;
        }

        ptr_src += fou_src_step;
        ptr_dst += d_step;
    }
}

class QuarterResizeBilinearC4NeonParallelTask : public ParallelTask {
public:
    QuarterResizeBilinearC4NeonParallelTask(
            const uint8_t* src_ptr,
            int src_stride,
            uint8_t* dst_ptr,
            int dst_w, 
            int dst_stride)
            : _src_ptr(src_ptr),
            _src_stride(src_stride),
            _dst_ptr(dst_ptr),
            _dst_w(dst_w), 
            _dst_stride(dst_stride) {}

    void operator() (const Range& range) const override {
        const int dou_src_step = _src_stride << 1;
        const int fou_src_step = dou_src_step << 1;
        const int dst_width_align2 = _dst_w & (~1);

        const uint8_t* ptr_src = _src_ptr + range.start() * fou_src_step;
        uint8_t* ptr_dst = _dst_ptr + range.start() * _dst_stride;
        for (int i = range.start(); i < range.end(); ++i) {
            const uint8_t* S01 = ptr_src + _src_stride;
            const uint8_t* S02 = ptr_src + dou_src_step;
            uint8_t* dst0 = ptr_dst;
            int dx = 0;
            for (; dx < dst_width_align2; dx += 2) {
                uint8x8_t src0_u8 = vld1_u8(S01 + 4);
                uint8x8_t src1_u8 = vld1_u8(S02 + 4);

                uint8x8_t src2_u8 = vld1_u8(S01 + 20);
                uint8x8_t src3_u8 = vld1_u8(S02 + 20);

                prefetch_l1(S01, 256);
                prefetch_l1(S02, 256);

                uint16x8_t hsum0_u16  = vaddl_u8(src0_u8, src1_u8); // 4 5 6 7  8 9 10 11
                uint16x8_t hsum1_u16  = vaddl_u8(src2_u8, src3_u8); // 20 21 22 23  24 25 26 27

                uint16x8_t vsum0_u16 = vcombine_u16(vget_low_u16(hsum0_u16), vget_low_u16(hsum1_u16));   // 4 5 6 7   20 21 22 23
                uint16x8_t vsum1_u16 = vcombine_u16(vget_high_u16(hsum0_u16), vget_high_u16(hsum1_u16)); // 8 9 10 11 24 25 26 27

                uint8x8_t res_u8  = vrshrn_n_u16(vaddq_u16(vsum0_u16, vsum1_u16), 2);

                vst1_u8(dst0, res_u8);

                S01 += 32;
                S02 += 32;
                dst0 += 8;
            }

            for (; dx < _dst_w; dx++) {
                uint8x8_t src0_u8 = vld1_u8(S01 + 4);
                uint8x8_t src1_u8 = vld1_u8(S02 + 4);
                prefetch_l1(S01, 256 + 4);
                prefetch_l1(S02, 256 + 4);

                uint16x8_t hsum0_u16  = vaddl_u8(src0_u8, src1_u8);
                uint16x8_t hsum1_u16 = vextq_u16(hsum0_u16, hsum0_u16, 4);//b2 g2 r2
                uint16x8_t vsum_u16  = vaddq_u16(hsum0_u16, hsum1_u16);

                uint8x8_t res_u8 = vrshrn_n_u16(vsum_u16, 2);

                vst1_u8(dst0, res_u8);

                S01 += 16;
                S02 += 16;
                dst0 += 4;
            }
            ptr_src += fou_src_step;
            ptr_dst += _dst_stride;
        }
    }
private:
    const uint8_t* _src_ptr;
    int _src_stride;
    uint8_t* _dst_ptr;
    int _dst_w;
    int _dst_stride;
};

static void resize_bilinear_c4_dn4x_neon(const Mat& src, Mat& dst) {
    const uint8_t* src_ptr = (const uint8_t*)src.data();
    int src_stride = src.stride();
    uint8_t* dst_ptr = (uint8_t*)dst.data();
    int dst_w = dst.width();
    int dst_h = dst.height();
    int dst_stride = dst.stride();

    QuarterResizeBilinearC4NeonParallelTask task(src_ptr,
            src_stride, dst_ptr, dst_w, dst_stride);
    parallel_run(Range(0, dst_h), task);
}

#define RESIZE_BILINEAR_FUNC_NEON(cn)                                               \
void resize_bilinear_c##cn##_neon(Mat& src, Mat& dst) {                             \
    const int src_w = src.width();                                                  \
    const int src_h = src.height();                                                 \
    const int dst_w = dst.width();                                                  \
    const int dst_h = dst.height();                                                 \
    double scale_x = (double)src_w / dst_w;                                         \
    double scale_y = (double)src_h / dst_h;                                         \
    const double diff = 1e-6;                                                       \
    if (fabs(scale_x - scale_y) < diff) {                                           \
        if (fabs(scale_x - 2.f) < diff) {                                           \
            resize_bilinear_c##cn##_dn2x_neon(src, dst);                            \
        } else if (fabs(scale_x - 4.f) < diff) {                                    \
            resize_bilinear_c##cn##_dn4x_neon(src, dst);                            \
        } else {                                                                    \
            resize_bilinear_c##cn##_comm_neon(src, dst);                            \
        }                                                                           \
    } else {                                                                        \
        resize_bilinear_c##cn##_comm_neon(src, dst);                                \
    }                                                                               \
}

RESIZE_BILINEAR_FUNC_NEON(1)
RESIZE_BILINEAR_FUNC_NEON(3)
RESIZE_BILINEAR_FUNC_NEON(4)
#undef RESIZE_BILINEAR_FUNC_NEON

void horizontal_resize_bn(
        const unsigned char *src0,
        const unsigned char *src1,
        unsigned short *rows0,
        unsigned short *rows1,
        const unsigned short *alphap,
        int* xofs,
        int dst_w) {
    const int dst_width_align8 = dst_w & (~7);
    uint16x8x2_t v_ap0_u16;
    uint8x8x2_t v_row0_u8, v_row1_u8;
    uint32x4_t v_y0l_u32, v_y0h_u32, v_y1l_u32, v_y1h_u32;
    uint16x4_t v_y0l_u16, v_y0h_u16, v_y1l_u16, v_y1h_u16;

    int dx = 0;
    for (dx = 0; dx < dst_width_align8; dx += 8) {
        const int* cx = &xofs[dx];
        v_row0_u8 = v_bilinear_horizonal_elems(src0, cx);
        v_row1_u8 = v_bilinear_horizonal_elems(src1, cx);

        v_ap0_u16 = vld2q_u16(alphap);

        v_y0l_u32 = vmull_u16(vget_low_u16(
                vmovl_u8(v_row0_u8.val[0])), vget_low_u16(v_ap0_u16.val[0]));
        v_y1l_u32 = vmull_u16(vget_low_u16(
                vmovl_u8(v_row1_u8.val[0])), vget_low_u16(v_ap0_u16.val[0]));

        v_y0h_u32 = vmull_u16(vget_high_u16(
                vmovl_u8(v_row0_u8.val[0])), vget_high_u16(v_ap0_u16.val[0]));
        v_y1h_u32 = vmull_u16(vget_high_u16(
                vmovl_u8(v_row1_u8.val[0])), vget_high_u16(v_ap0_u16.val[0]));

        v_y0l_u32 = vmlal_u16(v_y0l_u32, vget_low_u16(
                vmovl_u8(v_row0_u8.val[1])), vget_low_u16(v_ap0_u16.val[1]));
        v_y1l_u32 = vmlal_u16(v_y1l_u32, vget_low_u16(
                vmovl_u8(v_row1_u8.val[1])), vget_low_u16(v_ap0_u16.val[1]));

        v_y0h_u32 = vmlal_u16(v_y0h_u32, vget_high_u16(
                vmovl_u8(v_row0_u8.val[1])), vget_high_u16(v_ap0_u16.val[1]));
        v_y1h_u32 = vmlal_u16(v_y1h_u32, vget_high_u16(
                vmovl_u8(v_row1_u8.val[1])), vget_high_u16(v_ap0_u16.val[1]));

        v_y0l_u16 = vqmovn_u32(vshrq_n_u32(v_y0l_u32, 4));
        v_y0h_u16 = vqmovn_u32(vshrq_n_u32(v_y0h_u32, 4));

        v_y1l_u16 = vqmovn_u32(vshrq_n_u32(v_y1l_u32, 4));
        v_y1h_u16 = vqmovn_u32(vshrq_n_u32(v_y1h_u32, 4));

        vst1q_u16(rows0, vcombine_u16(v_y0l_u16, v_y0h_u16));
        vst1q_u16(rows1, vcombine_u16(v_y1l_u16, v_y1h_u16));

        rows0  += 8;
        rows1  += 8;
        alphap += 16;
    }

    for (; dx < dst_w; dx++) {
        int sx = xofs[dx];
        unsigned short a0 = *(alphap++);
        unsigned short a1 = *(alphap++);

        *(rows0++) = (src0[sx] * a0 + src0[sx + 1] * a1) >> 4;
        *(rows1++) = (src1[sx] * a0 + src1[sx + 1] * a1) >> 4;
    }
}

void resize_bilinear_yuv_comm_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;
    const int dst_width_align8 = dst_w & (~7);
    const int d_stride = dst.stride();
    const int s_stride = src.stride();

    unsigned char *dst_uv_ptr = dst_ptr + d_stride * dst_h;
    unsigned char *src_uv_ptr = src_ptr + s_stride * src_h;

    resize_bilinear_c1_neon_impl(src_ptr, dst_ptr,
            src_w, src_h, dst_w, dst_h, s_stride, d_stride);

    for (int dy = 0; dy < dst_h; dy += 2) {
        const int dst_offset = d_stride * dy;
        unsigned char *ptr_uv_dst = dst_uv_ptr + (dst_offset >> 1);

        //====================uv=======================
        int dx = 0, dx0 = 0, dx1 = 0, dx2 = 0, dx3 = 0;
        int src_y = (int)((dy >> 1) * scale_y);
        unsigned char *uv_ptr = src_uv_ptr + src_y * s_stride;
        for (dx = 0; dx < dst_width_align8; dx += 8) {
            dx0 = ((int)(dx * scale_x) >> 1) <<1;
            dx1 = ((int)((dx + 2) * scale_x) >> 1) << 1;
            dx2 = ((int)((dx + 4) * scale_x) >> 1) << 1;
            dx3 = ((int)((dx + 6) * scale_x) >> 1) << 1;

            unsigned char uv[8] = {uv_ptr[dx0], uv_ptr[dx0 + 1],
                    uv_ptr[dx1], uv_ptr[dx1 + 1],
                    uv_ptr[dx2], uv_ptr[dx2 + 1],
                    uv_ptr[dx3], uv_ptr[dx3+ 1]};
            uint8x8_t v_uv_u8 = vld1_u8(uv);
            vst1_u8(ptr_uv_dst, v_uv_u8);
            ptr_uv_dst += 8;
        }

        for (; dx < dst_w; dx++) {
            int src_x = (int)(dx * scale_x) / 2 * 2;
            unsigned char uv0 = uv_ptr[src_x];
            unsigned char uv1 = uv_ptr[src_x + 1];
            *(ptr_uv_dst++) = uv0;
            *(ptr_uv_dst++) = uv1;
        }

    }
}

int resize_bilinear_neon(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_bilinear_c1_neon(src, dst);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_bilinear_c3_neon(src, dst);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        resize_bilinear_c4_neon(src, dst);
        break;
    case FCVImageType::NV21:
    case FCVImageType::NV12:
        resize_bilinear_yuv_comm_neon(src, dst);
        break;
    default:
        resize_bilinear_common(src, dst);
       // LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

static void vertical_resize_cubic_u16(
        const int *row0,
        const int *row1,
        const int *row2,
        const int *row3,
        int src_width_align8,
        int src_w,
        short b0,
        short b1,
        short b2,
        short b3,
        unsigned char *dst) {
    int32x4_t v_0  = vdupq_n_s32(0);

    int dx = 0;
    for (; dx < src_width_align8; dx += 8) {
        int32x4_t v_s00_s32 = vld1q_s32(row0);
        int32x4_t v_s01_s32 = vld1q_s32(row1);
        int32x4_t v_s02_s32 = vld1q_s32(row2);
        int32x4_t v_s03_s32 = vld1q_s32(row3);

        int32x4_t _d0 = vmulq_n_s32(v_s00_s32, b0);
        int32x4_t v_s10_s32 = vld1q_s32(row0 + 4);

        int32x4_t _d1 = vmulq_n_s32(v_s01_s32, b1);
        int32x4_t v_s11_s32 = vld1q_s32(row1 + 4);

        int32x4_t _d2 = vmulq_n_s32(v_s02_s32, b2);
        int32x4_t v_s12_s32 = vld1q_s32(row2 + 4);

        int32x4_t _d3 = vmulq_n_s32(v_s03_s32, b3);
        int32x4_t v_s13_s32 = vld1q_s32(row3 + 4);

        _d0 = vaddq_s32(_d0, _d1);
        _d2 = vaddq_s32(_d2, _d3);

        int32x4_t row0_s32 = vaddq_s32(_d0, _d2);

        _d0 = vmulq_n_s32(v_s10_s32, b0);
        _d1 = vmulq_n_s32(v_s11_s32, b1);
        _d2 = vmulq_n_s32(v_s12_s32, b2);
        _d3 = vmulq_n_s32(v_s13_s32, b3);

        _d0 = vaddq_s32(_d0, _d1);
        _d2 = vaddq_s32(_d2, _d3);

        int32x4_t row1_s32 = vaddq_s32(_d0, _d2);

        int16x4_t rows_s16_lo = vrshrn_n_s32(vmaxq_s32(row0_s32, v_0), 15);
        int16x4_t rows_s16_hi = vrshrn_n_s32(vmaxq_s32(row1_s32, v_0), 15);

        vst1_u8(dst, vqrshrn_n_u16(vreinterpretq_u16_s16(vcombine_s16(rows_s16_lo, rows_s16_hi)), 7));

        dst += 8;
        row0 += 8;
        row1 += 8;
        row2 += 8;
        row3 += 8;
    }

    for (; dx < src_w; dx++) {
        int res = (*(row0++) * b0 + *(row1++) * b1 + *(row2++) * b2 + *(row3++) * b3);
        *(dst++) = fcv_cast_u8((FCV_MAX(res, 0) + (1 << 21)) >> 22);
    }
}

void resize_cubic_c1_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = dst_w & (~7);

    int* buf = nullptr;
    int *rows = nullptr;
    int buf_size = (dst_h + dst_w) * sizeof(int) + (dst_h + dst_w) * 4 * sizeof(short);
    int rows_size = d_stride * 4 * sizeof(int);
    buf = (int *)malloc(buf_size);
    rows = (int*)malloc(rows_size);
    get_resize_cubic_buf(src_w, src_h, dst_w, dst_h, 1, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    short* alpha = (short*)(yofs + dst_h);
    short* beta = (short*)(alpha + (dst_w << 2));

    int *rows0 = nullptr;
    int *rows1 = nullptr;
    int *rows2 = nullptr;
    int *rows3 = nullptr;

    unsigned char *ptr_dst = nullptr;

    rows0 = rows;
    rows1 = rows0 + d_stride;
    rows2 = rows1 + d_stride;
    rows3 = rows2 + d_stride;

    int prev_sy = -5;
    for (int dy = 0; dy < dst_h; dy++) {
        int sy0 = yofs[dy];
        short *alphap = alpha;
        const int sy_off = sy0 * s_stride;

        if (sy0 == prev_sy) {
            // hresize one row
            int *rows0_tmp = rows0;
            rows0 = rows1;
            rows1 = rows2;
            rows2 = rows3;
            rows3 = rows0_tmp;

            unsigned char *src3 = (unsigned char*)(src_ptr + sy_off + (s_stride * 3));
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int sx = xofs[dx];

                short a0 = *(alphap++);
                short a1 = *(alphap++);
                short a2 = *(alphap++);
                short a3 = *(alphap++);

                *(rows3p++) = (src3[sx] * a0 + src3[sx + 1] * a1 +
                        src3[sx + 2] * a2 + src3[sx + 3] * a3);

            }
        } else if (sy0 == prev_sy + 1) {
            // hresize two row
            int *rows0_tmp = rows0;
            int *rows1_tmp = rows1;
            rows0 = rows2;
            rows1 = rows3;
            rows2 = rows0_tmp;
            rows3 = rows1_tmp;

            unsigned char *src2 = (unsigned char*)(src_ptr + sy_off + (s_stride << 1));
            unsigned char *src3 = src2 + s_stride;

            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int sx = xofs[dx];
                short a0 = *(alphap++);
                short a1 = *(alphap++);
                short a2 = *(alphap++);
                short a3 = *(alphap++);

                *(rows2p++) = (src2[sx] * a0 + src2[sx + 1] * a1 +
                        src2[sx + 2] * a2 + src2[sx + 3] * a3);
                *(rows3p++) = (src3[sx] * a0 + src3[sx + 1] * a1 +
                        src3[sx + 2] * a2 + src3[sx + 3] * a3);
            }

        } else if (sy0 == prev_sy + 2) {
            // hresize three row
            int *rows0_tmp = rows0;
            rows0 = rows3;
            rows3 = rows2;
            rows2 = rows1;
            rows1 = rows0_tmp;

            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);
            unsigned char *src2 = src1 + s_stride;
            unsigned char *src3 = src2 + s_stride;

            int *rows1p = rows1;
            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int sx = xofs[dx];

                short a0 = *(alphap++);
                short a1 = *(alphap++);
                short a2 = *(alphap++);
                short a3 = *(alphap++);

                *(rows1p++) = (src1[sx] * a0 + src1[sx + 1] * a1 +
                        src1[sx + 2] * a2 + src1[sx + 3] * a3);
                *(rows2p++) = (src2[sx] * a0 + src2[sx + 1] * a1 +
                        src2[sx + 2] * a2 + src2[sx + 3] * a3);
                *(rows3p++) = (src3[sx] * a0 + src3[sx + 1] * a1 +
                        src3[sx + 2] * a2 + src3[sx + 3] * a3);
            }
        } else if (sy0 > prev_sy + 2) {
            // hresize four rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = src0 + s_stride;
            unsigned char *src2 = src1 + s_stride;;
            unsigned char *src3 = src2 + s_stride;;

            int *rows0p = rows0;
            int *rows1p = rows1;
            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int sx = xofs[dx];
                short a0 = *(alphap++);
                short a1 = *(alphap++);
                short a2 = *(alphap++);
                short a3 = *(alphap++);

                *(rows0p++) = (src0[sx] * a0 + src0[sx + 1] * a1 +
                        src0[sx + 2] * a2 + src0[sx + 3] * a3);
                *(rows1p++) = (src1[sx] * a0 + src1[sx + 1] * a1 +
                        src1[sx + 2] * a2 + src1[sx + 3] * a3);
                *(rows2p++) = (src2[sx] * a0 + src2[sx + 1] * a1 +
                        src2[sx + 2] * a2 + src2[sx + 3] * a3);
                *(rows3p++) = (src3[sx] * a0 + src3[sx + 1] * a1 +
                        src3[sx + 2] * a2 + src3[sx + 3] * a3);
            }
        }

        prev_sy = sy0 + 1;
        short b0 = beta[dy * 4];
        short b1 = beta[dy * 4 + 1];
        short b2 = beta[dy * 4 + 2];
        short b3 = beta[dy * 4 + 3];

        ptr_dst = dst_ptr + d_stride * dy;
        vertical_resize_cubic_u16(rows0, rows1, rows2, rows3,
                dst_width_align8, dst_w, b0, b1, b2, b3, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_cubic_c3_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = d_stride & (~7);

    int* buf = nullptr;
    int* rows = nullptr;
    int buf_size = (dst_h + dst_w) * sizeof(int) + (dst_h + dst_w) * 4 * sizeof(short);
    int rows_size = d_stride * 4 * sizeof(int);
    buf = (int *)malloc(buf_size);
    rows = (int*)malloc(rows_size);
    get_resize_cubic_buf(src_w, src_h, dst_w, dst_h, 3, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    short* alpha = (short*)(yofs + dst_h);
    short* beta = (short*)(alpha + (dst_w << 2));

    int *rows0 = nullptr;
    int *rows1 = nullptr;
    int *rows2 = nullptr;
    int *rows3 = nullptr;

    unsigned char *ptr_dst = nullptr;

    rows0 = rows;
    rows1 = rows0 + d_stride;
    rows2 = rows1 + d_stride;
    rows3 = rows2 + d_stride;

    int prev_sy = -5;
    for (int dy = 0; dy < dst_h; dy++) {
        int sy0 = yofs[dy];
        ptr_dst = dst_ptr + d_stride * dy;
        short *alphap = alpha;
        const int sy_off = sy0 * s_stride;

        if (sy0 == prev_sy) {
            // hresize one row
            int *rows0_tmp = rows0;
            rows0 = rows1;
            rows1 = rows2;
            rows2 = rows3;
            rows3 = rows0_tmp;

            unsigned char *src3 = (unsigned char*)(src_ptr + sy_off + (s_stride * 3));
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                short a0 = alphap[0];
                short a1 = alphap[1];
                short a2 = alphap[2];
                short a3 = alphap[3];
                const int cx = xofs[dx];
                const unsigned char *s3p = src3 + cx;

                rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                alphap += 4;
                rows3p += 3;

            }

        } else if (sy0 == prev_sy + 1) {
            // hresize two row
            int *rows0_tmp = rows0;
            int *rows1_tmp = rows1;
            rows0 = rows2;
            rows1 = rows3;
            rows2 = rows0_tmp;
            rows3 = rows1_tmp;

            unsigned char *src2 = (unsigned char*)(src_ptr + sy_off + (s_stride << 1));
            unsigned char *src3 = src2 + s_stride;

            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int cx = xofs[dx];

                const unsigned char *s2p = src2 + cx;
                const unsigned char *s3p = src3 + cx;

                short a0 = alphap[0];
                short a1 = alphap[1];
                short a2 = alphap[2];
                short a3 = alphap[3];

                rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                alphap += 4;
                rows2p += 3;
                rows3p += 3;
            }

        } else if (sy0 == prev_sy + 2) {
            // hresize three row
            int *rows0_tmp = rows0;
            rows0 = rows3;
            rows3 = rows2;
            rows2 = rows1;
            rows1 = rows0_tmp;

            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);
            unsigned char *src2 = src1 + s_stride;
            unsigned char *src3 = src2 + s_stride;

            int *rows1p = rows1;
            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int cx = xofs[dx];

                short a0 = alphap[0];
                short a1 = alphap[1];
                short a2 = alphap[2];
                short a3 = alphap[3];

                const unsigned char *s1p = src1 + cx;
                const unsigned char *s2p = src2 + cx;
                const unsigned char *s3p = src3 + cx;

                rows1p[0] = s1p[0] * a0 + s1p[3] * a1 + s1p[6] * a2 + s1p[9 ] * a3;
                rows1p[1] = s1p[1] * a0 + s1p[4] * a1 + s1p[7] * a2 + s1p[10] * a3;
                rows1p[2] = s1p[2] * a0 + s1p[5] * a1 + s1p[8] * a2 + s1p[11] * a3;

                rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                alphap += 4;

                rows1p += 3;
                rows2p += 3;
                rows3p += 3;
            }
        } else if (sy0 > prev_sy + 2) {
            // hresize four rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = src0 + s_stride;
            unsigned char *src2 = src1 + s_stride;
            unsigned char *src3 = src2 + s_stride;

            int *rows0p = rows0;
            int *rows1p = rows1;
            int *rows2p = rows2;
            int *rows3p = rows3;

            for (int dx = 0; dx < dst_w; dx++) {
                const int cx = xofs[dx];

                short a0 = alphap[0];
                short a1 = alphap[1];
                short a2 = alphap[2];
                short a3 = alphap[3];

                const unsigned char *s0p = src0 + cx;
                const unsigned char *s1p = src1 + cx;
                const unsigned char *s2p = src2 + cx;
                const unsigned char *s3p = src3 + cx;

                rows0p[0] = s0p[0] * a0 + s0p[3] * a1 + s0p[6] * a2 + s0p[9 ] * a3;
                rows0p[1] = s0p[1] * a0 + s0p[4] * a1 + s0p[7] * a2 + s0p[10] * a3;
                rows0p[2] = s0p[2] * a0 + s0p[5] * a1 + s0p[8] * a2 + s0p[11] * a3;

                rows1p[0] = s1p[0] * a0 + s1p[3] * a1 + s1p[6] * a2 + s1p[9 ] * a3;
                rows1p[1] = s1p[1] * a0 + s1p[4] * a1 + s1p[7] * a2 + s1p[10] * a3;
                rows1p[2] = s1p[2] * a0 + s1p[5] * a1 + s1p[8] * a2 + s1p[11] * a3;

                rows2p[0] = s2p[0] * a0 + s2p[3] * a1 + s2p[6] * a2 + s2p[9 ] * a3;
                rows2p[1] = s2p[1] * a0 + s2p[4] * a1 + s2p[7] * a2 + s2p[10] * a3;
                rows2p[2] = s2p[2] * a0 + s2p[5] * a1 + s2p[8] * a2 + s2p[11] * a3;

                rows3p[0] = s3p[0] * a0 + s3p[3] * a1 + s3p[6] * a2 + s3p[9 ] * a3;
                rows3p[1] = s3p[1] * a0 + s3p[4] * a1 + s3p[7] * a2 + s3p[10] * a3;
                rows3p[2] = s3p[2] * a0 + s3p[5] * a1 + s3p[8] * a2 + s3p[11] * a3;

                alphap += 4;

                rows0p += 3;
                rows1p += 3;
                rows2p += 3;
                rows3p += 3;
            }

        }

        prev_sy = sy0 + 1;
        int dy4 = dy << 2;
        short b0 = beta[dy4];
        short b1 = beta[dy4 + 1];
        short b2 = beta[dy4 + 2];
        short b3 = beta[dy4 + 3];

        vertical_resize_cubic_u16(rows0, rows1, rows2, rows3,
                dst_width_align8, d_stride, b0, b1, b2, b3, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

int resize_cubic_neon(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_cubic_c1_neon(src, dst);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_cubic_c3_neon(src, dst);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        //resize_bilinear_c4_neon(src, dst);
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

void resize_area_c1_comm_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    unsigned char* src_ptr = (unsigned char *)src.data();
    unsigned char* dst_ptr = (unsigned char *)dst.data();

    double inv_scale_x = static_cast<double>(dst_w) / src_w;
    double inv_scale_y = static_cast<double>(dst_h) / src_h;
    int channel = 1;

    const int dst_width_align8 = dst_w & (~15);

    int* buf = nullptr;
    uint8_t *rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = (d_stride << 3);
    buf = (int *)malloc(buf_size);
    rows = (uint8_t*)malloc(rows_size);
    get_resize_area_buf_c1(src_w, src_h, dst_w, dst_h,
            channel, inv_scale_x, inv_scale_y, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    uint8_t* alpha = (uint8_t*)(yofs + dst_h);
    uint8_t* beta = (uint8_t*)(alpha + dst_w + dst_w);

    uint8_t* rows0 = nullptr;
    uint8_t* rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + dst_w;

    int prev_sy = -1;
    for (int dy = 0; dy < dst_h; dy++) {
        int sy0 = yofs[dy];
        const int sy_off = sy0 * s_stride;

        uint8x8x2_t v_row0_u8, v_row1_u8, v_row2_u8, v_row3_u8;
        uint16x8_t v_y0l_u16, v_y1l_u16, v_y2l_u16, v_y3l_u16;

        uint8_t b0 = beta[dy * 2];
        uint8_t b1 = beta[dy * 2 + 1];

        unsigned char *ptr_dst = dst_ptr + d_stride * dy;

        if (sy0 == prev_sy) {
            uint8_t *alphap = alpha;
            uint8_t *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;

            const unsigned char *src1 = src_ptr + (sy0 + 1) * s_stride;

            uint8_t* rows1p  = rows1;

            for (int dx = 0; dx < dst_width_align8; dx += 16) {
                const int* cx = &xofs[dx];
                uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                // int32x4_t idx0 = vldq_s32(cx);
                // int32x4_t idx1 = vldq_s32(cx + 4);

                //prefetch_l1(src1, 128);
                //prefetch_l1(S01, 256);
                // v_row1_u8 = v_bilinear_horizonal_elems(src1, cx);
                // v_row3_u8 = v_bilinear_horizonal_elems(src1, cx + 8);

                // v_row1_u8 = vld2_u8(src1);
                // v_row3_u8 = vld2_u8(src1 + 8);

                v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                rows1p += 16;
                alphap += 32;
                //src1 += 16;
            }

            for (int dl = dst_width_align8; dl < dst_w; dl++) {
                int sx = xofs[dl];
                uint8_t a0 = *(alphap++);
                uint8_t a1 = *(alphap++);

                *(rows1p++) = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);
            }
        } else if (sy0 > prev_sy) {
            uint8_t *alphap = alpha;
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            uint8_t *rows0p = rows0;
            uint8_t *rows1p = rows1;

            for (int dx = 0; dx < dst_width_align8; dx += 16) {
                const int* cx = &xofs[dx];
                uint8x16x2_t v_ap0_u8 = vld2q_u8(alphap);

                // v_row0_u8 = v_bilinear_horizonal_elems(src0, cx);
                // v_row1_u8 = v_bilinear_horizonal_elems(src1, cx);
                // v_row2_u8 = v_bilinear_horizonal_elems(src0, cx + 8);
               // v_row3_u8 = v_bilinear_horizonal_elems(src1, cx + 8);

                v_row1_u8 = vld2_lane_u8(src1 + cx[0], v_row1_u8, 0);
                v_row1_u8 = vld2_lane_u8(src1 + cx[1], v_row1_u8, 1);
                v_row1_u8 = vld2_lane_u8(src1 + cx[2], v_row1_u8, 2);
                v_row1_u8 = vld2_lane_u8(src1 + cx[3], v_row1_u8, 3);
                v_row1_u8 = vld2_lane_u8(src1 + cx[4], v_row1_u8, 4);
                v_row1_u8 = vld2_lane_u8(src1 + cx[5], v_row1_u8, 5);
                v_row1_u8 = vld2_lane_u8(src1 + cx[6], v_row1_u8, 6);
                v_row1_u8 = vld2_lane_u8(src1 + cx[7], v_row1_u8, 7);

                v_row3_u8 = vld2_lane_u8(src1 + cx[8], v_row3_u8, 0);
                v_row3_u8 = vld2_lane_u8(src1 + cx[9], v_row3_u8, 1);
                v_row3_u8 = vld2_lane_u8(src1 + cx[10], v_row3_u8, 2);
                v_row3_u8 = vld2_lane_u8(src1 + cx[11], v_row3_u8, 3);
                v_row3_u8 = vld2_lane_u8(src1 + cx[12], v_row3_u8, 4);
                v_row3_u8 = vld2_lane_u8(src1 + cx[13], v_row3_u8, 5);
                v_row3_u8 = vld2_lane_u8(src1 + cx[14], v_row3_u8, 6);
                v_row3_u8 = vld2_lane_u8(src1 + cx[15], v_row3_u8, 7);

                v_row0_u8 = vld2_lane_u8(src0 + cx[0], v_row0_u8, 0);
                v_row0_u8 = vld2_lane_u8(src0 + cx[1], v_row0_u8, 1);
                v_row0_u8 = vld2_lane_u8(src0 + cx[2], v_row0_u8, 2);
                v_row0_u8 = vld2_lane_u8(src0 + cx[3], v_row0_u8, 3);
                v_row0_u8 = vld2_lane_u8(src0 + cx[4], v_row0_u8, 4);
                v_row0_u8 = vld2_lane_u8(src0 + cx[5], v_row0_u8, 5);
                v_row0_u8 = vld2_lane_u8(src0 + cx[6], v_row0_u8, 6);
                v_row0_u8 = vld2_lane_u8(src0 + cx[7], v_row0_u8, 7);

                v_row2_u8 = vld2_lane_u8(src0 + cx[8],  v_row2_u8, 0);
                v_row2_u8 = vld2_lane_u8(src0 + cx[9],  v_row2_u8, 1);
                v_row2_u8 = vld2_lane_u8(src0 + cx[10], v_row2_u8, 2);
                v_row2_u8 = vld2_lane_u8(src0 + cx[11], v_row2_u8, 3);
                v_row2_u8 = vld2_lane_u8(src0 + cx[12], v_row2_u8, 4);
                v_row2_u8 = vld2_lane_u8(src0 + cx[13], v_row2_u8, 5);
                v_row2_u8 = vld2_lane_u8(src0 + cx[14], v_row2_u8, 6);
                v_row2_u8 = vld2_lane_u8(src0 + cx[15], v_row2_u8, 7);

                v_y0l_u16 = vmull_u8(v_row0_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));
                v_y1l_u16 = vmull_u8(v_row1_u8.val[0], vget_low_u8(v_ap0_u8.val[0]));

                v_y0l_u16 = vmlal_u8(v_y0l_u16, v_row0_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));
                v_y1l_u16 = vmlal_u8(v_y1l_u16, v_row1_u8.val[1], vget_low_u8(v_ap0_u8.val[1]));

                v_y2l_u16 = vmull_u8(v_row2_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));
                v_y3l_u16 = vmull_u8(v_row3_u8.val[0], vget_high_u8(v_ap0_u8.val[0]));

                v_y2l_u16 = vmlal_u8(v_y2l_u16, v_row2_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));
                v_y3l_u16 = vmlal_u8(v_y3l_u16, v_row3_u8.val[1], vget_high_u8(v_ap0_u8.val[1]));

                vst1q_u8(rows0p, vcombine_u8(vshrn_n_u16(v_y0l_u16, 7), vshrn_n_u16(v_y2l_u16, 7)));
                vst1q_u8(rows1p, vcombine_u8(vshrn_n_u16(v_y1l_u16, 7), vshrn_n_u16(v_y3l_u16, 7)));

                rows0p += 16;
                rows1p += 16;
                alphap += 32;
            }

            for (int dl = dst_width_align8; dl < dst_w; dl++) {
                int sx = xofs[dl];
                uint8_t a0 = *(alphap++);
                uint8_t a1 = *(alphap++);

                uint8_t rows0_res = (uint8_t)((src0[sx] * a0 + src0[sx + 1] * a1) >> 7);
                uint8_t rows1_res = (uint8_t)((src1[sx] * a0 + src1[sx + 1] * a1) >> 7);

                *(rows0p++) = rows0_res;
                *(rows1p++) = rows1_res;
            }
        }

        prev_sy = sy0 + 1;
        vertical_resize_bilinear_u16(rows0, rows1,
                dst_width_align8, dst_w, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_area_c3_comm_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    double inv_scale_x = static_cast<double>(dst_w) / src_w;
    double inv_scale_y = static_cast<double>(dst_h) / src_h;
    int channel = 3;

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = d_stride & (~7);
    int dst_w_1 = dst_w - 1;

    int* buf = nullptr;
    unsigned short* rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = d_stride << 3;
    buf = (int*)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_area_buf(src_w, src_h, dst_w, dst_h,
            channel, inv_scale_x, inv_scale_y, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + d_stride;

    unsigned char *ptr_dst = nullptr;
    int dy = 0, dx= 0, sy0 = 0;
    unsigned short b0 = 0, b1 = 0;
    int prev_sy1 = -1;

    for (dy = 0; dy < dst_h; dy++) {
        ptr_dst = dst_ptr + d_stride * dy;

        sy0 = *(yofs + dy);
        const int sy_off = sy0 * s_stride;
        unsigned short *alphap = alpha;

        if (sy0 == prev_sy1) {
            unsigned short *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            for (dx = 0 ; dx < dst_w_1; dx++) {
                int idx = dx * 3;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);
                uint8x8_t v_s1 = uint8x8_t();

                v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t rows1_s16= vshrn_n_u32(v_rows1, 4);

                vst1_u16(rows1 + idx, rows1_s16);
            }

            {
                int idx = dst_w_1 + (dst_w_1 << 1);
                int cx = xofs[dst_w_1];
                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);
                const unsigned char* S1p = src1 + cx;

                rows1[idx] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            }
        } else if (sy0 > prev_sy1) {
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            unsigned short *rows0p = rows0;
            unsigned short *rows1p = rows1;

            for (dx = 0; dx < dst_w_1; dx++) {
                int idx = dx + (dx << 1);
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);
                uint8x8_t v_s0 = uint8x8_t();
                uint8x8_t v_s1 = uint8x8_t();

                v_s0 = vld1_lane_u8(S0p, v_s0, 0);
                v_s0 = vld1_lane_u8(S0p + 1, v_s0, 1);
                v_s0 = vld1_lane_u8(S0p + 2, v_s0, 2);
                v_s0 = vld1_lane_u8(S0p + 3, v_s0, 3);
                v_s0 = vld1_lane_u8(S0p + 4, v_s0, 4);
                v_s0 = vld1_lane_u8(S0p + 5, v_s0, 5);

                v_s1 = vld1_lane_u8(S1p, v_s1, 0);
                v_s1 = vld1_lane_u8(S1p + 1, v_s1, 1);
                v_s1 = vld1_lane_u8(S1p + 2, v_s1, 2);
                v_s1 = vld1_lane_u8(S1p + 3, v_s1, 3);
                v_s1 = vld1_lane_u8(S1p + 4, v_s1, 4);
                v_s1 = vld1_lane_u8(S1p + 5, v_s1, 5);

                uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                uint16x4_t v_s0_low = vget_low_u16(v_s0_u16);
                uint16x4_t v_s0_hi  = vext_u16(v_s0_low, vget_high_u16(v_s0_u16), 3);
                uint32x4_t v_rows0 = vmull_u16(v_s0_low, v_a0);
                v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                uint16x4_t rows0_u16= vshrn_n_u32(v_rows0, 4);

                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);
                uint16x4_t v_s1_low = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi  = vext_u16(v_s1_low, vget_high_u16(v_s1_u16), 3);
                uint32x4_t v_rows1 = vmull_u16(v_s1_low, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t rows1_u16= vshrn_n_u32(v_rows1, 4);

                vst1_u16(rows0p + idx, rows0_u16);
                vst1_u16(rows1p + idx, rows1_u16);
            }

            {
                int idx = dst_w_1 + (dst_w_1 << 1);
                int cx = xofs[dst_w_1];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;

                rows0p[idx    ] = (S0p[0] * a0 + S0p[3] * a1) >> 4;
                rows0p[idx + 1] = (S0p[1] * a0 + S0p[4] * a1) >> 4;
                rows0p[idx + 2] = (S0p[2] * a0 + S0p[5] * a1) >> 4;

                rows1p[idx    ] = (S1p[0] * a0 + S1p[3] * a1) >> 4;
                rows1p[idx + 1] = (S1p[1] * a0 + S1p[4] * a1) >> 4;
                rows1p[idx + 2] = (S1p[2] * a0 + S1p[5] * a1) >> 4;
            }
        }

        prev_sy1 = sy0 + 1;
        b0 = *(beta++);
        b1 = *(beta++);

        vertical_resize_bilinear_u16(rows0, rows1,
                dst_width_align8, d_stride, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_area_c4_comm_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    double inv_scale_x = static_cast<double>(dst_w) / src_w;
    double inv_scale_y = static_cast<double>(dst_h) / src_h;
    int channel = 4;

    const int s_stride = src.stride();
    const int d_stride = dst.stride();
    const int dst_width_align8 = d_stride & (~7);

    int* buf = nullptr;
    unsigned short* rows = nullptr;
    int buf_size = (dst_w + dst_h) << 3;
    int rows_size = d_stride << 3;
    buf = (int*)malloc(buf_size);
    rows = (unsigned short*)malloc(rows_size);
    get_resize_area_buf(src_w, src_h, dst_w, dst_h,
            channel, inv_scale_x, inv_scale_y, &buf);
    int* xofs = buf;
    int* yofs = buf + dst_w;
    unsigned short* alpha = (unsigned short*)(yofs + dst_h);
    unsigned short* beta  = (unsigned short*)(alpha + dst_w + dst_w);

    unsigned short *rows0 = nullptr;
    unsigned short *rows1 = nullptr;

    rows0 = rows;
    rows1 = rows + d_stride;

    unsigned char *ptr_dst = nullptr;
    int dy = 0, dx= 0, idx = 0, sy0 = 0, cx = 0;
    unsigned short b0 = 0, b1 = 0;

    int prev_sy1 = -1;
    for (; dy < dst_h; dy++) {
        ptr_dst = dst_ptr + d_stride * dy;
        sy0 = *(yofs + dy);
        const int sy_off = sy0 * s_stride;

        if (sy0 == prev_sy1) {
            unsigned short *rows0_old = rows0;
            rows0 = rows1;
            rows1 = rows0_old;
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);
            unsigned short *alphap = alpha;

            for (dx = 0; dx < dst_w; dx++) {
                idx = dx << 2;
                cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s1 = vld1_u8(S1p);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows1_res = vrshrn_n_u32(v_rows1, 4);
                vst1_u16(rows1 + idx, v_rows1_res);
            }
        } else if (sy0 > prev_sy1) {
            // hresize two rows
            unsigned char *src0 = (unsigned char*)(src_ptr + sy_off);
            unsigned char *src1 = (unsigned char*)(src_ptr + sy_off + s_stride);

            unsigned short *rows0p = rows0;
            unsigned short *rows1p = rows1;

            unsigned short *alphap = alpha;
            for (dx = 0; dx < dst_w; dx++) {
                int idx = dx << 2;
                int cx = xofs[dx];

                unsigned short a0 = *(alphap++);
                unsigned short a1 = *(alphap++);

                const unsigned char* S0p = src0 + cx;
                const unsigned char* S1p = src1 + cx;
                uint16x4_t v_a0 = vdup_n_u16(a0);
                uint16x4_t v_a1 = vdup_n_u16(a1);

                uint8x8_t v_s0 = vld1_u8(S0p);
                uint8x8_t v_s1 = vld1_u8(S1p);
                uint16x8_t v_s0_u16 = vmovl_u8(v_s0);
                uint16x8_t v_s1_u16 = vmovl_u8(v_s1);

                uint16x4_t v_s0_lo = vget_low_u16(v_s0_u16);
                uint16x4_t v_s1_lo = vget_low_u16(v_s1_u16);

                uint16x4_t v_s0_hi = vget_high_u16(v_s0_u16);
                uint16x4_t v_s1_hi = vget_high_u16(v_s1_u16);

                uint32x4_t v_rows0 = vmull_u16(v_s0_lo, v_a0);
                uint32x4_t v_rows1 = vmull_u16(v_s1_lo, v_a0);

                v_rows0 = vmlal_u16(v_rows0, v_s0_hi, v_a1);
                v_rows1 = vmlal_u16(v_rows1, v_s1_hi, v_a1);
                uint16x4_t v_rows0_res = vshrn_n_u32(v_rows0, 4);
                uint16x4_t v_rows1_res = vshrn_n_u32(v_rows1, 4);
                vst1_u16(rows0p + idx, v_rows0_res);
                vst1_u16(rows1p + idx, v_rows1_res);
            }
        }

        prev_sy1 = sy0 + 1;
        b0 = *beta++;
        b1 = *beta++;

        vertical_resize_bilinear_u16(rows0, rows1,
                dst_width_align8, d_stride, b0, b1, ptr_dst);
    }

    if (buf != nullptr) {
        free(buf);
        buf = nullptr;
    }

    if (rows != nullptr) {
        free(rows);
        rows = nullptr;
    }
}

void resize_area_cn_neon(Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int dst_w = dst.width();
    const int dst_h = dst.height();
    const int cn = src.channels();
    unsigned char *src_ptr = (unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    double inv_scale_x = static_cast<double>(dst_w) / src_w;
    double inv_scale_y = static_cast<double>(dst_h) / src_h;
    double scale_x = 1. / inv_scale_x;
    double scale_y = 1. / inv_scale_y;

    int iscale_x = fcv_round(scale_x);
    int iscale_y = fcv_round(scale_y);
    bool is_area_fast = std::abs(scale_x - iscale_x) < FCV_EPSILON &&
            std::abs(scale_y - iscale_y) < FCV_EPSILON;

    if (inv_scale_x >= 1 && inv_scale_x >= 1) {
        switch(cn) {
        case 1:
            resize_area_c1_comm_neon(src, dst);
            break;
        case 3:
            resize_area_c3_comm_neon(src, dst);
            break;
        case 4:
            resize_area_c4_comm_neon(src, dst);
            break;
        default:
            break;
        };
    } else {
        int dx = 0, dy = 0;
        bool fast_mode = scale_x == 2 && scale_y == 2 && (cn == 1 || cn == 3 || cn == 4);
        if (is_area_fast && fast_mode) {
            int dwidth1 = (src_w / scale_x) * cn;
            int dst_w1 = dst_w * cn;
            for (dy = 0; dy < dst_h; dy++) {
                unsigned char *dst_row = dst_ptr + dy * dst_w * cn;
                int sy0 = dy * scale_y;
                int w = sy0 + scale_y <= src_h ? dwidth1 : 0;
                if (sy0 >= src_h) {
                    for (dx = 0; dx < dst_w1; dx++) {
                        dst_row[dx] = 0;
                    }
                    continue;
                }

                unsigned char *src_row = src_ptr + sy0 * src_w * cn;
                unsigned char* nextS = src_row + src_w * cn;
                dx = 0;
                uint16x8_t v_2 = vdupq_n_u16(2);

                if (cn == 1) {
                    for (; dx <= w - 16; dx += 16, src_row += 32, nextS += 32, dst_row += 16) {
                        uint8x16x2_t v_row0 = vld2q_u8(src_row), v_row1 = vld2q_u8(nextS);
                        uint16x8_t v_dst0 = vaddl_u8(vget_low_u8(v_row0.val[0]), \
                        vget_low_u8(v_row0.val[1]));
                        v_dst0 = vaddq_u16(v_dst0, vaddl_u8(vget_low_u8(v_row1.val[0]), \
                        vget_low_u8(v_row1.val[1])));
                        v_dst0 = vshrq_n_u16(vaddq_u16(v_dst0, v_2), 2);
                        uint16x8_t v_dst1 = vaddl_u8(vget_high_u8(v_row0.val[0]), vget_high_u8(v_row0.val[1]));
                        v_dst1 = vaddq_u16(v_dst1, vaddl_u8(vget_high_u8(v_row1.val[0]), \
                        vget_high_u8(v_row1.val[1])));
                        v_dst1 = vshrq_n_u16(vaddq_u16(v_dst1, v_2), 2);
                        vst1q_u8(dst_row, vcombine_u8(vmovn_u16(v_dst0), vmovn_u16(v_dst1)));
                    }

                    for(; dx < w; ++dx ) {
                        int index = dx * 2;
                        dst_row[dx] = (src_row[index] + src_row[index + 1] + \
                        nextS[index] + nextS[index+1] + 2) >> 2;
                    }
                } else if (cn == 3) {
                    for (; dx <= w - 24; dx += 24, src_row += 48, nextS += 48, dst_row += 24) {
                        uint8x16x3_t v_row0 = vld3q_u8(src_row), v_row1 = vld3q_u8(nextS);

                        uint16x8_t v_dst0 = vpaddlq_u8(v_row0.val[0]);
                        v_dst0 = vpadalq_u8(v_dst0, v_row1.val[0]);
                        uint16x8_t v_dst1 = vpaddlq_u8(v_row0.val[1]);
                        v_dst1 = vpadalq_u8(v_dst1, v_row1.val[1]);
                        uint16x8_t v_dst2 = vpaddlq_u8(v_row0.val[2]);
                        v_dst2 = vpadalq_u8(v_dst2, v_row1.val[2]);

                        v_dst0 = vshrq_n_u16(vaddq_u16(v_dst0, v_2), 2);
                        v_dst1 = vshrq_n_u16(vaddq_u16(v_dst1, v_2), 2);
                        v_dst2 = vshrq_n_u16(vaddq_u16(v_dst2, v_2), 2);

                        uint8x8x3_t v_dst;
                        v_dst.val[0] = vmovn_u16(v_dst0);
                        v_dst.val[1] = vmovn_u16(v_dst1);
                        v_dst.val[2] = vmovn_u16(v_dst2);
                        vst3_u8(dst_row, v_dst);
                    }

                    for(; dx < w; dx += 3 ) {
                        int index = dx * 2;
                        dst_row[dx] = (src_row[index] + src_row[index+3] + nextS[index] + nextS[index+3] + 2) >> 2;
                        dst_row[dx+1] = (src_row[index+1] + src_row[index+4] + \
                        nextS[index+1] + nextS[index+4] + 2) >> 2;
                        dst_row[dx+2] = (src_row[index+2] + src_row[index+5] + \
                        nextS[index+2] + nextS[index+5] + 2) >> 2;
                    }
                } else if (cn == 4) {
                    for (; dx <= w - 8; dx += 8, src_row += 16, nextS += 16, dst_row += 8) {
                        uint8x16_t v_row0 = vld1q_u8(src_row), v_row1 = vld1q_u8(nextS);

                        uint16x8_t v_row00 = vmovl_u8(vget_low_u8(v_row0));
                        uint16x8_t v_row01 = vmovl_u8(vget_high_u8(v_row0));
                        uint16x8_t v_row10 = vmovl_u8(vget_low_u8(v_row1));
                        uint16x8_t v_row11 = vmovl_u8(vget_high_u8(v_row1));

                        uint16x4_t v_p0 = vadd_u16(vadd_u16(vget_low_u16(v_row00), vget_high_u16(v_row00)),
                                vadd_u16(vget_low_u16(v_row10), vget_high_u16(v_row10)));
                        uint16x4_t v_p1 = vadd_u16(vadd_u16(vget_low_u16(v_row01), vget_high_u16(v_row01)),
                                vadd_u16(vget_low_u16(v_row11), vget_high_u16(v_row11)));
                        uint16x8_t v_dst = vshrq_n_u16(vaddq_u16(vcombine_u16(v_p0, v_p1), v_2), 2);

                        vst1_u8(dst_row, vmovn_u16(v_dst));
                    }

                    for (; dx < w; dx += 4 ) {
                        int index = dx * 2;
                        dst_row[dx] = (src_row[index] + src_row[index+4] + nextS[index] + nextS[index+4] + 2) >> 2;
                        dst_row[dx+1] = (src_row[index+1] + src_row[index+5] + \
                        nextS[index+1] + nextS[index+5] + 2) >> 2;
                        dst_row[dx+2] = (src_row[index+2] + src_row[index+6] + \
                        nextS[index+2] + nextS[index+6] + 2) >> 2;
                        dst_row[dx+3] = (src_row[index+3] + src_row[index+7] + \
                        nextS[index+2] + nextS[index+7] + 2) >> 2;
                    }
                }
            }
        } else {
            resize_area_common(src, dst);
        }
    }
}

int resize_area_neon(Mat& src, Mat& dst) {
    switch (src.type()) {
    case FCVImageType::GRAY_U8:
        resize_area_cn_neon(src, dst);
        break;
    case FCVImageType::PKG_RGB_U8:
    case FCVImageType::PKG_BGR_U8:
        resize_area_cn_neon(src, dst);
        break;
    case FCVImageType::PKG_RGBA_U8:
    case FCVImageType::PKG_BGRA_U8:
        resize_area_cn_neon(src, dst);
        break;
    default:
        LOG_ERR("resize type not support yet!");
        break;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
