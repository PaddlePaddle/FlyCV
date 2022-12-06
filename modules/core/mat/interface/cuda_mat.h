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

#pragma once

#include <cuda_runtime.h>
#include <stdint.h>

#include <vector>

#include "falconcv_namespace.h"
#include "modules/core/allocator/interface/base_allocator.h"
#include "modules/core/base/interface/basic_types.h"
#include "modules/core/base/interface/log.h"
#include "modules/core/base/interface/macro_ns.h"
#include "modules/core/basic_math/interface/basic_math.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

#define CHECK(call)                                                         \
    do {                                                                    \
        const cudaError_t error_code = call;                                \
        if (error_code != cudaSuccess) {                                    \
            printf("CUDA Error:\n");                                        \
            printf("    File:       %s\n", __FILE__);                       \
            printf("    Line:       %d\n", __LINE__);                       \
            printf("    Error code: %d\n", error_code);                     \
            printf("    Error text: %s\n", cudaGetErrorString(error_code)); \
            exit(1);                                                        \
        }                                                                   \
    } while (0)

class FCV_API CudaMat {
public:
    //! default allocator
    static BaseAllocator* defaultAllocator();
    static void setDefaultAllocator(BaseAllocator* allocator);

    //! default constructor
    explicit CudaMat(BaseAllocator* allocator);

    //! constructs CudaMat of the specified size and type
    CudaMat(int width, int height, FCVImageType type, BaseAllocator* allocator);
    CudaMat(Size size, FCVImageType type, BaseAllocator* allocator);

    //! constructs CudaMat and fills it with the specified value _s
    CudaMat(int width, int height, int type, Scalar s,
            BaseAllocator* allocator);
    CudaMat(Size size, int type, Scalar s, BaseAllocator* allocator);

    //! copy constructor
    CudaMat(const CudaMat& m);

    //! destructor - calls release()
    ~CudaMat();

    //! assignment operators
    CudaMat& operator=(const CudaMat& m);

    //! allocates new CudaMat data unless the CudaMat already has specified size
    //! and type
    void create(int width, int height, int type);
    void create(Size size, int type);

    //! decreases reference counter, deallocate the data when reference counter
    //! reaches 0
    void release();

    //! swaps with other smart pointer
    void swap(CudaMat& CudaMat);

    /** @brief Performs data upload to CudaMat (Blocking call)

    This function copies data from host memory to device memory. As being a
    blocking call, it is guaranteed that the copy operation is finished when
    this function returns.
    */
    void upload(CudaMat arr);

    /** @brief Performs data upload to CudaMat (Non-Blocking call)

    This function copies data from host memory to device memory. As being a
    non-blocking call, this function may return even if the copy operation is
    not finished.

    The copy operation may be overlapped with operations in other non-default
    streams if \p stream is not the default stream and \p dst is HostMem
    allocated with HostMem::PAGE_LOCKED option.
    */
    void upload(CudaMat arr, Stream& stream);

    /** @brief Performs data download from CudaMat (Blocking call)

    This function copies data from device memory to host memory. As being a
    blocking call, it is guaranteed that the copy operation is finished when
    this function returns.
    */
    void download(CudaMat dst) const;

    /** @brief Performs data download from CudaMat (Non-Blocking call)

    This function copies data from device memory to host memory. As being a
    non-blocking call, this function may return even if the copy operation is
    not finished.

    The copy operation may be overlapped with operations in other non-default
    streams if \p stream is not the default stream and \p dst is HostMem
    allocated with HostMem::PAGE_LOCKED option.
    */
    void download(CudaMat dst, Stream& stream) const;

    //! returns deep copy of the CudaMat, i.e. the data is copied
    CudaMat clone() const;

    //! copies the CudaMat content to device memory (Blocking call)
    void copy_to(CudaMat& dst) const;

    //! copies the CudaMat content to device memory (Non-Blocking call)
    void copy_to(CudaMat& dst, Stream& stream) const;

