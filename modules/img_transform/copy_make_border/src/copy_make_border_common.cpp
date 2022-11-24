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

#include "modules/img_transform/copy_make_border/include/copy_make_border_common.h"
#include "modules/core/base/include/type_info.h"
#include "modules/core/base/interface/log.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

//static inline void memcpy_neon(
//        unsigned char* dst,
//        unsigned char* src,
//        unsigned long length) {
//    int count = length / 64;
//    unsigned long *s = (unsigned long *)src;
//    unsigned long *d = (unsigned long *)dst;
//
//    for (int i = 0; i < count; i++) {
//        vst1q_u64(d, vld1q_u64(s)); 
//        vst1q_u64(d + 2, vld1q_u64(s + 2)); 
//        vst1q_u64(d + 4, vld1q_u64(s + 4)); 
//        vst1q_u64(d + 6, vld1q_u64(s + 6));
//        s += 8;
//        d += 8;
//    }
//
//    if (length > count * 64) {
//        memcpy(dst + count, src + count, length - count);
//    }
//}

static inline void memcpy_minisize(unsigned char* dst, const unsigned char* src, size_t len) {
    unsigned char *dd = dst + len;
    const unsigned char *ss = src + len;

    if (len <= 63) {
        switch (len) {
        case 68:    *((int*)(dd - 68)) = *((int*)(ss - 68));
        case 64:	*((int*)(dd - 64)) = *((int*)(ss - 64));
        case 60:	*((int*)(dd - 60)) = *((int*)(ss - 60));
        case 56:	*((int*)(dd - 56)) = *((int*)(ss - 56));
        case 52:	*((int*)(dd - 52)) = *((int*)(ss - 52));
        case 48:	*((int*)(dd - 48)) = *((int*)(ss - 48));
        case 44:	*((int*)(dd - 44)) = *((int*)(ss - 44));
        case 40:	*((int*)(dd - 40)) = *((int*)(ss - 40));
        case 36:	*((int*)(dd - 36)) = *((int*)(ss - 36));
        case 32:	*((int*)(dd - 32)) = *((int*)(ss - 32));
        case 28:	*((int*)(dd - 28)) = *((int*)(ss - 28));
        case 24:	*((int*)(dd - 24)) = *((int*)(ss - 24));
        case 20:	*((int*)(dd - 20)) = *((int*)(ss - 20));
        case 16:	*((int*)(dd - 16)) = *((int*)(ss - 16));
        case 12:	*((int*)(dd - 12)) = *((int*)(ss - 12));
        case  8:	*((int*)(dd - 8)) = *((int*)(ss - 8));
        case  4:	*((int*)(dd - 4)) = *((int*)(ss - 4));
                    break;
        case 67:    *((int*)(dd - 67)) = *((int*)(ss - 67));
        case 63:    *((int*)(dd - 63)) = *((int*)(ss - 63));
        case 59:    *((int*)(dd - 59)) = *((int*)(ss - 59));
        case 55:    *((int*)(dd - 55)) = *((int*)(ss - 55));
        case 51:    *((int*)(dd - 51)) = *((int*)(ss - 51));
        case 47:    *((int*)(dd - 47)) = *((int*)(ss - 47));
        case 43:    *((int*)(dd - 43)) = *((int*)(ss - 43));
        case 39:    *((int*)(dd - 39)) = *((int*)(ss - 39));
        case 35:    *((int*)(dd - 35)) = *((int*)(ss - 35));
        case 31:    *((int*)(dd - 31)) = *((int*)(ss - 31));
        case 27:    *((int*)(dd - 27)) = *((int*)(ss - 27));
        case 23:    *((int*)(dd - 23)) = *((int*)(ss - 23));
        case 19:    *((int*)(dd - 19)) = *((int*)(ss - 19));

        case 15:    *((int*)(dd - 15)) = *((int*)(ss - 15));
        case 11:    *((int*)(dd - 11)) = *((int*)(ss - 11));
        case  7:    *((int*)(dd - 7)) = *((int*)(ss - 7));
                    *((int*)(dd - 4)) = *((int*)(ss - 4));
                    break;
        case  3:    *((short*)(dd - 3)) = *((short*)(ss - 3));
                    dd[-1] = ss[-1];
                    break;
        case 66:    *((int*)(dd - 66)) = *((int*)(ss - 66));
        case 62:    *((int*)(dd - 62)) = *((int*)(ss - 62));
        case 58:    *((int*)(dd - 58)) = *((int*)(ss - 58));
        case 54:    *((int*)(dd - 54)) = *((int*)(ss - 54));
        case 50:    *((int*)(dd - 50)) = *((int*)(ss - 50));
        case 46:    *((int*)(dd - 46)) = *((int*)(ss - 46));
        case 42:    *((int*)(dd - 42)) = *((int*)(ss - 42));
        case 38:    *((int*)(dd - 38)) = *((int*)(ss - 38));
        case 34:    *((int*)(dd - 34)) = *((int*)(ss - 34));
        case 30:    *((int*)(dd - 30)) = *((int*)(ss - 30));
        case 26:    *((int*)(dd - 26)) = *((int*)(ss - 26));
        case 22:    *((int*)(dd - 22)) = *((int*)(ss - 22));
        case 18:    *((int*)(dd - 18)) = *((int*)(ss - 18));
        case 14:    *((int*)(dd - 14)) = *((int*)(ss - 14));
        case 10:    *((int*)(dd - 10)) = *((int*)(ss - 10));
        case  6:    *((int*)(dd - 6)) = *((int*)(ss - 6));
        case  2:    *((short*)(dd - 2)) = *((short*)(ss - 2));
                    break;
        case 65:    *((int*)(dd - 65)) = *((int*)(ss - 65));
        case 61:    *((int*)(dd - 61)) = *((int*)(ss - 61));
        case 57:    *((int*)(dd - 57)) = *((int*)(ss - 57));
        case 53:    *((int*)(dd - 53)) = *((int*)(ss - 53));
        case 49:    *((int*)(dd - 49)) = *((int*)(ss - 49));
        case 45:    *((int*)(dd - 45)) = *((int*)(ss - 45));
        case 41:    *((int*)(dd - 41)) = *((int*)(ss - 41));
        case 37:    *((int*)(dd - 37)) = *((int*)(ss - 37));
        case 33:    *((int*)(dd - 33)) = *((int*)(ss - 33));
        case 29:    *((int*)(dd - 29)) = *((int*)(ss - 29));
        case 25:    *((int*)(dd - 25)) = *((int*)(ss - 25));
        case 21:    *((int*)(dd - 21)) = *((int*)(ss - 21));
        case 17:    *((int*)(dd - 17)) = *((int*)(ss - 17));
        case 13:    *((int*)(dd - 13)) = *((int*)(ss - 13));
        case  9:    *((int*)(dd - 9)) = *((int*)(ss - 9));
        case  5:    *((int*)(dd - 5)) = *((int*)(ss - 5));
        case  1:    dd[-1] = ss[-1];
                    break;
        case 0:
        default: break;
        };

        len -= 4;
    } else {
        memcpy(dst, src, len);
    }
}

