// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
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

#include "modules/core/mat/include/mat_convert_to_arm.h"

#include <arm_neon.h>

#include "modules/core/parallel/interface/parallel.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

class ConvertToF32ParallelTask : public ParallelTask {
public:
    ConvertToF32ParallelTask(
            const unsigned char* src,
            float* dst,
            double scale,
            double shift) :
            _src(src),
            _dst(dst),
            _scale(scale),
            _shift(shift) {}

    void operator()(const Range& range) const {
        const unsigned char* src_ptr = _src + range.start();
        float* dst_ptr = _dst + range.start();
        int cnt = range.size() >> 4;
        int remain = range.size() & 15;

        float32x4_t v_scale = vdupq_n_f32(_scale);
        float32x4_t v_shift = vdupq_n_f32(_shift);

        /* the fast way to implement direct type conversion when scale = 1 and shift = 0 */
        if (is_almost_equal(_scale, 1.0) && is_almost_equal(_shift, 0.0)) {
            if (cnt > 0) {
#if __aarch64__
                asm volatile(
                    "0:                                     \n"                    \
                    "prfm  pldl1keep, [%0]                  \n"
                    "ldp     d0, d1, [%[in]], #16           \n" /* load  16 u8*/   \
                    "subs    %w2, %w2, #1                   \n"                    \
                    "ushll   v2.8h, v0.8b, #0               \n" /* trans to u16*/  \
                    "ushll   v3.8h, v1.8b, #0               \n" /* trans to u16*/  \
                    "ushll   v4.4s, v2.4h, #0               \n" /* trans to u32*/  \
                    "ushll2  v5.4s, v2.8h, #0               \n" /* trans to u32*/  \
                    "ushll   v6.4s, v3.4h, #0               \n" /* trans to u32*/  \
                    "ushll2  v7.4s, v3.8h, #0               \n" /* trans to u32*/  \
                    "scvtf   v8.4s, v4.4s                   \n" /* trans to fp32*/ \
                    "scvtf   v9.4s, v5.4s                   \n" /* trans to fp32*/ \
                    "scvtf   v10.4s, v6.4s                  \n" /* trans to fp32*/ \
                    "scvtf   v11.4s, v7.4s                  \n" /* trans to fp32*/ \
                    "stp     q8, q9, [%[out]], #32          \n" /* write to memory*/\
                    "stp     q10, q11, [%[out]], #32        \n"                     \
                    "bne     0b                             \n"
                     : [in] "+r"(src_ptr),
                    [out] "+r"(dst_ptr),
                    [nn] "+r"(cnt)
                    :
                    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
#else
                asm volatile(
                    "0:                                     \n"
                    "vld1.8     {d0-d1}, [%[in]]!           \n" /* load  16 u8*/
                    "vmovl.u8      q2, d0                   \n" /* trans to u16*/
                    "vmovl.u8      q3, d1                   \n" /* trans to u16*/
                    "vmovl.u16     q4, d4                   \n" /* trans to u32*/
                    "vmovl.u16     q5, d5                   \n" /* trans to u32*/
                    "vmovl.u16     q6, d6                   \n" /* trans to u32*/
                    "vmovl.u16     q7, d7                   \n" /* trans to u32*/
                    "vcvt.f32.u32  q8, q4                   \n" /* trans to fp32*/
                    "vcvt.f32.u32  q9, q5                   \n" /* trans to fp32*/
                    "vcvt.f32.u32  q10, q6                  \n" /* trans to fp32*/
                    "vcvt.f32.u32  q11, q7                  \n" /* trans to fp32*/
                    "subs    %2, %2, #1                     \n"
                    "vst1.f32    {d16-d19}, [%[out]]!       \n" /* write to memory*/
                    "vst1.f32    {d20-d23}, [%[out]]!       \n" /* write to memory*/
                    "bne     0b                             \n"
                    : [in] "+r"(src_ptr),
                    [out] "+r"(dst_ptr),
                    [nn] "+r"(cnt)
                    :
                    : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11");
#endif  // __aarch64__
            }

            if (remain) {
                const unsigned char* src_left = _src + range.start() + ((range.size() >> 4) << 4);
                float* dst_left = _dst + range.start() + ((range.size() >> 4) << 4);
                for (int i = 0; i < remain; i++) {
                    dst_left[i] =  static_cast<float>(src_left[i]);
                }
            }
        } else { // the general way to implement direct type conversion when scale != 1 and shift != 0
            if (cnt > 0) {
#if __aarch64__
                asm volatile(
                    "0:                                     \n"                    \
                    "prfm  pldl1keep, [%0]                  \n"
                    "ldp     d0, d1, [%[in]], #16           \n" /* load  16  u8*/  \
                    "subs    %w2, %w2, #1                   \n"                    \
                    "ushll   v2.8h, v0.8b, #0               \n" /* trans to u16*/  \
                    "ushll   v3.8h, v1.8b, #0               \n" /* trans to u16*/  \
                    "ushll   v4.4s, v2.4h, #0               \n" /* trans to u32*/  \
                    "ushll2  v5.4s, v2.8h, #0               \n" /* trans to u32*/  \
                    "ushll   v6.4s, v3.4h, #0               \n" /* trans to u32*/  \
                    "ushll2  v7.4s, v3.8h, #0               \n" /* trans to u32*/  \
                    "scvtf   v8.4s, v4.4s                   \n" /* trans to fp32*/ \
                    "scvtf   v9.4s, v5.4s                   \n" /* trans to fp32*/ \
                    "scvtf   v10.4s, v6.4s                  \n" /* trans to fp32*/ \
                    "scvtf   v11.4s, v7.4s                  \n" /* trans to fp32*/ \
                    "mov     v4.16b, %[shift].16b           \n" /* trans to u16*/  \
                    "mov     v5.16b, %[shift].16b           \n" /* trans to u16*/  \
                    "mov     v6.16b, %[shift].16b           \n" /* trans to u16*/  \
                    "mov     v7.16b, %[shift].16b           \n" /* trans to u16*/  \
                    "fmla    v4.4s, v8.4s, %[scale].4s      \n" /* mul with scale*/\
                    "fmla    v5.4s, v9.4s, %[scale].4s      \n" /* mul with scale*/\
                    "fmla    v6.4s, v10.4s, %[scale].4s     \n" /* mul with scale*/\
                    "fmla    v7.4s, v11.4s, %[scale].4s     \n" /* mul with scale*/\
                    "stp     q4, q5, [%[out]], #32          \n" /* write to memory*/\
                    "stp     q6, q7, [%[out]], #32          \n"                     \
                    "bne     0b                             \n"
                     : [in] "+r"(src_ptr),
                    [out] "+r"(dst_ptr),
                    [nn] "+r"(cnt)
                    : [scale] "w"(v_scale),
                    [shift] "w"(v_shift)
                    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11");
#else
                asm volatile(
                    "0:                                     \n"
                    "vld1.8    {d0-d1}, [%[in]]!            \n" /* load  16 u8*/
                    "subs    %2, %2, #1                     \n"
                    "vmovl.u8      q2, d0                   \n" /* trans to u16*/
                    "vmovl.u8      q3, d1                   \n" /* trans to u16*/
                    "vmovl.u16     q4, d4                   \n" /* trans to u32*/
                    "vmovl.u16     q5, d5                   \n" /* trans to u32*/
                    "vcvt.f32.u32  q8, q4                   \n" /* trans to fp32*/
                    "vcvt.f32.u32  q9, q5                   \n" /* trans to fp32*/
                    "vmovl.u16     q6, d6                   \n" /* trans to u32*/
                    "vmovl.u16     q7, d7                   \n" /* trans to u32*/
                    "vcvt.f32.u32  q10, q6                  \n" /* trans to fp32*/
                    "vcvt.f32.u32  q11, q7                  \n" /* trans to fp32*/
                    "vmov.f32      q4, %q[shift]            \n" /* trans to u16*/
                    "vmov.f32      q5, %q[shift]            \n" /* trans to u16*/
                    "vmov.f32      q6, %q[shift]            \n" /* trans to u32*/
                    "vmov.f32      q7, %q[shift]            \n" /* trans to u32*/
                    "vmla.f32    q4, q8, %q[scale]          \n" /* mul with scale*/
                    "vmla.f32    q5, q9, %q[scale]          \n" /* mul with scale*/
                    "vmla.f32    q6, q10, %q[scale]         \n" /* mul with scale*/
                    "vmla.f32    q7, q11, %q[scale]         \n" /* mul with scale*/
                    "vst1.f32    {d8-d11}, [%[out]]!        \n" /* write to memory*/
                    "vst1.f32    {d12-d15}, [%[out]]!       \n" /* write to memory*/
                    "bne     0b                             \n"
                    : [in] "+r"(src_ptr),
                    [out] "+r"(dst_ptr),
                    [nn] "+r"(cnt)
                    : [scale] "w"(v_scale),
                    [shift] "w"(v_shift)
                    : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10", "q11");
#endif  // __aarch64__
            }

            if (remain) {
                const unsigned char* src_left = _src + range.start() + ((range.size() >> 4) << 4);
                float* dst_left = _dst + range.start() + ((range.size() >> 4) << 4);
                for (int i = 0; i < remain; i++) {
                    dst_left[i] = (src_left[i] * _scale) + _shift;
                }
            }
        }
    }

private:
    const unsigned char* _src;
    float* _dst;
    double _scale;
    double _shift;
};

static void convert_u8_to_f32_neon(
        const unsigned char* src,
        float* dst,
        int count,
        double scale,
        double shift) {
    ConvertToF32ParallelTask task(src, dst, scale, shift);
    parallel_run(Range(0, count), task);
}

int convert_to_neon(
        FCVImageType src_type,
        void* src_data,
        FCVImageType dst_type,
        long dst_cnt,
        void* dst_data,
        double scale,
        double shift) {
    TypeInfo src_type_info;
    get_type_info(src_type, src_type_info);

    TypeInfo dst_type_info;
    get_type_info(dst_type, dst_type_info);

    if (dst_type_info.data_type == DataType::F64) {
        return -1;
    }

    if (src_type_info.data_type == DataType::UINT8
            && dst_type_info.data_type == DataType::F32) {
        convert_u8_to_f32_neon((unsigned char *)src_data,
                (float*)dst_data, dst_cnt, scale, shift);
    } else {
        return -1;
    }

    return 0;
}

G_FCV_NAMESPACE1_END()
