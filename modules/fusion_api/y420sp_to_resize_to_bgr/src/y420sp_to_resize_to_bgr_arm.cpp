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

#include "modules/fusion_api/y420sp_to_resize_to_bgr/include/y420sp_to_resize_to_bgr_arm.h"

#include <arm_neon.h>
#include <math.h>
#include <stdlib.h>

#include <iostream>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"
#include "modules/img_transform/resize/include/resize_arm.h"
#include "modules/img_transform/resize/include/resize_common.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class Y420spToResizeBilinearToBgrNeon : public ParallelTask {
public:
    Y420spToResizeBilinearToBgrNeon(
            unsigned char *src,
            int src_w,
            int src_h,
            unsigned char *dst,
            int dst_w,
            int dst_h,
            bool is_nv12)
            : _src_ptr(src),
            _dst_ptr(dst),
            _src_w(src_w),
            _src_h(src_h),
            _dst_w(dst_w),
            _dst_h(dst_h),
            _is_nv12(is_nv12) {
        _buf_size = (_dst_w + _dst_h) << 3;  //(dst_w + dst_h) * 2 * sizeof(int)
        _scale_x = (double)_src_w / _dst_w;
        _scale_y = (double)_src_h / _dst_h;

        _buf = (int *)malloc(_buf_size);
        memset(_buf, 0, _buf_size);

        get_resize_bilinear_buf(_src_w, _src_h, _dst_w, _dst_h, 1, &_buf);
        _xofs = _buf;
        _yofs = _buf + _dst_w;
        _alpha = (unsigned short *)(_yofs + _dst_h);
        _beta = (unsigned short *)(_alpha + _dst_w + _dst_w);
    }

    ~Y420spToResizeBilinearToBgrNeon() {
        if (_buf != nullptr) {
            free(_buf);
            _buf = nullptr;
        }
    }

    void operator()(const Range &range) const {
        unsigned char *src_ptr = _src_ptr;
        unsigned char *dst_ptr = _dst_ptr;
        unsigned char *src_uv_ptr = src_ptr + _src_w * _src_h;

        const int src_w = _src_w;
        const int dst_w = _dst_w;

        const int dst_width_align8 = dst_w & (~7);

        const int d1_stride = dst_w;
        const int d2_stride = dst_w << 1;
        const int d3_stride = dst_w + d2_stride;

        int rows_size = d1_stride << 3;
        unsigned short *rows = (unsigned short *)malloc(rows_size);
        memset(rows, 0, rows_size);

        unsigned short *rows00 = nullptr;
        unsigned short *rows01 = nullptr;
        unsigned short *rows10 = nullptr;
        unsigned short *rows11 = nullptr;
        unsigned char *ptr_dst0 = nullptr;
        unsigned char *ptr_dst1 = nullptr;

        uint8x8_t vc149 = vdup_n_u8(149);
        uint16x4_t vc102 = vdup_n_u16(102);
        uint16x4_t vc14266 = vdup_n_u16(14266);
        uint16x4_t vc52 = vdup_n_u16(52);
        uint16x4_t vc25 = vdup_n_u16(25);
        uint16x4_t vc8679 = vdup_n_u16(8679);
        uint16x4_t vc129 = vdup_n_u16(129);
        uint16x4_t vc17715 = vdup_n_u16(17715);

        int dy = range.start(), sy0 = 0, sy1 = 0;
        unsigned short b0 = 0, b1 = 0, b2 = 0, b3 = 0;
        int dx0 = 0, dx1 = 0, dx2 = 0, dx3 = 0, channel = 3;
        // std::cout << "range start: " << range.start()  << " end " <<
        // range.end() << std::endl;
        for (; dy < range.end(); dy += 2) {
            const int dst_offset = d3_stride * dy;
            rows00 = rows;
            rows01 = rows + d1_stride;
            rows10 = rows + d2_stride;
            rows11 = rows + d3_stride;
            sy0 = *(_yofs + dy);
            sy1 = *(_yofs + dy + 1);

            unsigned short *alphap = _alpha;
            ptr_dst0 = dst_ptr + dst_offset;
            ptr_dst1 = dst_ptr + dst_offset + d3_stride;
            const int sy0_off = sy0 * src_w;
            const int sy1_off = sy1 * src_w;

            // hresize two rows
            unsigned char *src00 = (unsigned char *)(src_ptr + sy0_off);
            unsigned char *src01 = (unsigned char *)(src_ptr + sy0_off + src_w);

            unsigned char *src10 = (unsigned char *)(src_ptr + sy1_off);
            unsigned char *src11 = (unsigned char *)(src_ptr + sy1_off + src_w);

            //================line 0================
            horizontal_resize_bn(src00, src01, rows00, rows01, alphap, _xofs, dst_w);
            //================line 1================
            horizontal_resize_bn(src10, src11, rows10, rows11, alphap, _xofs, dst_w);

            b0 = *(_beta + dy * 2);
            b1 = *(_beta + dy * 2 + 1);
            b2 = *(_beta + dy * 2 + 2);
            b3 = *(_beta + dy * 2 + 3);

            uint16x4_t v_b0 = vdup_n_u16(b0);
            uint16x4_t v_b1 = vdup_n_u16(b1);
            uint16x4_t v_b2 = vdup_n_u16(b2);
            uint16x4_t v_b3 = vdup_n_u16(b3);

            uint8x8x3_t res;
            uint16x8_t y_lo_s16, y_hi_s16;
            int16x8_t b_s16, r_s16, g_s16;
            uint8x8_t b_u8, g_u8, r_u8;
            int src_y = (int)((dy >> 1) * _scale_y);
            unsigned char *uv_ptr = src_uv_ptr + src_y * src_w;
            int dx = 0;
            for (; dx < dst_width_align8; dx += 8) {
                //====================y0=======================
                uint16x8_t v_s00_u16 = vld1q_u16(rows00);
                uint16x8_t v_s01_u16 = vld1q_u16(rows01);

                //====================y1=======================
                uint16x8_t v_s10_u16 = vld1q_u16(rows10);
                uint16x8_t v_s11_u16 = vld1q_u16(rows11);

                uint32x4_t row0_u32_lo = vmull_u16(vget_low_u16(v_s00_u16), v_b0);
                uint32x4_t row0_u32_hi = vmull_u16(vget_high_u16(v_s00_u16), v_b0);
                uint32x4_t row1_u32_lo = vmull_u16(vget_low_u16(v_s10_u16), v_b2);
                uint32x4_t row1_u32_hi = vmull_u16(vget_high_u16(v_s10_u16), v_b2);

                row0_u32_lo = vmlal_u16(row0_u32_lo, vget_low_u16(v_s01_u16), v_b1);
                row0_u32_hi = vmlal_u16(row0_u32_hi, vget_high_u16(v_s01_u16), v_b1);
                row1_u32_lo = vmlal_u16(row1_u32_lo, vget_low_u16(v_s11_u16), v_b3);
                row1_u32_hi = vmlal_u16(row1_u32_hi, vget_high_u16(v_s11_u16), v_b3);

                // y00 = (*(rows00++) * b0 + *(rows01++) * b1 + (1 << 17)) >>
                // 18;
                uint8x8_t v_y0_u8 = vrshrn_n_u16(vcombine_u16(
                        vrshrn_n_u32(row0_u32_lo, 16), vrshrn_n_u32(row0_u32_hi, 16)), 2);
                uint8x8_t v_y1_u8 = vrshrn_n_u16(vcombine_u16(
                        vrshrn_n_u32(row1_u32_lo, 16), vrshrn_n_u32(row1_u32_hi, 16)), 2);

                dx0 = ((int)(dx * _scale_x) >> 1) << 1;
                dx1 = ((int)((dx + 2) * _scale_x) >> 1) << 1;
                dx2 = ((int)((dx + 4) * _scale_x) >> 1) << 1;
                dx3 = ((int)((dx + 6) * _scale_x) >> 1) << 1;

                unsigned short uv[8] = {uv_ptr[dx0],
                        uv_ptr[dx0 + 1],
                        uv_ptr[dx1],
                        uv_ptr[dx1 + 1],
                        uv_ptr[dx2],
                        uv_ptr[dx2 + 1],
                        uv_ptr[dx3],
                        uv_ptr[dx3 + 1]};

                uint16x4x2_t v_uv_u16 = vld2_u16(uv);

                uint16x4_t u_u16 = _is_nv12 ? v_uv_u16.val[0] : v_uv_u16.val[1];
                uint16x4_t v_u16 = _is_nv12 ? v_uv_u16.val[1] : v_uv_u16.val[0];

                uint16x4x2_t w_u_u16 = vzip_u16(u_u16, u_u16);  //[0] u0 u0 u1 u1 [1] u2 u2 u3 u3
                uint16x4x2_t w_v_u16 = vzip_u16(v_u16, v_u16);

                int16x4_t v_vr_lo = vreinterpret_s16_u16(vmls_u16(vc14266, w_v_u16.val[0], vc102));  // v0 v0 v1 v1
                int16x4_t v_ub_lo = vreinterpret_s16_u16(vmls_u16(vc17715, w_u_u16.val[0], vc129));
                int16x4_t v_ug_lo = vreinterpret_s16_u16(vmls_u16(vc8679, w_u_u16.val[0], vc25));
                int16x4_t v_guv_lo = vreinterpret_s16_u16(vmls_u16(v_ug_lo, w_v_u16.val[0], vc52));

                int16x4_t v_vr_hi = vreinterpret_s16_u16(vmls_u16(vc14266, w_v_u16.val[1], vc102));  // v2 v2 v3 v3
                int16x4_t v_ub_hi = vreinterpret_s16_u16(vmls_u16(vc17715, w_u_u16.val[1], vc129));
                int16x4_t v_ug_hi = vreinterpret_s16_u16(vmls_u16(vc8679, w_u_u16.val[1], vc25));
                int16x4_t v_guv_hi = vreinterpret_s16_u16(vmls_u16(v_ug_hi, w_v_u16.val[1], vc52));

                y_lo_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y0_u8, vc149), 1));
                y_hi_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y1_u8, vc149), 1));

                b_s16 = vsubq_s16(y_lo_s16, vcombine_s16(v_ub_lo,
                                                         v_ub_hi));  // y0-ub0 y1-ub0 y2-ub1 y3-ub1
                r_s16 = vsubq_s16(y_lo_s16, vcombine_s16(v_vr_lo, v_vr_hi));
                g_s16 = vaddq_s16(y_lo_s16, vcombine_s16(v_guv_lo, v_guv_hi));

                b_u8 = vqshrun_n_s16(b_s16, 6);
                g_u8 = vqshrun_n_s16(g_s16, 6);
                r_u8 = vqshrun_n_s16(r_s16, 6);

                res.val[0] = b_u8;
                res.val[1] = g_u8;
                res.val[2] = r_u8;

                vst3_u8(ptr_dst0, res);

                b_s16 = vsubq_s16(y_hi_s16, vcombine_s16(v_ub_lo,
                                                         v_ub_hi));  // y0-ub0 y1-ub0 y2-ub1 y3-ub1
                r_s16 = vsubq_s16(y_hi_s16, vcombine_s16(v_vr_lo, v_vr_hi));
                g_s16 = vaddq_s16(y_hi_s16, vcombine_s16(v_guv_lo, v_guv_hi));

                b_u8 = vqshrun_n_s16(b_s16, 6);
                g_u8 = vqshrun_n_s16(g_s16, 6);
                r_u8 = vqshrun_n_s16(r_s16, 6);

                res.val[0] = b_u8;
                res.val[1] = g_u8;
                res.val[2] = r_u8;

                vst3_u8(ptr_dst1, res);

                rows00 += 8;
                rows01 += 8;
                rows10 += 8;
                rows11 += 8;

                ptr_dst0 += 24;
                ptr_dst1 += 24;
            }

            for (; dx < dst_w; dx += 2) {
                int src_x = (int)(dx * _scale_x) / 2 * 2;
                // calculate y
                int y00 = (*(rows00++) * b0 + *(rows01++) * b1 + (1 << 17)) >> 18;
                int y01 = (*(rows00++) * b0 + *(rows01++) * b1 + (1 << 17)) >> 18;

                int y10 = (*(rows10++) * b2 + *(rows11++) * b3 + (1 << 17)) >> 18;
                int y11 = (*(rows10++) * b2 + *(rows11++) * b3 + (1 << 17)) >> 18;

                // calculate uv, nearest interpolation
                int uv0 = _is_nv12 ? uv_ptr[src_x] : uv_ptr[src_x + 1];  // u
                int uv1 = _is_nv12 ? uv_ptr[src_x + 1] : uv_ptr[src_x];  // v

                int vr = uv1 * 102 - 14266;
                int ub = 129 * uv0 - 17715;
                int uvg = 8679 - 52 * uv1 - 25 * uv0;

                convet_yuv_to_one_col(y00, ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(y01, ub, uvg, vr, ptr_dst0, 0, 2, channel);
                ptr_dst0 += channel;
                convet_yuv_to_one_col(y10, ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
                convet_yuv_to_one_col(y11, ub, uvg, vr, ptr_dst1, 0, 2, channel);
                ptr_dst1 += channel;
            }
        }

        if (rows != nullptr) {
            free(rows);
            rows = nullptr;
        }
    }

