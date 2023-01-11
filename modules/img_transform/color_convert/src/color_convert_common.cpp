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

#include "modules/core/mat/interface/mat.h"
#include "modules/img_transform/color_convert/include/color_convert_common.h"
#include "modules/core/parallel/interface/parallel.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

// get mat data type
int get_cvt_color_dst_mat_type(ColorConvertType type) {
    static std::map<ColorConvertType, FCVImageType> ElemType;
    // init
    if (ElemType.empty()) {
        typedef std::pair<ColorConvertType, FCVImageType> KV;
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2GRAY, FCVImageType::GRAY_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2GRAY, FCVImageType::GRAY_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_GRAY2PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_GRAY2PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_GRAY2PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_GRAY2PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV122PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV212PA_RGB, FCVImageType::PKG_RGB_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV122PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV212PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_I4202PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2PL_BGR, FCVImageType::PLA_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PL_BGR2PA_BGR, FCVImageType::PKG_BGR_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV122PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV212PA_BGRA, FCVImageType::PKG_BGRA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV122PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_NV212PA_RGBA, FCVImageType::PKG_RGBA_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2NV12, FCVImageType::NV12));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2NV12, FCVImageType::NV12));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2NV21, FCVImageType::NV21));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2NV21, FCVImageType::NV21));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2NV12, FCVImageType::NV12));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2NV12, FCVImageType::NV12));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2NV21, FCVImageType::NV21));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2NV21, FCVImageType::NV21));
        ElemType.insert(KV(ColorConvertType::CVT_GRAY2PA_BGR565, FCVImageType::PKG_BGR565_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGR2PA_BGR565, FCVImageType::PKG_BGR565_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGB2PA_BGR565, FCVImageType::PKG_BGR565_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_BGRA2PA_BGR565, FCVImageType::PKG_BGR565_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2PA_BGR565, FCVImageType::PKG_BGR565_U8));
        ElemType.insert(KV(ColorConvertType::CVT_PA_RGBA2PA_mRGBA, FCVImageType::PKG_RGBA_U8));
    }

    std::map<ColorConvertType, FCVImageType>::iterator iter;
    iter = ElemType.find(type);
    if (iter != ElemType.end()) {
        FCVImageType type_val = iter->second;
        return (int)type_val;
    }

    return -1;
}

/****************************************************************************************\
*                                convert yuv to c3/c4                                   *
\****************************************************************************************/
/* R’ = 1.164(Y’– 16) + 1.596(Cr – 128)
 G’ = 1.164(Y’– 16) – 0.813(Cr – 128) – 0.392(Cb – 128)
 B’ = 1.164(Y’– 16) + 2.017(Cb – 128)
浮点乘法用 6位精度处理（即a*b = ((a << 6)*b )>>6
 R’ = 74.5(Y’– 16) + 102(Cr – 128) + 32= 74.5Y' + 102V - 14216
 G’ = 74.5(Y’– 16) – 52(Cr – 128) – 25(Cb – 128) + 32= 74.5Y' - 52V - 25U + 8696
 B’ = 74.5(Y’– 16) + 129(Cb – 128) + 32 = 74.5Y + 129U - 17672*/
void convet_yuv_to_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        const unsigned char *vu,
        int src_w,
        int inter_start,
        int src_stride,
        int dst_stride,
        bool is_nv12,
        int b_idx,
        int r_idx,
        int channel) {
    const unsigned char *y0_ptr = src_ptr ;
    const unsigned char *y1_ptr = src_ptr + src_stride;

    unsigned char *dst0_ptr = dst_ptr ;
    unsigned char *dst1_ptr = dst_ptr + dst_stride;

    int j = inter_start;
    unsigned char y00 = 0, y01 = 0, y10 = 0, y11 = 0, u = 0, v = 0;
    for (; j < src_w; j += 2) {
        y00 = y0_ptr[0];
        y01 = y0_ptr[1];
        y10 = y1_ptr[0];
        y11 = y1_ptr[1];

        if (is_nv12) {
            u = vu[0];
            v = vu[1];
        } else {
            v = vu[0];
            u = vu[1];
        }

        y0_ptr += 2;
        y1_ptr += 2;
        vu += 2;

        int vr = v * 102 - 14216;
        int ub = 129 * u - 17672;
        int uvg = 8696 - 52 * v - 25 * u;

        convet_yuv_to_one_col(FCV_MAX(y00, 16), ub,
                uvg, vr, dst0_ptr, b_idx, r_idx, channel);
        dst0_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y01, 16), ub,
                uvg, vr, dst0_ptr, b_idx, r_idx, channel);
        dst0_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y10, 16), ub,
                uvg, vr, dst1_ptr, b_idx, r_idx, channel);
        dst1_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y11, 16), ub,
                uvg, vr, dst1_ptr, b_idx, r_idx, channel);
        dst1_ptr += channel;
    }
}

