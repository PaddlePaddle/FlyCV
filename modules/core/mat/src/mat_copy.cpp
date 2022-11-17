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

#include <algorithm>

#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

template<typename T>
static void copy_mask(
        const unsigned char* src,
        int sstep,
        const unsigned char* mask,
        int mstep,
        unsigned char* dst,
        int dstep,
        int width,
        int height,
        int mcn,
        int scn) {
    const T* src_data = (const T*)src;
    T* dst_data = (T*)dst;

    int x = 0;
    if (scn == mcn) {
        int size = width * scn;
        int size_algin4 = (width * scn) & (~3);
        for (; height--; src_data += sstep, dst_data += dstep, mask += mstep) {
            for (x = 0; x < size_algin4; x += 4) {
                if (mask[x]) dst_data[x] = src_data[x];
                if (mask[x+1]) dst_data[x+1] = src_data[x+1];
                if (mask[x+2]) dst_data[x+2] = src_data[x+2];
                if (mask[x+3]) dst_data[x+3] = src_data[x+3];
            }

            for(; x < size; x++ ) {
                if(mask[x]) {
                    dst_data[x] = src_data[x];
                }
            }
        }
    } else if ((mcn == 1) && (scn == 3)){
        int size_algin4 = width & (~3);
        for (; height--; src_data += sstep, dst_data += dstep, mask += mstep) {
            for (x = 0; x < size_algin4; x += 4) {
                int idx = x * 3;
                if (mask[x]) {
                    dst_data[idx] = src_data[idx];
                    dst_data[idx + 1] = src_data[idx + 1];
                    dst_data[idx + 2] = src_data[idx + 2];
                }
                if (mask[x + 1]) {
                    dst_data[idx + 3] = src_data[idx + 3];
                    dst_data[idx + 4] = src_data[idx + 4];
                    dst_data[idx + 5] = src_data[idx + 5];
                }
                if (mask[x + 2]) {
                    dst_data[idx + 6] = src_data[idx + 6];
                    dst_data[idx + 7] = src_data[idx + 7];
                    dst_data[idx + 8] = src_data[idx + 8];
                }
                if (mask[x + 3]) {
                    dst_data[idx + 9] = src_data[idx + 9];
                    dst_data[idx + 10] = src_data[idx + 10];
                    dst_data[idx + 11] = src_data[idx + 11];
                }
            }

            for(; x < width; x++) {
                int idx = x * 3;
                if(mask[x]) {
                    dst_data[idx] = src_data[idx];
                    dst_data[idx + 1] = src_data[idx + 1];
                    dst_data[idx + 2] = src_data[idx + 2];
                }
            }
        }
    } else if ((mcn == 1) && (scn == 4)){
        int size_algin4 = width & (~3);
        for (; height--; src_data += sstep, dst_data += dstep, mask += mstep) {
            for (x = 0; x < size_algin4; x += 4) {
                int idx = x << 2;
                if (mask[x]) {
                    dst_data[idx] = src_data[idx];
                    dst_data[idx + 1] = src_data[idx + 1];
                    dst_data[idx + 2] = src_data[idx + 2];
                    dst_data[idx + 3] = src_data[idx + 3];
                }
                if (mask[x + 1]) {
                    dst_data[idx + 4] = src_data[idx + 4];
                    dst_data[idx + 5] = src_data[idx + 5];
                    dst_data[idx + 6] = src_data[idx + 6];
                    dst_data[idx + 7] = src_data[idx + 7];
                }
                if (mask[x + 2]) {
                    dst_data[idx + 8] = src_data[idx + 8];
                    dst_data[idx + 9] = src_data[idx + 9];
                    dst_data[idx + 10] = src_data[idx + 10];
                    dst_data[idx + 11] = src_data[idx + 11];
                }
                if (mask[x + 3]) {
                    dst_data[idx + 12] = src_data[idx + 12];
                    dst_data[idx + 13] = src_data[idx + 13];
                    dst_data[idx + 14] = src_data[idx + 14];
                    dst_data[idx + 15] = src_data[idx + 15];
                }
            }

            for(; x < width; x++) {
                int idx = x << 2;
                if(mask[x]) {
                    dst_data[idx] = src_data[idx];
                    dst_data[idx + 1] = src_data[idx + 1];
                    dst_data[idx + 2] = src_data[idx + 2];
                    dst_data[idx + 3] = src_data[idx + 3];
                }
            }
        }
    } else {
        LOG_ERR("The data type of src is not supported!");
        return;
    }
}