private:
    unsigned char *_src_ptr;
    unsigned char *_dst_ptr;
    int _src_w;
    int _src_h;
    int _dst_w;
    int _dst_h;
    bool _is_nv12;
    int _buf_size;
    int *_buf;
    double _scale_x;
    double _scale_y;
    int *_xofs;
    int *_yofs;
    unsigned short *_alpha;
    unsigned short *_beta;
};

int y420sp_to_resize_bilinear_to_bgr_neon(Mat &src, Mat &dst, bool is_nv12) {
    Y420spToResizeBilinearToBgrNeon task((unsigned char *)src.data(),
            src.width(), src.height(), (unsigned char *)dst.data(),
            dst.width(), dst.height(), is_nv12);

    parallel_run(Range(0, dst.height()), task, dst.height() / 2);

    return 0;
}

/*
 y00 y01 y02 y03 y04 y05 y06 y07
    --------        --------
 y10|y11 y12|y13 y14|y15 y16|y17
 y20|y21 y22|y23 y24|y25 y26|y27
    --------        --------
 y30 y31 y32 y33 y34 y35 y36 y37

====> (y11+y12+y21+y22+2)>>2  (y15+y16+y25+y26+2)>>2
*/
//class Y420spToResizeBilinearDn4xToBgrNeon : public ParallelTask {
//public:
//    Y420spToResizeBilinearDn4xToBgrNeon(
//            unsigned char *src,
//            int src_w,
//            int src_h,
//            int src_s,
//            unsigned char *dst,
//            int dst_w,
//            int dst_h,
//            int dst_s,
//            bool is_nv12)
//            : _src_ptr(src),
//            _dst_ptr(dst),
//            _src_w(src_w),
//            _src_h(src_h),
//            _src_s(src_s),
//            _dst_w(dst_w),
//            _dst_h(dst_h),
//            _dst_s(dst_s),
//            _is_nv12(is_nv12) {}
//
//    void operator()(const Range &range) const {
//        int src_w = _src_w;
//        int src_h = _src_h;
//        int dst_w = _dst_w;
//
//        const unsigned char *ptr_src = (const unsigned char *)_src_ptr;
//        const unsigned char *src_uv_ptr = ptr_src + src_w * src_h;
//        unsigned char *ptr_dst = (unsigned char *)_dst_ptr;
//        int src_stride = _src_s;
//        int dst_stride = _dst_s;
//
//        const int tri_src_step = src_stride * 3;
//        const int eight_src_step = src_stride << 3;
//        const int dou_dst_step = dst_stride << 1;
//
//        uint8x8_t src00_u8, src01_u8, src10_u8, src11_u8;
//        // std::cout << "BilinearDn4x start: " << range.start()
//        //           << " end: " << range.end() << std::endl;
//        int dy = range.start(), dx = 0, channel = 3;
//        for (; dy < range.end(); dy += 2) {
//            const unsigned char *S01 = ptr_src + dy / 2 * eight_src_step + src_stride;
//            const unsigned char *S02 = S01 + src_stride;
//
//            const unsigned char *S11 = S02 + tri_src_step;
//            const unsigned char *S12 = S11 + src_stride;
//
//            unsigned char *dst0 = (unsigned char *)ptr_dst + dy / 2 * dou_dst_step;
//            unsigned char *dst1 = (unsigned char *)dst0 + dst_stride;
//
//            const unsigned char *uv_ptr = src_uv_ptr + (dy << 1) * src_w;
//
//            for (dx = 0; dx < dst_w; dx += 2) {
//                src00_u8 = vld1_u8(S01 + 1);  // y1 y2 y3 ... y8
//                src01_u8 = vld1_u8(S02 + 1);
//
//                src10_u8 = vld1_u8(S11 + 1);  // y1 y2 y3 ... y8
//                src11_u8 = vld1_u8(S12 + 1);
//
//                uint16x8_t hsum0_u16 = vaddl_u8(src00_u8, src01_u8);  // lin1+lin2
//                uint16x8_t hsum1_u16 = vaddl_u8(src10_u8, src11_u8);
//
//                uint16x4_t h0_u16 = vpadd_u16(vget_low_u16(hsum0_u16),
//                                              vget_high_u16(hsum0_u16));  // a1+a2 a3+a4 a5+a6 a7+a1
//                uint16x4_t h1_u16 = vpadd_u16(vget_low_u16(hsum1_u16), vget_high_u16(hsum1_u16));
//
//                // calculate uv, linear interpolation for horizontal
//                int uv0 = (uv_ptr[2] + uv_ptr[4] + 1) >> 1;
//                int uv1 = (uv_ptr[3] + uv_ptr[5] + 1) >> 1;
//
//                int u = _is_nv12 ? uv0 : uv1;  // u
//                int v = _is_nv12 ? uv1 : uv0;  // v
//
//                int vr = v * 102 - 14216;
//                int ub = 129 * u - 17672;
//                int uvg = 8696 - 52 * v - 25 * u;
//
//                convet_yuv_to_one_col(FCV_MAX(h0_u16[0], 16), ub, uvg, vr, dst0, 0, 2, channel);
//                dst0 += channel;
//                convet_yuv_to_one_col(FCV_MAX(h0_u16[2], 16), ub, uvg, vr, dst0, 0, 2, channel);
//                dst0 += channel;
//                convet_yuv_to_one_col(FCV_MAX(h1_u16[0], 16), ub, uvg, vr, dst1, 0, 2, channel);
//                dst1 += channel;
//                convet_yuv_to_one_col(FCV_MAX(h1_u16[2], 16), ub, uvg, vr, dst1, 0, 2, channel);
//                dst1 += channel;
//
//                uv_ptr += 8;
//            }
//        }
//    }
//
//private:
//    unsigned char *_src_ptr;
//    unsigned char *_dst_ptr;
//    int _src_w;
//    int _src_h;
//    int _src_s;
//    int _dst_w;
//    int _dst_h;
//    int _dst_s;
//    bool _is_nv12;
//};

