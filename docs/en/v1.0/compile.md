## FlyCV Compilation Instruction Manual

For easy compilation, we provide docker images for different platforms, you can also compile it on your local machine without docker.


Go to install docker（optional）: [Docker Installation](https://www.docker.com/)

### Contents

- <a href="#compile-options">:bulb: Compile Options Support</a>
  - <a href="#function-compile-options">Function compile options</a>
  - <a href="#cpu-compile-options">CPU optimization compile options</a>
  - <a href="#other-compile-options">Other compile options</a>

- <a href="#android">:bulb: Compile Android Libraries</a>

- <a href="#armlinux">:bulb: Compile Armlinux Libraries</a>

- <a href="#macos">:bulb: Compile MacOS (x86 & arm) Libraries</a>

- <a href="#windows">:bulb: Compile Windows Libraries</a>

- <a href="#linux">:bulb: Compile Linux Libraries</a>

<br>

### <span id="compile-options">Compile Options Support</span>
#### <span id="function-compile-options">Function compile options</span>
|compile options|default value|description|
|:--|:--:|:--|
|:file_folder: BUILD_FCV_CORE|ON|core functions cannot be turned off|
|:file_folder: BUILD_FCV_IMG_TRANSFORM|ON|image transform functions|
|:page_with_curl: WITH_FCV_COLOR_CONVERT|ON|supports frequently used color space conversion functions|
|:page_with_curl: WITH_FCV_RESIZE|ON|image resize|
|:page_with_curl: WITH_FCV_WARP_AFFINE|ON|image affine transformation|
|:page_with_curl: WITH_FCV_WARP_PERSPECTIVE|ON|image perspective transformation|
|:page_with_curl: WITH_FCV_ROTATION|ON|image rotation|
|:page_with_curl: WITH_FCV_FLIP|ON|image flipped along the axes|
|:page_with_curl: WITH_FCV_SUBTRACT|ON|image subtraction|
|:page_with_curl: WITH_FCV_COPY_MAKE_BORDER|ON|image border padding|
|:page_with_curl: WITH_FCV_CROP|ON|crop image|
|:page_with_curl: WITH_FCV_REMAP|ON|image remap|
|:page_with_curl: WITH_FCV_ADD_WEIGHTED|ON|fusion of two images|
|:page_with_curl: WITH_FCV_EXTRACT_CHANNEL|ON|extract the data of the specified channel|
|:file_folder: BUILD_FCV_IMG_CALCULATION|ON|calculate from image|
|:page_with_curl: WITH_FCV_MATRIX_MUL|ON|matrix multiplication|
|:page_with_curl: WITH_FCV_NORM|ON|calculate image norm|
|:page_with_curl: WITH_FCV_MEAN|ON|calculate image mean|
|:page_with_curl: WITH_FCV_MIN_AREA_RECT|ON|generate the min bounding rectangle|
|:page_with_curl: WITH_FCV_MIN_MAX_LOC|ON|get the minimum and maximum coordinate points|
|:page_with_curl: WITH_FCV_CONNECTED_COMPONENTS|ON|calculate the connected domain|
|:page_with_curl: WITH_FCV_FIND_HOMOGRAPHY|ON|calculate the optimal single-map transformation|
|:file_folder: BUILD_FCV_FUSION_API|ON|fusion of image processing|
|:page_with_curl: WITH_FCV_BGR_TO_RGBA_WITH_MASK|ON|convert bgr to bgra with mask|
|:page_with_curl: WITH_FCV_NORMALIZE_TO_SUBMEAN_TO_REORDER|ON|image normalize & submean & reorder|
|:page_with_curl: WITH_FCV_SPLIT_TO_MEMCPY|ON|split image and copy to the specific memory|
|:page_with_curl: WITH_FCV_Y420SP_TO_RESIZE_TO_BGR|ON|convert yuv to bgr with resize|
|:page_with_curl: WITH_FCV_BGRA_TO_RESIZE_TO_BGR|ON|convert bgra to bgr with resize|
|:file_folder: BUILD_FCV_MATH_OPERATOR|ON|base math operators|
|:page_with_curl: WITH_FCV_VECTOR_OPERATOR|ON|vector operators|
|:file_folder: BUILD_FCV_MEDIA_IO|ON|media io|
|:page_with_curl: WITH_FCV_IMGCODECS|ON|image codesc|
|:page_with_curl: WITH_LIB_JPEG_TURBO|ON|support jpg codesc|
|:page_with_curl: WITH_LIB_PNG|ON|support png codesc|
|:file_folder: BUILD_FCV_IMG_DRAW|ON|drawing on image|
|:page_with_curl: WITH_FCV_FILL_POLY|ON|polygon drawing and filling|
|:page_with_curl: WITH_FCV_POLY_LINES|ON|polygon drawing|
|:page_with_curl: WITH_FCV_CIRCLE|ON|circle drawing|
|:page_with_curl: WITH_FCV_LINE|ON|line drawing|

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
