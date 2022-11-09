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

#include "modules/img_transform/color_convert/include/color_convert_sve.h"

#include <chrono>

#ifdef HAVE_SVE2
#include <arm_sve.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef HAVE_SVE2

void bgr2gray_sve(
        const unsigned char* src,
        unsigned char* dst,
        int width,
        int height,
        int src_stride,
        int dst_stride) {

    // Coalesce rows.
    if ((src_stride == width * 3) && (dst_stride == width)) {
        width *= height;
        height = 1;
        src_stride = dst_stride = 0;
    }

    // Coalesce rows.
    auto rb = svdup_n_u8(static_cast<unsigned char>(29));
    auto rg = svdup_n_u8(static_cast<unsigned char>(150));
    auto rr = svdup_n_u8(static_cast<unsigned char>(77));
    auto rz = svdup_n_u8(static_cast<unsigned char>(0));

    auto vdeta = svdup_n_u16(static_cast<unsigned char>(1 << 7));
    auto vq = svdup_n_u8(static_cast<unsigned char>(8));
    auto vq_u16 = svdup_n_u16(static_cast<unsigned char>(8));
    auto vz_u16 = svdup_n_u16(static_cast<unsigned char>(0));

    for (int j = 0; j < height; j++) {
        int      i  = 0;            
        svbool_t pg = svwhilelt_b8(i, width);
        do
        {
            // pg, loop flag
            svuint8x3_t vbgr = svld3(pg, src);
            // top svmovlt
            svuint16_t vtemp_u16_top = svmul_u16_z(pg, svmovlt_u16(rb), svmovlt_u16(svget3(vbgr, 0)));
            vtemp_u16_top = svmlalt_u16(vtemp_u16_top, rg, svget3(vbgr, 1));
            vtemp_u16_top = svmlalt_u16(vtemp_u16_top, rr, svget3(vbgr, 2));
            svuint16_t vsum_top = svadd_u16_m(pg, vtemp_u16_top, vdeta);
            svuint8_t vout_u8_2 = svqshrnb(vsum_top, 8);

            // bottom
            svuint16_t vtemp_u16_bottom = svmul_u16_z(pg, svmovlb_u16(rb), svmovlb_u16(svget3(vbgr, 0)));
            vtemp_u16_bottom = svmlalb_u16(vtemp_u16_bottom, rg, svget3(vbgr, 1));
            vtemp_u16_bottom = svmlalb_u16(vtemp_u16_bottom, rr, svget3(vbgr, 2));
            svuint16_t vsum_bottom = svadd_u16_m(pg, vtemp_u16_bottom, vdeta);
            // shift right
            svuint8_t vout_u8 = svqshrnb(vsum_bottom, 8);
            vout_u8 = svtrn1_u8(vout_u8, vout_u8_2);

            // store
            svst1(pg, dst, vout_u8);

            i += svcntb();
            src += 3 * svcntb();
            dst += svcntb();
            pg = svwhilelt_b8(i, width);

        } while (svptest_any(svptrue_b8(), pg));
    }

}

void rgb2gray_sve(
        const unsigned char* src,
        unsigned char* dst,
        int width,
        int height,
        int src_stride,
        int dst_stride) {

    // Coalesce rows.
    if ((src_stride == width * 3) && (dst_stride == width)) {
        width *= height;
        height = 1;
        src_stride = dst_stride = 0;
    }

    auto rb = svdup_n_u8(static_cast<unsigned char>(29));
    auto rg = svdup_n_u8(static_cast<unsigned char>(150));
    auto rr = svdup_n_u8(static_cast<unsigned char>(77));
    auto rz = svdup_n_u8(static_cast<unsigned char>(0));

    auto vdeta = svdup_n_u16(static_cast<unsigned char>(1 << 7));
    auto vq = svdup_n_u8(static_cast<unsigned char>(8));
    auto vq_u16 = svdup_n_u16(static_cast<unsigned char>(8));
    auto vz_u16 = svdup_n_u16(static_cast<unsigned char>(0));

    for (int j = 0; j < height; j++) {
        int      i  = 0;            
        svbool_t pg = svwhilelt_b8(i, width);
        do
        {
            // pg, loop flag
            svuint8x3_t vrgb = svld3(pg, src);
            // top svmovlt
            svuint16_t vtemp_u16_top = svmul_u16_z(pg, svmovlt_u16(rr), svmovlt_u16(svget3(vrgb, 0)));
            vtemp_u16_top = svmlalt_u16(vtemp_u16_top, rg, svget3(vrgb, 1));
            vtemp_u16_top = svmlalt_u16(vtemp_u16_top, rb, svget3(vrgb, 2));
            svuint16_t vsum_top = svadd_u16_m(pg, vtemp_u16_top, vdeta);
            svuint8_t vout_u8_2 = svqshrnt(rz, vsum_top, 8);

            // bottom
            svuint16_t vtemp_u16_bottom = svmul_u16_z(pg, svmovlb_u16(rr), svmovlb_u16(svget3(vrgb, 0)));
            vtemp_u16_bottom = svmlalb_u16(vtemp_u16_bottom, rg, svget3(vrgb, 1));
            vtemp_u16_bottom = svmlalb_u16(vtemp_u16_bottom, rb, svget3(vrgb, 2));
            svuint16_t vsum_bottom = svadd_u16_m(pg, vtemp_u16_bottom, vdeta);
            // shift right
            svuint8_t vout_u8 = svqshrnb(vsum_bottom, 8);
            // add top and bottom
            vout_u8 = svadd_u8_x(pg, vout_u8, vout_u8_2);

            // store
            svst1(pg, dst, vout_u8);

            i += svcntb();
            src += 3 * svcntb();
            dst += svcntb();
            pg = svwhilelt_b8(i, width);

        } while (svptest_any(svptrue_b8(), pg));
    }

}

#endif

G_FCV_NAMESPACE1_END()