void convet_yuv420_to_bgr_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        const unsigned char *u,
        const unsigned char *v,
        int src_w,
        int src_stride,
        int dst_stride,
        int inter_start,
        int channel) {
    const unsigned char *y0_ptr = src_ptr ;
    const unsigned char *y1_ptr = src_ptr + src_stride;

    unsigned char *dst0_ptr = dst_ptr ;
    unsigned char *dst1_ptr = dst_ptr + dst_stride;

    const unsigned char *u0 = u;
    const unsigned char *v0 = v;

    int j = inter_start;
    unsigned char y00 = 0, y01 = 0, y10 = 0, y11 = 0, u00 = 0, v00 = 0;
    for (; j < src_w; j += 2) {
        y00 = y0_ptr[0];
        y01 = y0_ptr[1];
        y10 = y1_ptr[0];
        y11 = y1_ptr[1];

        u00 = u0[0];
        v00 = v0[0];

        y0_ptr += 2;
        y1_ptr += 2;
        v0++;
        u0++;

        int vr = v00 * 102 - 14216;
        int ub = 129 * u00 - 17672;
        int uvg = 8696 - 52 * v00 - 25 * u00;

        convet_yuv_to_one_col(FCV_MAX(y00, 16), ub, uvg, vr, dst0_ptr, 0, 2, channel);
        dst0_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y01, 16), ub, uvg, vr, dst0_ptr, 0, 2, channel);
        dst0_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y10, 16), ub, uvg, vr, dst1_ptr, 0, 2, channel);
        dst1_ptr += channel;
        convet_yuv_to_one_col(FCV_MAX(y11, 16), ub, uvg, vr, dst1_ptr, 0, 2, channel);
        dst1_ptr += channel;
    }
}

//=====================================cvt yuv to bgr===============================

class ConvertYuvToCTask : public ParallelTask {
public:
    ConvertYuvToCTask(const unsigned char* src, 
            const unsigned char* ptr_vu,
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride,
            bool is_nv12,
            int b_idx,
            int r_idx,
            int channel): 
            _src(src), 
            _ptr_vu(ptr_vu),
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _is_nv12(is_nv12),
            _b_idx(b_idx),
            _r_idx(r_idx),
            _channel(channel){}

    void operator() (const Range & range) const {
        const int doub_s_stride = _src_stride << 1;
        const int doub_d_stride = _dst_stride << 1;

        const unsigned char* src_ptr = _src + range.start() * doub_s_stride;
        unsigned char * dst_ptr = _dst + range.start() * doub_d_stride;
        const unsigned char* vu_ptr = _ptr_vu + range.start() * _src_stride;

        for (int i = range.start(); i < range.end(); i += 1) {
            convet_yuv_to_one_row(src_ptr, dst_ptr, vu_ptr, _src_w,
                    0, _src_stride, _dst_stride, _is_nv12, _b_idx, _r_idx, _channel);

            src_ptr += doub_s_stride;
            dst_ptr += doub_d_stride;
            vu_ptr += _src_stride;
        }
    }

private:
    const unsigned char* _src;
    const unsigned char* _ptr_vu;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
    bool _is_nv12;
    int _b_idx;
    int _r_idx;
    int _channel;
};

static void convert_yuv_to_c(
        const Mat& src,
        Mat& dst,
        bool is_nv12,
        int b_idx,
        int r_idx,
        int channel) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));

    const unsigned char *ptr_vu = src_ptr + src_w * src_h;

    const int range_h = src_h / 2;
    ConvertYuvToCTask task(src_ptr, ptr_vu, dst_ptr, src_w, src_stride, 
            dst_stride, is_nv12, b_idx, r_idx, channel);
    parallel_run(Range(0, range_h), task);
}

void convert_nv12_to_rgb_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, true, 2, 0, 3);
}

void convert_nv21_to_rgb_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, false, 2, 0, 3);
}

