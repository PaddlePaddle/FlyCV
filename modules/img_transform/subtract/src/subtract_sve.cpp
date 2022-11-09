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

#include "modules/img_transform/subtract/include/subtract_sve.h"

#include <chrono>

#ifdef HAVE_SVE2
#include <arm_sve.h>
#endif

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#ifdef HAVE_SVE2
void subtract_sve(
        const float* src,
        const int width,
        const int height,
        const int stride,
        const int channel,
        Scalar scalar,
        float* dst) {

    for (int i = 0; i < height; i++) {
        float *src0 = (float*)src;
        float *dst0 = (float*)dst;        

        if (channel == 1) {
            int j = 0;
            float scalar0 = (float)scalar[0];            
            auto rscalar0 = svdup_n_f32(scalar0);
            // pg, loop flag
            svbool_t pg = svwhilelt_b32(j, width);

            do
            {                
                svfloat32_t	vgray = svld1_f32(pg, src0);
                svfloat32_t vgray_res = svsub_f32_z(pg, vgray, rscalar0);	
                
                // store
                svst1_f32(pg, dst0, vgray_res);
                src0 += svcntw();
                dst0 += svcntw();
                j += svcntw();
                pg = svwhilelt_b32(j, width);

            } while (svptest_any(svptrue_b32(), pg));
            
        } else if (channel == 3) {
            int j = 0;
            auto rscalar0 = svdup_n_f32((float)scalar[0]);
            auto rscalar1 = svdup_n_f32((float)scalar[1]);
            auto rscalar2 = svdup_n_f32((float)scalar[2]);

            // pg, loop flag
            svbool_t pg = svwhilelt_b32(j, width);

            do
            {
                svfloat32x3_t vbgr = svld3_f32(pg, src0);

                svfloat32_t vb = svsub_f32_z(pg, svget3_f32(vbgr, 0), rscalar0);	
                svfloat32_t vg = svsub_f32_z(pg, svget3_f32(vbgr, 1), rscalar1);	
                svfloat32_t vr = svsub_f32_z(pg, svget3_f32(vbgr, 2), rscalar2);	
                
                // store                
                svfloat32x3_t bgr_res = svcreate3_f32(vb, vg, vr);
                svst3_f32(pg, dst0, bgr_res);       
                src0 += 3 * svcntw();
                dst0 += 3 * svcntw();
                j += svcntw();
                pg = svwhilelt_b32(j, width);

            } while (svptest_any(svptrue_b32(), pg));
            
        } else if (channel == 4) {

            int j = 0;
            auto rscalar0 = svdup_n_f32((float)scalar[0]);
            auto rscalar1 = svdup_n_f32((float)scalar[1]);
            auto rscalar2 = svdup_n_f32((float)scalar[2]);
            auto rscalar3 = svdup_n_f32((float)scalar[3]);

            // pg, loop flag
            svbool_t pg = svwhilelt_b32(i, width);
            do
            {
                svfloat32x4_t vbgr = svld4_f32(pg, src);

                svfloat32_t vb = svsub_f32_z(pg, svget4_f32(vbgr, 0), rscalar0);	
                svfloat32_t vg = svsub_f32_z(pg, svget4_f32(vbgr, 1), rscalar1);	
                svfloat32_t vr = svsub_f32_z(pg, svget4_f32(vbgr, 2), rscalar2);	
                svfloat32_t va = svsub_f32_z(pg, svget4_f32(vbgr, 3), rscalar3);	
                
                // store                
                svfloat32x4_t bgra_res = svcreate4_f32(vb, vg, vr, va);
                svst4_f32(pg, dst, bgra_res);    
                src0 += 4 * svcntw();
                dst0 += 4 * svcntw();
                j += svcntw();        
                pg = svwhilelt_b32(i, width);

            } while (svptest_any(svptrue_b32(), pg));

        }

        src += stride;
        dst += stride;
    }
   
}

#endif

G_FCV_NAMESPACE1_END()