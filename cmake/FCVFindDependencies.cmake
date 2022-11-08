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
            add_dependencies(fcv_libpng fcv_zlib)
            add_definitions(-DWITH_LIB_PNG)
        endif(WITH_LIB_PNG)
    endif(WITH_FCV_IMGCODECS)
endif(BUILD_FCV_MEDIA_IO)