void convert_nv12_to_bgr_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, true, 0, 2, 3);
}

void convert_nv21_to_bgr_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, false, 0, 2, 3);
}

void convert_nv12_to_rgba_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, true, 2, 0, 4);
}

void convert_nv21_to_rgba_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, false, 2, 0, 4);
}

void convert_nv12_to_bgra_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, true, 0, 2, 4);
}

void convert_nv21_to_bgra_c(const Mat& src, Mat& dst) {
    return convert_yuv_to_c(src, dst, false, 0, 2, 4);
}

class ConvertYuv420ToBgrCTask : public ParallelTask {
public:
    ConvertYuv420ToBgrCTask(const unsigned char* src, 
            unsigned char * dst,
            const unsigned char* u,
            const unsigned char* v,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _u(u),
            _v(v),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const int doub_src_stride = _src_stride << 1;
        const int doub_dst_stride = _dst_stride << 1;
        
        const int uv_stride = _src_stride >> 1;
        const unsigned char* src_ptr = _src + range.start() * doub_src_stride;
        unsigned char * dst_ptr = _dst + range.start() * doub_dst_stride;
        const unsigned char *u = _u + range.start() * uv_stride;
        const unsigned char *v = _v + range.start() * uv_stride;

        for (int i = range.start(); i < range.end(); i += 1) {
            convet_yuv420_to_bgr_one_row(src_ptr, dst_ptr,
                u, v, _src_w, _src_stride, _dst_stride, 0, 3);
            
            src_ptr += doub_src_stride;
            dst_ptr += doub_dst_stride;
            u += uv_stride;
            v += uv_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const unsigned char* _u;
    const unsigned char* _v;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_yuv420_to_bgr_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));
    const unsigned char *ptr_vu = src_ptr + src_stride * src_h;
    const unsigned char *u = ptr_vu;
    const unsigned char *v = ptr_vu + src_stride * (src_h >> 2);

    const int range_h = src_h / 2;
    ConvertYuv420ToBgrCTask task(src_ptr, dst_ptr, u, v, src_w, src_stride, dst_stride);
    parallel_run(Range(0, range_h), task);
}

void convert_yuv420_to_bgr_c(
        const Mat& src,
        Mat& src_u,
        Mat& src_v,
        Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    int dst_w = dst.width();
    CHECK_CVT_SIZE(((src_w % 2) == 0) && ((src_h % 2) == 0));

    const unsigned char *u = (const unsigned char *)src_u.data();
    const unsigned char *v = (const unsigned char *)src_v.data();

    const int range_h = src_h / 2;
    ConvertYuv420ToBgrCTask task(src_ptr, dst_ptr, u, v, src_w, src_stride, dst_stride);
    parallel_run(Range(0, range_h), task);
}

/****************************************************************************************\
*                                convert c3/c4 to yuv                                   *
\****************************************************************************************/
/*
Y = 0.257R + 0.504G + 0.098B + 16 = (66R + 129G + 25B + 0x1080) >> 8
V = 0.439R - 0.368G - 0.071B + 128 = (112R - 94G - 18B + 128 << 8) >> 8
U = -0.148R - 0.291G + 0.439B + 128 = (112B - 74G - 38R + 128 << 8) >> 8
*/
void convert_to_yuv_one_row(
        const unsigned char *src_ptr,
        unsigned char *dst_ptr,
        unsigned char *ptr_vu,
        int src_w,
        int s_stride,
        int iter_start,
        bool is_nv12,
        int b_id,
        int r_id,
        int channel) {
    const unsigned char *src_ptr0 = src_ptr;
    const unsigned char *src_ptr1 = src_ptr + s_stride;
    unsigned char *dst_ptr0 = dst_ptr;
    unsigned char *dst_ptr1 = dst_ptr + src_w;

    unsigned char *uv_ptr = ptr_vu;

    unsigned char b00 = 0, g00 = 0, r00 = 0, b01 = 0, g01 = 0, r01 = 0;
    unsigned char b10 = 0, g10 = 0, r10 = 0, b11 = 0, g11 = 0, r11 = 0;
    int j = iter_start;
    for (; j < src_w; j += 2) {
        b00 = src_ptr0[b_id];
        g00 = src_ptr0[1];
        r00 = src_ptr0[r_id];
        b01 = src_ptr0[b_id + channel];
        g01 = src_ptr0[channel + 1];
        r01 = src_ptr0[r_id + channel];
        b10 = src_ptr1[b_id];
        g10 = src_ptr1[1];
        r10 = src_ptr1[r_id];
        b11 = src_ptr1[b_id + channel];
        g11 = src_ptr1[channel + 1];
        r11 = src_ptr1[r_id + channel];

        src_ptr0 += 2 * channel;
        src_ptr1 += 2 * channel;

        int y00 = (YC + YR * r00 + YG * g00 + YB * b00) >> Q;
        int y01 = (YC + YR * r01 + YG * g01 + YB * b01) >> Q;
        int y10 = (YC + YR * r10 + YG * g10 + YB * b10) >> Q;
        int y11 = (YC + YR * r11 + YG * g11 + YB * b11) >> Q;

        int u00 = (UVC + UR * r00 + UG * g00 + UB * b00) >> Q;
        int v00 = (UVC + VR * r00 + VG * g00 + VB * b00) >> Q;

        dst_ptr0[0] = fcv_cast_u8(y00);
        dst_ptr0[1] = fcv_cast_u8(y01);
        dst_ptr1[0] = fcv_cast_u8(y10);
        dst_ptr1[1] = fcv_cast_u8(y11);

        if (is_nv12) {
            uv_ptr[0] = fcv_cast_u8(u00);
            uv_ptr[1] = fcv_cast_u8(v00);
        } else {
            uv_ptr[0] = fcv_cast_u8(v00);
            uv_ptr[1] = fcv_cast_u8(u00);
        }

        dst_ptr0 += 2;
        dst_ptr1 += 2;
        uv_ptr += 2;
    }
}