    //! sets some of the CudaMat elements to s (Blocking call)
    CudaMat& set_to(Scalar s);

    //! sets some of the CudaMat elements to s (Non-Blocking call)
    CudaMat& set_to(Scalar s, Stream& stream);

    //! converts CudaMat to another datatype (Blocking call)
    void convert_to(CudaMat& dst, FCVImageType rtype) const;

    //! converts CudaMat to another datatype (Non-Blocking call)
    void convert_to(CudaMat& dst, FCVImageType rtype, Stream& stream) const;

    //! converts CudaMat to another datatype with scaling (Blocking call)
    void convert_to(CudaMat& dst, FCVImageType rtype, double alpha,
                   double beta = 0.0) const;

    //! converts CudaMat to another datatype with scaling (Non-Blocking call)
    void convert_to(CudaMat& dst, FCVImageType rtype, double alpha, double beta,
                   Stream& stream) const;

    //! returns pointer to y-th row
    uint8_t* ptr(int y = 0);
    const uint8_t* ptr(int y = 0) const;

    //! template version of the above method
    template <typename _Tp>
    _Tp* ptr(int y = 0);
    template <typename _Tp>
    const _Tp* ptr(int y = 0) const;

    //! extracts a rectangular sub-CudaMat (this is a generalized form of row,
    //! rowRange etc.)
    CudaMat operator()(Rect roi) const;

    //! locates CudaMat header within a parent CudaMat
    void locate_roi(Size& wholeSize, Point& ofs) const;

    //! moves/resizes the current CudaMat ROI inside the parent CudaMat
    CudaMat& adjust_roi(int dtop, int dbottom, int dleft, int dright);

    //! returns true i the CudaMat data is continuous
    //! (i.e. when there are no gaps between successive rows)
    bool is_continuous() const;

    //! returns element size in bytes
    size_t elem_size() const;

    //! returns the size of element channel in bytes
    size_t elem_size1() const;

    //! returns element type
    FCVImageType type() const;

    //! returns element type
    int depth() const;

    //! returns number of channels
    int channels() const;

    //! returns step/elemSize1()
    size_t step1() const;

    //! returns CudaMat size : width == number of columns, height == number of
    //! rows
    Size size() const;

    //! returns true if CudaMat data is NULL
    bool empty() const;

    // returns pointer to cuda memory
    void* cuda_ptr() const;

    //! internal use method: updates the continuity flag
    void update_continuity_flag();

    /*! includes several bit-fields:
    - the magic signature
    - continuity flag
    - depth
    - number of channels
    */
    int flags;

    //! the number of width and height
    int width, height;

    //! a distance between successive rows in bytes; includes the gap if any
    size_t step;

    //! pointer to the data
    uint8_t* data;

    //! helper fields used in locateROI and adjustROI
    uint8_t* datastart;
    const uint8_t* dataend;

    //! allocator
    std::unique_ptr<BaseAllocator> allocator;

    //! GPU memory info
    CUDAMemoryType memory_type;
};

class FCV_API Allocator {
public:
    virtual ~Allocator() {}

    // allocator must fill data, step and refcount fields
    virtual bool allocate(CudaMat* CudaMat, int width, int height,
                          size_t elemSize) = 0;
    virtual void free(CudaMat* CudaMat) = 0;
};

class FCV_API Stream {
public:
    Stream() : stream(0) {
        cudaStreamCreate(&stream);
        CHECK(cudaGetLastError());
    }

    ~Stream() {
        if (stream) {
            cudaStreamDestroy(stream);
        }
    }

    static Stream& Null() {
        cudaStream_t stream = NULL;
        return (std::make_shared<Stream>(stream));
    }

public:
    cudaStream_t stream;

private:
    Stream(cudaStream_t& st) : stream(st) {}
};

G_FCV_NAMESPACE1_END()