class Y420spToResizeNearestToBgrGenericNeon : public ParallelTask {
public:
    Y420spToResizeNearestToBgrGenericNeon(
            unsigned char *src,
            int src_w,
            int src_h,
            int src_s,
            unsigned char *dst,
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
            _dst_s(dst_s),
            _is_nv12(is_nv12) {
        _buf_size = (dst_w + dst_h) << 3;  //(dst_w + dst_h + (dst_w + 1) / 2 +
                                           //(dst_h + 1) / 2) * sizeof(int)
        _buf = (int *)malloc(_buf_size);

        _y_xofs = _buf;
        _y_yofs = _buf + dst_w;

        _uv_xofs = _y_yofs + dst_h;
        _uv_yofs = _uv_xofs + (dst_w + 1) / 2;

        nearest_cal_offset(_y_xofs, dst_w, src_w);
        nearest_cal_offset(_y_yofs, dst_h, src_h);

        nearest_cal_offset(_uv_xofs, (dst_w + 1) / 2, (src_w + 1) / 2);
        nearest_cal_offset(_uv_yofs, (dst_h + 1) / 2, (src_h + 1) / 2);
    }

    ~Y420spToResizeNearestToBgrGenericNeon() {
        if (_buf != nullptr) {
            free(_buf);
            _buf = nullptr;
        }
    }

    void operator()(const Range &range) const {
        const int src_w = _src_w;
        const int src_h = _src_h;
        const int dst_w = _dst_w;
        const int src_stride = _src_s;
        const int dst_stride = _dst_s;
        unsigned char *src_ptr = _src_ptr;
        unsigned char *dst_ptr = _dst_ptr;

        unsigned char *src_uv_ptr = src_ptr + src_w * src_h;

        int dy = range.start(), dx = 0, channel = 3;
        // std::cout << "Generic start: " << range.start()
        //           << " end: " << range.end() << std::endl;
        for (; dy < range.end(); dy += 2) {
            const unsigned char *src0 = src_ptr + (_y_yofs[dy] >> 16) * src_stride;
            const unsigned char *src1 = src_ptr + (_y_yofs[dy + 1] >> 16) * src_stride;
            int uv_dy = _uv_yofs[dy >> 1] >> 16;
            unsigned char *uv_ptr = src_uv_ptr + uv_dy * src_stride;

            unsigned char *ptr_dst0 = dst_ptr + dst_stride * dy;
            unsigned char *ptr_dst1 = ptr_dst0 + dst_stride;

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

                // calculate uv, nearest interpolation
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
    unsigned char *_src_ptr;
    unsigned char *_dst_ptr;
    int _src_w;
    int _src_h;
    int _src_s;
    int _dst_w;
    int _dst_s;
    bool _is_nv12;
    int _buf_size;
    int *_buf;
    int *_y_xofs;
    int *_y_yofs;
    int *_uv_xofs;
    int *_uv_yofs;
};

class Y420spToResizeNearestDn2xToBgrNeon : public ParallelTask {
public:
    Y420spToResizeNearestDn2xToBgrNeon(
            unsigned char *src,
            int src_w,
            int src_h,
            int src_s,
            unsigned char *dst,
            int dst_w,
            int dst_s,
            bool is_nv12)
            : _src_ptr(src),
            _dst_ptr(dst),
            _src_w(src_w),
            _src_h(src_h),
            _src_s(src_s),
            _dst_w(dst_w),
            _dst_s(dst_s),
            _is_nv12(is_nv12) {}

    void operator()(const Range &range) const {
        int src_w = _src_w;
        int src_h = _src_h;
        int dst_w = _dst_w;

        const unsigned char *ptr_src = (const unsigned char *)_src_ptr;
        const unsigned char *src_uv_ptr = ptr_src + src_w * src_h;

        unsigned char *ptr_dst = (unsigned char *)_dst_ptr;
        int src_stride = _src_s;
        int dst_stride = _dst_s;

        const int dou_src_step = src_stride << 1;
        const int tri_src_step = src_stride + dou_src_step;
        const int four_src_step = src_stride << 2;

        const int dou_dst_step = dst_stride << 1;

        int width_align16 = dst_w & (~15);
        int remain = dst_w - width_align16;

        uint8x8_t vc149 = vdup_n_u8(149);
        uint8x8_t vc102 = vdup_n_u8(102);
        uint16x8_t vc14216 = vdupq_n_u16(14216);
        uint8x8_t vc52 = vdup_n_u8(52);
        uint8x8_t vc25 = vdup_n_u8(25);
        uint16x8_t vc8696 = vdupq_n_u16(8696);
        uint8x8_t vc129 = vdup_n_u8(129);
        uint16x8_t vc17672 = vdupq_n_u16(17672);
        uint8x16_t vc16 = vdupq_n_u8(16);

        int16x8_t b_odd_s16, r_odd_s16, g_odd_s16, b_eve_s16, g_eve_s16, r_eve_s16;
        uint8x8_t b_odd_u8, b_eve_u8, g_odd_u8, g_eve_u8, r_odd_u8, r_eve_u8;
        uint8x8x2_t vb_u8, vg_u8, vr_u8;
        int16x8_t v_vr, v_ub, v_guv;

        // std::cout << "Dn2x start: " << range.start() << " end: " <<
        // range.end()
        //           << std::endl;

        int dy = range.start(), dx = 0, channel = 3;
        for (; dy < range.end(); dy += 2) {
            const unsigned char *S01 = ptr_src + dy / 2 * four_src_step + src_stride;
            const unsigned char *S02 = ptr_src + dy / 2 * four_src_step + tri_src_step;

            unsigned char *dst0 = (unsigned char *)ptr_dst + dy / 2 * dou_dst_step;
            unsigned char *dst1 = (unsigned char *)ptr_dst + dy / 2 * dou_dst_step + dst_stride;

            const unsigned char *uv_ptr = src_uv_ptr + dy / 2 * dou_src_step + src_stride;

            dx = 0;
            for (; dx < width_align16; dx += 16) {
                uint8x16x2_t y0_u8 = vld2q_u8(S01);  // y0 y1 y2 y3 ... get y1 y3 ...
                uint8x16x2_t y1_u8 = vld2q_u8(S02);

                uint8x8x4_t uv_u8 = vld4_u8(uv_ptr);  // u0 v0 u1 v1 ... get u1 v1

                uint8x16_t v_y0 = y0_u8.val[1];  // 1 3 5 7 ... 15 ... 31
                uint8x16_t v_y1 = y1_u8.val[1];
                uint8x8_t v_u_u8 = _is_nv12 ? uv_u8.val[2] : uv_u8.val[3];  // u
                uint8x8_t v_v_u8 = _is_nv12 ? uv_u8.val[3] : uv_u8.val[2];  // v

                v_y0 = vmaxq_u8(v_y0, vc16);
                v_y1 = vmaxq_u8(v_y1, vc16);
                uint8x8x2_t v_y0_u8 = vuzp_u8(vget_low_u8(v_y0),
                                              vget_high_u8(v_y0));  // [0]1 5 9 13 ... [1] 3 7 11 15...
                uint8x8x2_t v_y1_u8 = vuzp_u8(vget_low_u8(v_y1), vget_high_u8(v_y1));

                int16x8_t y_odd_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y0_u8.val[0], vc149), 1));
                int16x8_t y_eve_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y0_u8.val[1], vc149), 1));