//=====================================cvt bgr to yuv=================================

class ConvertToYuvCTask : public ParallelTask {
public:
    ConvertToYuvCTask(const unsigned char* src, 
            unsigned char* ptr_vu,
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride,
            bool is_nv12,
            int b_idx,
            int r_idx,
            int channel): 
            _src(src), 
            _ptr_vu(ptr_vu),
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride),
            _is_nv12(is_nv12),
            _b_idx(b_idx),
            _r_idx(r_idx),
            _channel(channel){}

    void operator() (const Range & range) const {
        const int doub_s_stride = _src_stride << 1;
        const int doub_d_stride = _dst_stride << 1;

        const unsigned char* src_ptr = _src + range.start() * doub_s_stride;
        unsigned char * dst_ptr = _dst + range.start() * doub_d_stride;
        unsigned char* vu_ptr = _ptr_vu + range.start() * _dst_stride;

        for (int i = range.start(); i < range.end(); i += 1) {
            convert_to_yuv_one_row(src_ptr, dst_ptr, vu_ptr,
                _src_w, _src_stride, 0, _is_nv12, _b_idx, _r_idx, _channel);

            dst_ptr += doub_d_stride;
            src_ptr += doub_s_stride;
            vu_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _ptr_vu;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
    bool _is_nv12;
    int _b_idx;
    int _r_idx;
    int _channel;
};

void convert_to_yuv_c(
        const Mat& src,
        Mat& dst,
        bool is_nv_12,
        int b_idx,
        int r_idx,
        int channel) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    CHECK_CVT_SIZE(dst.height() != (src.height() * 3 / 2));

    unsigned char *ptr_vu = dst_ptr + src_w * src_h;
    const int range_h = src_h / 2;
    ConvertToYuvCTask task(src_ptr, ptr_vu, dst_ptr, src_w, src_stride,
            dst_stride, is_nv_12, b_idx, r_idx,channel);
    parallel_run(Range(0, range_h), task);
}

void convert_bgr_to_nv12_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, true, 0, 2, 3);
}

void convert_bgr_to_nv21_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, false, 0, 2, 3);
}

void convert_rgb_to_nv12_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, true, 2, 0, 3);
}

void convert_rgb_to_nv21_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, false, 2, 0, 3);
}

void convert_bgra_to_nv12_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, true, 0, 2, 4);
}

void convert_bgra_to_nv21_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, false, 0, 2, 4);
}

void convert_rgba_to_nv21_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, false, 2, 0, 4);
}

void convert_rgba_to_nv12_c(const Mat& src, Mat& dst) {
    convert_to_yuv_c(src, dst, true, 2, 0, 4);
}

//=================================cvt about bgr/bgra or gray==============================