template<typename T>
static int copy_make_border_constant(
        Mat& src,
        Mat& dst,
        int top,
        int bottom,
        int left,
        int right,
        const Scalar& value) {
    int channels = src.channels();
    int pixel_bytes = channels * src.type_byte_size();
    int src_step = src.width() * pixel_bytes;
    int dst_step = dst.width() * pixel_bytes;

    std::vector<unsigned char> border_map(dst_step);
    T* border_ptr = reinterpret_cast<T*>(border_map.data());

    int i = 0, j = 0;
    for (i = 0; i < dst.width(); ++i) {
        int idx = i * channels;
        if (channels == 3) {
            border_ptr[idx] = static_cast<T>(value[0]);
            border_ptr[idx + 1] = static_cast<T>(value[1]);
            border_ptr[idx + 2] = static_cast<T>(value[2]);
        } else if (channels == 4) {
            border_ptr[idx] = static_cast<T>(value[0]);
            border_ptr[idx + 1] = static_cast<T>(value[1]);
            border_ptr[idx + 2] = static_cast<T>(value[2]);
            border_ptr[idx + 3] = static_cast<T>(value[3]);
        } else {
            for (j = 0; j < channels; ++j) {
                border_ptr[idx + j] = static_cast<T>(value[j]);
            }
        }
    }

    unsigned char* src_data = reinterpret_cast<unsigned char*>(src.data());
    unsigned char* dst_data = reinterpret_cast<unsigned char*>(dst.data());
    unsigned char* border_data = border_map.data();
    left *= pixel_bytes;
    right *= pixel_bytes; 

    // fill top data
    for (i = 0; i < top; ++i) {
        memcpy(dst_data, border_data, dst_step);
        dst_data += dst_step;
    }

    // fill middle data
    for (i = 0; i < src.height(); ++i) {
        memcpy_minisize(dst_data, border_data, left);
        memcpy(dst_data + left, src_data, src_step);
        memcpy_minisize(dst_data + left + src_step, border_data, right);
        src_data += src_step;
        dst_data += dst_step;
    }

    // fill bottom data
    for (i = 0; i < bottom; ++i) {
        memcpy(dst_data, border_data, dst_step);
        dst_data += dst_step;
    }

    return 0;
}

int copy_make_border_common(
        Mat& src,
        Mat& dst,
        int top,
        int bottom,
        int left,
        int right,
        BorderType border_type,
        const Scalar& value) {
    TypeInfo type_info;

    int status = get_type_info(src.type(), type_info);

    if (status != 0) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    if (border_type == BorderType::BORDER_CONSTANT) {
        switch (type_info.data_type) {
        case DataType::UINT8:
            status = copy_make_border_constant<unsigned char>(src,
                    dst, top, bottom, left, right, value);
            break;
        case DataType::UINT16:
            status = copy_make_border_constant<unsigned short>(src,
                    dst, top, bottom, left, right, value);
            break;
        case DataType::SINT32:
            status = copy_make_border_constant<signed int>(src,
                    dst, top, bottom, left, right, value);
            break;
        case DataType::F32:
            status = copy_make_border_constant<float>(src,
                    dst, top, bottom, left, right, value);
            break;
        case DataType::F64:
            status = copy_make_border_constant<double>(src,
                    dst, top, bottom, left, right, value);
            break;
        default:
            break;
        }
    }

    return status;
}

G_FCV_NAMESPACE1_END()