                // calculate uv, attention the data type, avoid overflow,
                // especially negative number
                v_vr = (int16x8_t)vmlsl_u8(vc14216, v_v_u8, vc102);
                v_ub = (int16x8_t)vmlsl_u8(vc17672, v_u_u8, vc129);
                uint16x8_t v_ug = vmlsl_u8(vc8696, v_u_u8, vc25);
                v_guv = (int16x8_t)vmlsl_u8(v_ug, v_v_u8, vc52);

                b_odd_s16 = vsubq_s16(y_odd_s16, v_ub);
                r_odd_s16 = vsubq_s16(y_odd_s16, v_vr);
                g_odd_s16 = vaddq_s16(y_odd_s16, v_guv);

                b_eve_s16 = vsubq_s16(y_eve_s16, v_ub);
                r_eve_s16 = vsubq_s16(y_eve_s16, v_vr);
                g_eve_s16 = vaddq_s16(y_eve_s16, v_guv);

                b_odd_u8 = vqrshrun_n_s16(b_odd_s16, 6);
                b_eve_u8 = vqrshrun_n_s16(b_eve_s16, 6);

                g_odd_u8 = vqrshrun_n_s16(g_odd_s16, 6);
                g_eve_u8 = vqrshrun_n_s16(g_eve_s16, 6);

