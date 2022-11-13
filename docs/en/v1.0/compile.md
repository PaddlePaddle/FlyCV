## FlyCV Compilation Instruction Manual

For easy compilation, we provide docker images for different platforms, you can also compile it on your local machine without docker.


Go to install docker（optional）: [Docker Installation](https://www.docker.com/)

### Contents

- <a href="#compile-options">:bulb: Compile Options Support</a>
  - <a href="#function-compile-options">Function compile options</a>
  - <a href="#cpu-compile-options">CPU optimization compile options</a>
  - <a href="#other-compile-options">Other compile options</a>

- <a href="#android">:bookmark: Compile Android Libraries</a>

- <a href="#armlinux">:bookmark: Compile Armlinux Libraries</a>

- <a href="#macos">:bookmark: Compile MacOS (x86 & arm) Libraries</a>

- <a href="#windows">:bookmark: Compile Windows Libraries</a>

- <a href="#linux">:bookmark: Compile Linux Libraries</a>

<br>

### <span id="compile-options">Compile Options Support</span>
#### <span id="function-compile-options">Function compile options</span>
|compile options|default value|description|
|:--|:--:|:--|
|:file_folder: BUILD_FCV_CORE|ON|Core functions cannot be turned off|
|:file_folder: BUILD_FCV_IMG_TRANSFORM|ON|Image transform functions|
|:page_with_curl: WITH_FCV_COLOR_CONVERT|ON|Supports frequently used color space conversion functions|
|:page_with_curl: WITH_FCV_RESIZE|ON|Image resize|
|:page_with_curl: WITH_FCV_WARP_AFFINE|ON|Image affine transformation|
|:page_with_curl: WITH_FCV_WARP_PERSPECTIVE|ON|Image perspective transformation|
|:page_with_curl: WITH_FCV_ROTATION|ON|Image rotation|
|:page_with_curl: WITH_FCV_FLIP|ON|Image flipped along the axes|
|:page_with_curl: WITH_FCV_SUBTRACT|ON|Image subtraction|
|:page_with_curl: WITH_FCV_COPY_MAKE_BORDER|ON|Image border padding|
|:page_with_curl: WITH_FCV_CROP|ON|Crop image|
|:page_with_curl: WITH_FCV_REMAP|ON|Image remap|
|:page_with_curl: WITH_FCV_ADD_WEIGHTED|ON|Fusion of two images|
|:page_with_curl: WITH_FCV_EXTRACT_CHANNEL|ON|Extract the data of the specified channel|
|:file_folder: BUILD_FCV_IMG_CALCULATION|ON|Calculate from image|
|:page_with_curl: WITH_FCV_MATRIX_MUL|ON|Matrix multiplication|
|:page_with_curl: WITH_FCV_NORM|ON|Calculate image norm|
|:page_with_curl: WITH_FCV_MEAN|ON|Calculate image mean|
|:page_with_curl: WITH_FCV_MIN_AREA_RECT|ON||
|:page_with_curl: WITH_FCV_MIN_MAX_LOC|ON||
|:page_with_curl: WITH_FCV_CONNECTED_COMPONENTS|ON||
|:page_with_curl: WITH_FCV_FIND_HOMOGRAPHY|ON||
|:file_folder: BUILD_FCV_FUSION_API|ON||
|:page_with_curl: WITH_FCV_BGR_TO_RGBA_WITH_MASK|ON||
|:page_with_curl: WITH_FCV_NORMALIZE_TO_SUBMEAN_TO_REORDER|ON||
|:page_with_curl: WITH_FCV_SPLIT_TO_MEMCPY|ON||
|:page_with_curl: WITH_FCV_Y420SP_TO_RESIZE_TO_BGR|ON||
|:page_with_curl: WITH_FCV_BGRA_TO_RESIZE_TO_BGR|ON||
|:file_folder: BUILD_FCV_MATH_OPERATOR|ON||
|:page_with_curl: WITH_FCV_VECTOR_OPERATOR|ON||
|:file_folder: BUILD_FCV_MEDIA_IO|ON||
|:page_with_curl: WITH_FCV_IMGCODECS|ON||
|:page_with_curl: WITH_LIB_JPEG_TURBO|ON||
|:page_with_curl: WITH_LIB_PNG|ON||
|:file_folder: BUILD_FCV_IMG_DRAW|ON||
|:page_with_curl: WITH_FCV_FILL_POLY|ON||
|:page_with_curl: WITH_FCV_POLY_LINES|ON||
|:page_with_curl: WITH_FCV_CIRCLE|ON||
|:page_with_curl: WITH_FCV_LINE|ON||

#### <span id="cpu-compile-options">CPU compile options</span>
|compile options|default value|description|
|:--|:--|:--|
|ENABLE_SSE|||
|ENABLE_AVX|||
|ENABLE_NEON|||
|ANDROID_ARM_NEON|||
|ANDROID_ARM_SVE2|||

#### <span id="other-compile-options">Other compile options</span>
|compile options|default value|description|
|:--|:--|:--|
|BUILD_TEST|||
|BUILD_BENCHMARK|||
|BUILD_SAMPLES|||

<hr>

### <span id="android">For Android Libraries<span>

#### compile in docker container
  
```
docker pull flycv/x86_64:android_17c_23c
docker attach
source /etc/ndk/r17c.env
./scripts/armlinux/build.sh
```

#### compile on local machine
  
  
```
export ANDROID_NDK=/Users/taotianran/Library/Android/sdk/ndk/17.2.4988734/
./scripts/android/build.sh
```

<hr>

### <span id="armlinux">For Armlinux Libraries</span>
  
```
./scripts/armlinux/build.sh
```

<hr>


### <span id="macos">For Macos Libraries</span>
  
```
./scripts/macos/build_x86.sh
```

<hr>

### <span id="windows">For Windows Libraries</span>
  
```
./scripts/windows/build.sh
```

<hr>

### <span id="linux">For Linux Libraries</span>
  
```
./scripts/linux/build.sh
```