class ConvertBgrToRgbCTask : public ParallelTask {
public:
    ConvertBgrToRgbCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        

        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                b00 = src_ptr0[0];
                g00 = src_ptr0[1];
                r00 = src_ptr0[2];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;
    
                src_ptr0 += 3;
                dst_ptr0 += 3;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgr_to_rgb_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgrToRgbCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);

}

class ConvertBgrToBgraCTask : public ParallelTask {
public:
    ConvertBgrToBgraCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                b00 = src_ptr0[0];
                g00 = src_ptr0[1];
                r00 = src_ptr0[2];

                dst_ptr0[0] = b00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = r00;
                dst_ptr0[3] = 255;

                src_ptr0 += 3;
                dst_ptr0 += 4;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgr_to_bgra_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgrToBgraCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgrToRgbaCTask : public ParallelTask {
public:
    ConvertBgrToRgbaCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                b00 = src_ptr0[0];
                g00 = src_ptr0[1];
                r00 = src_ptr0[2];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;
                dst_ptr0[3] = 255;

                src_ptr0 += 3;
                dst_ptr0 += 4;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgr_to_rgba_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgrToRgbaCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgraToBgrCTask : public ParallelTask {
public:
    ConvertBgraToBgrCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                b00 = src_ptr0[0];
                g00 = src_ptr0[1];
                r00 = src_ptr0[2];

                dst_ptr0[0] = b00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = r00;

                src_ptr0 += 4;
                dst_ptr0 += 3;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgra_to_bgr_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgraToBgrCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgraToRgbCTask : public ParallelTask {
public:
    ConvertBgraToRgbCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                r00 = src_ptr0[0];
                g00 = src_ptr0[1];
                b00 = src_ptr0[2];

                dst_ptr0[0] = b00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = r00;

                src_ptr0 += 4;
                dst_ptr0 += 3;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgra_to_rgb_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgraToRgbCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgraToRgbaCTask : public ParallelTask {
public:
    ConvertBgraToRgbaCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned char b00 = 0, g00 = 0, r00 = 0, a00 = 0;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;
            for (int j = 0; j < _src_w; j++) {
                b00 = src_ptr0[0];
                g00 = src_ptr0[1];
                r00 = src_ptr0[2];
                a00 = src_ptr0[3];

                dst_ptr0[0] = r00;
                dst_ptr0[1] = g00;
                dst_ptr0[2] = b00;
                dst_ptr0[3] = a00;

                src_ptr0 += 4;
                dst_ptr0 += 4;
            }

            src_ptr += _src_stride;
            dst_ptr += _dst_stride;
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgra_to_rgba_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgraToRgbaCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertRgbToGrayCTask : public ParallelTask {
public:
    ConvertRgbToGrayCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned short temp = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                unsigned char r = *(src_ptr0++);
                unsigned char g = *(src_ptr0++);
                unsigned char b = *(src_ptr0++);
                temp = static_cast<unsigned short>(r * R_RATION + g * G_RATION + b * B_RATION);
                *(dst_ptr0++) = (unsigned char)((temp + (1 << (Q - 1))) >> Q);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_rgb_to_gray_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertRgbToGrayCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);

}

class ConvertBgrToGrayCTask : public ParallelTask {
public:
    ConvertBgrToGrayCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        unsigned short temp = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                unsigned char b = *(src_ptr0++);
                unsigned char g = *(src_ptr0++);
                unsigned char r = *(src_ptr0++);
                temp = static_cast<unsigned short>(r * R_RATION + g * G_RATION + b * B_RATION);
                *(dst_ptr0++) = (unsigned char)((temp + (1 << (Q - 1))) >> Q);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgr_to_gray_c(const Mat& src, Mat& dst) {
    const int src_h = src.height();
    const int src_w = src.width();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgrToGrayCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertGrayToBgrCTask : public ParallelTask {
public:
    ConvertGrayToBgrCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                unsigned char gray = *(src_ptr0++);

                *(dst_ptr0++) = gray;
                *(dst_ptr0++) = gray;
                *(dst_ptr0++) = gray;
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_gray_to_bgr_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertGrayToBgrCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertGrayToBgraCTask : public ParallelTask {
public:
    ConvertGrayToBgraCTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;

        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                unsigned char gray = *(src_ptr0++);

                *(dst_ptr0++) = gray;
                *(dst_ptr0++) = gray;
                *(dst_ptr0++) = gray;
                *(dst_ptr0++) = 255;
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_gray_to_bgra_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertGrayToBgraCTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

// converts R, G, B (B, G, R) pixels to  RGB(BGR)565 format respectively
// 0xF800 1111 1000 0000 0000 high
// 0x07E0 0000 0111 1110 0000
// 0X001F 0000 0000 0001 1111 low
void convertTo565(const unsigned short b,
        const unsigned short g,
        const unsigned short r,
        unsigned char *dst) {
    //rrrr rggg gggb bbbb
    *((unsigned short *)dst) = (b >> 3) | ((g << 3) & (0x07E0)) | ((r << 8) & (0xF800));
}

class ConvertGrayToBgr565CTask : public ParallelTask {
public:
    ConvertGrayToBgr565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        int dj = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                dj = j << 1;
                convertTo565(src_ptr0[j], src_ptr0[j], src_ptr0[j], dst_ptr0 + dj);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_gray_to_bgr565_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertGrayToBgr565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgrToBgr565CTask : public ParallelTask {
public:
    ConvertBgrToBgr565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        int sj = 0, dj = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                sj = j * 3;
                dj = j << 1;
                convertTo565(src_ptr0[sj], src_ptr0[sj + 1], src_ptr0[sj + 2], dst_ptr0 + dj);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgr_to_bgr565_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgrToBgr565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertRgbToBgr565CTask : public ParallelTask {
public:
    ConvertRgbToBgr565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        int sj = 0, dj = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                sj = j * 3;
                dj = j << 1;
                convertTo565(src_ptr0[sj + 2], src_ptr0[sj + 1], src_ptr0[sj], dst_ptr0 + dj);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_rgb_to_bgr565_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertRgbToBgr565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertBgraToBgr565CTask : public ParallelTask {
public:
    ConvertBgraToBgr565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        int sj = 0, dj = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                sj = j << 2;
                dj = j << 1;
                convertTo565(src_ptr0[sj], src_ptr0[sj + 1], src_ptr0[sj + 2], dst_ptr0 + dj);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_bgra_to_bgr565_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertBgraToBgr565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);

}

class ConvertRgbaToBgr565CTask : public ParallelTask {
public:
    ConvertRgbaToBgr565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;
        int sj = 0, dj = 0;
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                sj = j << 2;
                dj = j << 1;
                convertTo565(src_ptr0[sj + 2], src_ptr0[sj + 1], src_ptr0[sj], dst_ptr0 + dj);
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_rgba_to_bgr565_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertRgbaToBgr565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);
}

class ConvertRgbaToMrgba565CTask : public ParallelTask {
public:
    ConvertRgbaToMrgba565CTask(const unsigned char* src, 
            unsigned char * dst,
            const int src_w,
            const int src_stride,
            const int dst_stride): 
            _src(src), 
            _dst(dst),
            _src_w(src_w),
            _src_stride(src_stride),
            _dst_stride(dst_stride){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _src_stride;
        unsigned char * dst_ptr = _dst + range.start() * _dst_stride;

        const unsigned char max_val  = 255;
        const unsigned char half_val = 128;
        
        for (int i = range.start(); i < range.end(); i += 1) {
            const unsigned char *src_ptr0 = src_ptr;
            unsigned char *dst_ptr0 = dst_ptr;

            for (int j = 0; j < _src_w; j++) {
                unsigned char v0 = src_ptr0[0];
                unsigned char v1 = src_ptr0[1];
                unsigned char v2 = src_ptr0[2];
                unsigned char v3 = src_ptr0[3];

                dst_ptr0[0] = (v0 * v3 + half_val) / max_val;
                dst_ptr0[1] = (v1 * v3 + half_val) / max_val;
                dst_ptr0[2] = (v2 * v3 + half_val) / max_val;
                dst_ptr0[3] = v3;

                src_ptr0 += 4;
                dst_ptr0 += 4;
            }
            src_ptr += _src_stride;
            dst_ptr += _dst_stride; 
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _src_w;
    const int _src_stride;
    const int _dst_stride;
};

void convert_rgba_to_mrgba_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const int src_stride = src.stride();
    const int dst_stride = dst.stride();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    ConvertRgbaToMrgba565CTask task(src_ptr, dst_ptr, src_w, src_stride, dst_stride);
    parallel_run(Range(0, src_h), task);

}

//bgrbgr..... to bbbb..ggg...rrr...

class ConvertPackageToPlanerCTask : public ParallelTask {
public:
    ConvertPackageToPlanerCTask(const unsigned char* src, 
            unsigned char * dst,
            const int cnt,
            const int channel): 
            _src(src), 
            _dst(dst),
            _cnt(cnt),
            _channel(channel){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start() * _channel;
        unsigned char * dst_ptr = _dst + range.start();

        if (_channel == 3) {
            // the first address of each channel
            unsigned char *dstb_ptr = dst_ptr;
            unsigned char *dstg_ptr = dstb_ptr + _cnt;
            unsigned char *dstr_ptr = dstg_ptr + _cnt;
            for (int n = range.start(); n < range.end(); n++) {
                *(dstb_ptr++) = *(src_ptr++);
                *(dstg_ptr++) = *(src_ptr++);
                *(dstr_ptr++) = *(src_ptr++);
            }
        } else if (_channel == 4) {
             // the first address of each channel
            unsigned char *dstb_ptr = dst_ptr;
            unsigned char *dstg_ptr = dstb_ptr + _cnt;
            unsigned char *dstr_ptr = dstg_ptr + _cnt;
            unsigned char *dsta_ptr = dstr_ptr + _cnt;
    
            for (int n = range.start(); n < range.end(); n++) {
                *(dstb_ptr++) = *(src_ptr++);
                *(dstg_ptr++) = *(src_ptr++);
                *(dstr_ptr++) = *(src_ptr++);
                *(dsta_ptr++) = *(src_ptr++);
            }
        } else {
            LOG_ERR("the channel of planer convert to package not supported!\n");
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _cnt;
    const int _channel;
};

void convert_package_to_planer_c(const Mat& src, Mat& dst) {
    const int src_w = src.width();
    const int src_h = src.height();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();

    const int channel = src.channels();
    int cnt = src_h * src_w;

    ConvertPackageToPlanerCTask task(src_ptr, dst_ptr, cnt, channel);
    parallel_run(Range(0, cnt), task);
}

//bbb...ggg...rrr... convert bgrbgr...

class ConvertPlanerToPackageCTask : public ParallelTask {
public:
    ConvertPlanerToPackageCTask(const unsigned char* src, 
            unsigned char * dst,
            const int cnt,
            const int channel): 
            _src(src), 
            _dst(dst),
            _cnt(cnt),
            _channel(channel){}

    void operator() (const Range & range) const {
        const unsigned char* src_ptr = _src + range.start();
        unsigned char * dst_ptr = _dst + range.start() * _channel;

        if (_channel == 3) {
            // the first address of each channel
            const unsigned char *srcb_ptr = src_ptr;
            const unsigned char *srcg_ptr = srcb_ptr + _cnt;
            const unsigned char *srcr_ptr = srcg_ptr + _cnt;
            for (int n = range.start(); n < range.end(); n++) {
                *(dst_ptr++) = *(srcb_ptr++);
                *(dst_ptr++) = *(srcg_ptr++);
                *(dst_ptr++) = *(srcr_ptr++);
            }
        } else if (_channel == 4) {
             // the first address of each channel
            const unsigned char *srcb_ptr = src_ptr;
            const unsigned char *srcg_ptr = srcb_ptr + _cnt;
            const unsigned char *srcr_ptr = srcg_ptr + _cnt;
            const unsigned char *srca_ptr = srcr_ptr + _cnt;
    
            for (int n = range.start(); n < range.end(); n++) {
                *(dst_ptr++) = *(srcb_ptr++);
                *(dst_ptr++) = *(srcg_ptr++);
                *(dst_ptr++) = *(srcr_ptr++);
                *(dst_ptr++) = *(srca_ptr++);
            }
        } else {
            LOG_ERR("the channel of planer convert to package not supported!\n");
        }
    }

private:
    const unsigned char* _src;
    unsigned char* _dst;
    const int _cnt;
    const int _channel;
};

void convert_planer_to_package_c(const Mat& src, Mat& dst) {
    const int src_h = src.height();
    const int src_w = src.width();
    const unsigned char *src_ptr = (const unsigned char *)src.data();
    unsigned char *dst_ptr = (unsigned char *)dst.data();
    const int channel = src.channels();
    int cnt = src_h * src_w;

    ConvertPlanerToPackageCTask task(src_ptr, dst_ptr, cnt, channel);
    parallel_run(Range(0, cnt), task);
}

int cvt_color_c(const Mat& src, Mat& dst, ColorConvertType cvt_type) {
    switch (cvt_type) {
    case ColorConvertType::CVT_NV212PA_BGR:
        convert_nv21_to_bgr_c(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_BGR:
        convert_nv12_to_bgr_c(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_RGB:
        convert_nv21_to_rgb_c(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_RGB:
        convert_nv12_to_rgb_c(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_BGRA:
        convert_nv21_to_bgra_c(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_BGRA:
        convert_nv12_to_bgra_c(src, dst);
        break;
    case ColorConvertType::CVT_NV212PA_RGBA:
        convert_nv21_to_rgba_c(src, dst);
        break;
    case ColorConvertType::CVT_NV122PA_RGBA:
        convert_nv12_to_rgba_c(src, dst);
        break;
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGR2NV12:
        convert_bgr_to_nv12_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGR2NV21:
        convert_bgr_to_nv21_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGB2NV12:
        convert_rgb_to_nv12_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGB2NV21:
        convert_rgb_to_nv21_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGRA2NV12:
        convert_bgra_to_nv12_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGRA2NV21:
        convert_bgra_to_nv21_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGBA2NV12:
        convert_rgba_to_nv12_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGBA2NV21:
        convert_rgba_to_nv21_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGR2GRAY:
        convert_bgr_to_gray_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_RGB2GRAY:
        convert_rgb_to_gray_c(src, dst);
        break;

    //cvt from bgr/rgb to rgb/bgr/rgba/bgra
    case ColorConvertType::CVT_PA_BGR2PA_RGB:
    case ColorConvertType::CVT_PA_RGB2PA_BGR:
        convert_bgr_to_rgb_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_BGRA:
    case ColorConvertType::CVT_PA_RGB2PA_RGBA:
        convert_bgr_to_bgra_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_RGBA:
    case ColorConvertType::CVT_PA_RGB2PA_BGRA:
        convert_bgr_to_rgba_c(src, dst);
        break;

    //cvt from bgra/rgba to rgb/bgr/rgba/bgra
    case ColorConvertType::CVT_PA_BGRA2PA_BGR:
    case ColorConvertType::CVT_PA_RGBA2PA_RGB:
        convert_bgra_to_bgr_c(src, dst);
        break;
    case ColorConvertType::CVT_PA_BGRA2PA_RGB:
    case ColorConvertType::CVT_PA_RGBA2PA_BGR:
        convert_bgra_to_rgb_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_BGRA:
    case ColorConvertType::CVT_PA_BGRA2PA_RGBA:
        convert_bgra_to_rgba_c(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_RGB:
    case ColorConvertType::CVT_GRAY2PA_BGR:
        convert_gray_to_bgr_c(src, dst);
        break;

    case ColorConvertType::CVT_GRAY2PA_RGBA:
    case ColorConvertType::CVT_GRAY2PA_BGRA:
        convert_gray_to_bgra_c(src, dst);
        break;

    case ColorConvertType::CVT_PL_BGR2PA_BGR:
        convert_planer_to_package_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PL_BGR:
        convert_package_to_planer_c(src, dst);
        break;

     case ColorConvertType::CVT_GRAY2PA_BGR565:
        convert_gray_to_bgr565_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGR2PA_BGR565:
        convert_bgr_to_bgr565_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGB2PA_BGR565:
        convert_rgb_to_bgr565_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_BGRA2PA_BGR565:
        convert_bgra_to_bgr565_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_BGR565:
        convert_rgba_to_bgr565_c(src, dst);
        break;

    case ColorConvertType::CVT_PA_RGBA2PA_mRGBA:
        convert_rgba_to_mrgba_c(src, dst);
        break;

    default:
        LOG_ERR( "cvt type not support yet !");
        return -1;
    }

    return 0;
}

int cvt_color_c(
        const Mat& src,
        Mat& src_u,
        Mat& src_v,
        Mat& dst,
        ColorConvertType cvt_type) {
    if (src.empty()) {
        LOG_ERR("Input Mat to cvtColor is empty !");
        return -1;
    }

    switch (cvt_type) {
    case ColorConvertType::CVT_I4202PA_BGR:
        convert_yuv420_to_bgr_c(src, src_u, src_v, dst);
        break;
    default:
        LOG_ERR( "cvt type not support yet !");
        return -1;
    };

    return 0;
}

G_FCV_NAMESPACE1_END()