                r_odd_u8 = vqrshrun_n_s16(r_odd_s16, 6);
                r_eve_u8 = vqrshrun_n_s16(r_eve_s16, 6);

                vb_u8 = vzip_u8(b_odd_u8,
                                b_eve_u8);  //[0]b0 b1 ...b7 [1]b7 b8 b9...b15
                vg_u8 = vzip_u8(g_odd_u8, g_eve_u8);
                vr_u8 = vzip_u8(r_odd_u8, r_eve_u8);

                uint8x16x3_t res;
                res.val[0] = vcombine_u8(vb_u8.val[0], vb_u8.val[1]);
                res.val[1] = vcombine_u8(vg_u8.val[0], vg_u8.val[1]);
                res.val[2] = vcombine_u8(vr_u8.val[0], vr_u8.val[1]);

                vst3q_u8(dst0, res);

                y_odd_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y1_u8.val[0], vc149), 1));
                y_eve_s16 = vreinterpretq_s16_u16(vshrq_n_u16(vmull_u8(v_y1_u8.val[1], vc149), 1));

                b_odd_s16 = vsubq_s16(y_odd_s16, v_ub);
                r_odd_s16 = vsubq_s16(y_odd_s16, v_vr);
                g_odd_s16 = vaddq_s16(y_odd_s16, v_guv);
                b_eve_s16 = vsubq_s16(y_eve_s16, v_ub);
                r_eve_s16 = vsubq_s16(y_eve_s16, v_vr);
                g_eve_s16 = vaddq_s16(y_eve_s16, v_guv);

                b_odd_u8 = vqrshrun_n_s16(b_odd_s16, 6);
                b_eve_u8 = vqrshrun_n_s16(b_eve_s16, 6);

                g_odd_u8 = vqrshrun_n_s16(g_odd_s16, 6);
                g_eve_u8 = vqrshrun_n_s16(g_eve_s16, 6);

                r_odd_u8 = vqrshrun_n_s16(r_odd_s16, 6);
                r_eve_u8 = vqrshrun_n_s16(r_eve_s16, 6);

                vb_u8 = vzip_u8(b_odd_u8,
                                b_eve_u8);  //[0]b0 b1 ...b7 [1]b7 b8 b9...b15
                vg_u8 = vzip_u8(g_odd_u8, g_eve_u8);
                vr_u8 = vzip_u8(r_odd_u8, r_eve_u8);

                res.val[0] = vcombine_u8(vb_u8.val[0], vb_u8.val[1]);
                res.val[1] = vcombine_u8(vg_u8.val[0], vg_u8.val[1]);
                res.val[2] = vcombine_u8(vr_u8.val[0], vr_u8.val[1]);

                vst3q_u8(dst1, res);

                S01 += 32;
                S02 += 32;
                uv_ptr += 32;

                dst0 += 48;
                dst1 += 48;
            }

            if (remain) {
                unsigned char uv0 = 0, uv1 = 0, y00 = 0, y10 = 0, y01 = 0, y11 = 0;
                for (; dx < dst_w; dx += 2) {
                    // calculate y, nearest interpolation
                    y00 = S01[1];
                    y01 = S01[3];

                    y10 = S02[1];
                    y11 = S02[3];

                    // calculate uv
                    uv0 = _is_nv12 ? uv_ptr[2] : uv_ptr[3];  // u
                    uv1 = _is_nv12 ? uv_ptr[3] : uv_ptr[2];  // v

                    int vr = uv1 * 102 - 14216;
                    int ub = 129 * uv0 - 17672;
                    int uvg = 8696 - 52 * uv1 - 25 * uv0;

                    convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, dst0, 0, 2, channel);
                    dst0 += channel;
                    convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, dst0, 0, 2, channel);
                    dst0 += channel;
                    convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, dst1, 0, 2, channel);
                    dst1 += channel;
                    convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, dst1, 0, 2, channel);
                    dst1 += channel;

                    S01 += 4;
                    S02 += 4;

                    uv_ptr += 4;
                }
            }
        }
    }

