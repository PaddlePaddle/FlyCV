option(WITH_LIB_JPEG_TURBO "Turn this ON when enable jpeg file support with libjpeg-turbo" ON)
option(WITH_LIB_PNG "Turn this ON when enable png file support with libpng" ON)

if(BUILD_FCV_MEDIA_IO)
    if(WITH_FCV_IMGCODECS)
        # Build with jpeg
        if(WITH_LIB_JPEG_TURBO)
            include(external/libjpeg-turbo)
            add_definitions(-DWITH_LIB_JPEG_TURBO)
        endif(WITH_LIB_JPEG_TURBO)

        # Build with libpng.
        if(WITH_LIB_PNG)
            include(external/libpng)
            include(external/zlib)
            add_dependencies(libpng zlib)
            add_definitions(-DWITH_LIB_PNG)
        endif(WITH_LIB_PNG)
    endif(WITH_FCV_IMGCODECS)
endif(BUILD_FCV_MEDIA_IO)

if (BUILD_FCV_IMG_TRANSFORM)
    if(WITH_FCV_WARP_AFFINE OR WITH_FCV_WARP_PERSPECTIVE)
        set(WITH_FCV_REMAP ON)
    endif()
endif()

if (BUILD_FCV_IMG_CALCULATION)
    if(WITH_FCV_FIND_HOMOGRAPHY)
        set(WITH_FCV_MATRIX_MUL ON)
    endif()
endif()

if (CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
    set(WITH_FCV_OPENCL OFF)
endif()

if (BUILD_FCV_IMG_DRAW)
    set(WITH_FCV_LINE ON)
    if (WITH_FCV_CIRCLE)
      set(WITH_FCV_POLY_LINES ON)
    endif()
endif()