// dst = src
void Mat::copy_to(Mat& dst) const {
    if (dst.empty()) {
        dst = Mat(_width, _height, _type, _stride);
    }

    unsigned char *sptr = (unsigned char *)_data;
    unsigned char *dptr = (unsigned char *)dst.data();
    int dstride = dst.stride();

    for (int i = 0, j = 0; i < _height && j < dst.height();
         i++, j++, sptr += _stride, dptr += dstride) {
        memcpy(dptr, sptr, std::min(dstride, _stride));
    }
}

// dst = src if mask != 0
int Mat::copy_to(Mat& dst, Mat& mask) const {
    TypeInfo mask_type_info;
    int status = get_type_info(mask.type(), mask_type_info);

    if (status != 0) {
        LOG_ERR("The mask type is not supported!");
        return -1;
    }

    // mask_type must be u8
    if (mask_type_info.data_type != DataType::UINT8
            || mask.width() != _width
            || mask.height() != _height) {
        LOG_ERR("The size of mask should be the same with src and the data type must be u8!");
        return -1;
    }

    int mcn = mask.channels();
    if (mcn != 1 && mcn != channels()) {
        LOG_ERR("The channels of mask should be the same with src or 1!");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(_width, _height, _type);
    }

    TypeInfo src_type_info;
    status = get_type_info(_type, src_type_info);

    if (status != 0) {
        LOG_ERR("The dst type is not supported!");
        return -1;
    }

    const unsigned char *src_data  = (const unsigned char *)_data;
    const unsigned char *mask_data = (const unsigned char *)mask.data();
    unsigned char *dst_data = (unsigned char *)dst.data();

    switch (src_type_info.data_type) {
    case DataType::UINT8:
        copy_mask<unsigned char>(src_data, _stride, mask_data, mask.stride(),
                dst_data, dst.stride(), _width, _height, mcn, channels());
        break;
    case DataType::UINT16:
        copy_mask<unsigned short>(src_data, _stride, mask_data, mask.stride(),
                dst_data, dst.stride(), _width, _height, mcn, channels());
        break;
    case DataType::SINT32:
        copy_mask<int>(src_data, _stride, mask_data, mask.stride(),
                dst_data, dst.stride(), _width, _height, mcn, channels());
        break;
    case DataType::F32:
        copy_mask<float>(src_data, _stride, mask_data, mask.stride(),
                dst_data, dst.stride(), _width, _height, mcn, channels());
        break;
    default:
        LOG_ERR("The src data type is not supported!");
        return -1;
    };

    return 0;
}

/** @brief copy the rect area to dst mat.
@param dst output array
@param rect, as specified in Rect_(T x, T y, T width, T height)
*/
int Mat::copy_to(Mat& dst, Rect& rect) const {
    TypeInfo type_info;
    int status = get_type_info(_type, type_info);

    if (status != 0) {
        LOG_ERR("Unsupport image type!");
        return -1;
    }

    int size = type_info.pixel_size;
    if (size <= 0) {
        LOG_ERR("Invalid Mat type for copy_to!");
        return -1;
    }

    Size dst_size(rect.width(), rect.height());
    if ((rect.x() + rect.width() > dst.width())
            || (rect.y() + rect.height() > dst.height())) {
        LOG_ERR("the rect area is out of the bounds in copy_to with rect!");
        return -1;
    }

    if (dst.empty()) {
        dst = Mat(rect.width(), rect.height(), _type);
    }

    int stride = rect.width() * size;
    uint8_t* src_data = reinterpret_cast<uint8_t*>(_data);
    uint8_t* dst_addr = reinterpret_cast<uint8_t*>(dst.data()) +
            dst.stride() * rect.y() + rect.x() * size;

    for (int i = 0; i < rect.height(); ++i) {
        memcpy(dst_addr, src_data, stride);
        src_data += _stride;
        dst_addr += dst.stride();
    }

    return 0;
}

G_FCV_NAMESPACE1_END()