private:
    unsigned char *_src_ptr;
    unsigned char *_dst_ptr;
    int _src_w;
    int _src_h;
    int _src_s;
    int _dst_w;
    int _dst_s;
    bool _is_nv12;
};

int y420sp_to_resize_nearest_to_bgr_neon(Mat &src, Mat &dst, bool is_nv12) {
    int src_w = src.width();
    int src_h = src.height();
    int dst_w = dst.width();
    int dst_h = dst.height();
    double scale_x = (double)src_w / dst_w;
    double scale_y = (double)src_h / dst_h;

    const double diff = 1e-6;
    if (fabs(scale_x - scale_y) < diff) {
        if (fabs(scale_x - 2.f) < diff) {
            Y420spToResizeNearestDn2xToBgrNeon task((unsigned char *)src.data(),
                    src.width(),
                    src.height(),
                    src.stride(),
                    (unsigned char *)dst.data(),
                    dst.width(),
                    dst.stride(),
                    is_nv12);

            parallel_run(Range(0, dst.height()), task, dst.height() / 2);
        } else {
            Y420spToResizeNearestToBgrGenericNeon task((unsigned char *)src.data(),
                    src.width(),
                    src.height(),
                    src.stride(),
                    (unsigned char *)dst.data(),
                    dst.width(),
                    dst.height(),
                    dst.stride(),
                    is_nv12);

            parallel_run(Range(0, dst.height()), task, dst.height() / 2);
        }
    } else {
        Y420spToResizeNearestToBgrGenericNeon task((unsigned char *)src.data(),
                src.width(),
                src.height(),
                src.stride(),
                (unsigned char *)dst.data(),
                dst.width(),
                dst.height(),
                dst.stride(),
                is_nv12);

        parallel_run(Range(0, dst.height()), task, dst.height() / 2);
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
