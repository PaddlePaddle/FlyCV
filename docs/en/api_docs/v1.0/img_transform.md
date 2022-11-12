## img\_transform模块API说明

### __枚举类型__
#### __`ColorConvertType - 颜色空间转换枚举`__
```cpp
enum class ColorConvertType {
    CVT_INVALID = 0,

    CVT_PA_BGR2GRAY,
    CVT_PA_RGB2GRAY,

    CVT_PA_BGR2PA_RGB,
    CVT_PA_RGB2PA_BGR,
    CVT_PA_BGR2PA_BGRA,
    CVT_PA_RGB2PA_RGBA,
    CVT_PA_BGR2PA_RGBA,
    CVT_PA_RGB2PA_BGRA,
    CVT_PA_BGRA2PA_BGR,
    CVT_PA_RGBA2PA_RGB,
    CVT_PA_RGBA2PA_BGR,
    CVT_PA_BGRA2PA_RGB,
    CVT_PA_BGRA2PA_RGBA,
    CVT_PA_RGBA2PA_BGRA,

    CVT_GRAY2PA_RGB,
    CVT_GRAY2PA_BGR,
    CVT_GRAY2PA_BGRA,
    CVT_GRAY2PA_RGBA,

    CVT_PA_BGR2NV12,
    CVT_PA_BGR2NV21,
    CVT_PA_RGB2NV12,
    CVT_PA_RGB2NV21,

    CVT_PA_BGRA2NV12,
    CVT_PA_BGRA2NV21,
    CVT_PA_RGBA2NV12,
    CVT_PA_RGBA2NV21,

    CVT_NV122PA_RGB,
    CVT_NV212PA_RGB,
    CVT_NV122PA_BGR,
    CVT_NV212PA_BGR,
    CVT_I4202PA_BGR,

    CVT_NV122PA_BGRA,
    CVT_NV212PA_BGRA,
    CVT_NV122PA_RGBA,
    CVT_NV212PA_RGBA,

    CVT_PA_BGR2PL_BGR,  //bgrbgrbgr... convert to bbb...ggg...rrr
    CVT_PL_BGR2PA_BGR,  //bb..gg..rr.. convert to bgrbgr..

    CVT_PA_GRAY2PA_BGR565,
    CVT_PA_BGR2PA_BGR565,
    CVT_PA_RGB2PA_BGR565,
    CVT_PA_BGRA2PA_BGR565,
    CVT_PA_RGBA2PA_BGR565,

    CVT_PA_RGBA2PA_mRGBA,
};

```
**说明：**ColorConvertType类型名中的PA指代`Package排列`，PL为`Planar排列`，左侧指源格式，右侧指目标格式。

|类型|源格式|目标格式|
|:--|:-- |:--|
|CVT\_INVALID|无效的转换类型||
|CVT\_PA\_BGR2GRAY|PACKAGE排列，三通道，BGR图像|单通道，灰度图|
|CVT\_PA\_RGB2GRAY|PACKAGE排列，三通道，RGB图像|单通道，灰度图|
|CVT\_PA\_BGR2PA\_RGB|PACKAGE排列，三通道，BGR图像|PACKAGE排列，三通道，RGB图像|
|CVT\_PA\_RGB2PA\_BGR|PACKAGE排列，三通道，RGB图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_BGR2PA\_BGRA|PACKAGE排列，三通道，BGR图像|PACAKGE排列，四通道，BGRA图像|
|CVT\_PA\_RGB2PA\_RGBA|PACKAGE排列，三通道，RGB图像|PACAKGE排列，四通道，RGBA图像|
|CVT\_PA\_BGR2PA\_RGBA|PACKAGE排列，三通道，BGR图像|PACAKGE排列，四通道，RGBA图像|
|CVT\_PA\_RGB2PA\_BGRA|PACKAGE排列，三通道，RGB图像|PACAKGE排列，四通道，BGRA图像|
|CVT\_PA\_BGRA2PA\_BGR|PACKAGE排列，四通道，BGRA图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_RGBA2PA\_RGB|PACKAGE排列，四通道，RGBA图像|PACKAGE排列，三通道，RGB图像|
|CVT\_PA\_RGBA2PA\_BGR|PACKAGE排列，四通道，RGBA图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_BGRA2PA\_RGB|PACKAGE排列，四通道，BGRA图像|PACKAGE排列，三通道，RGB图像|
|CVT\_PA\_BGRA2PA\_RGBA|PACKAGE排列，四通道，BGRA图像|PACKAGE排列，四通道，RGBA图像|
|CVT\_PA\_RGBA2PA\_BGRA|PACKAGE排列，四通道，RGBA图像|PACKAGE排列，四通道，BGRA图像|
|CVT\_GRAY2PA\_RGB|单通道，灰度图|PACKAGE排列，三通道，RGB图像|
|CVT\_GRAY2PA\_BGR|单通道，灰度图|PACKAGE排列，三通道，BGR图像|
|CVT\_GRAY2PA\_BGRA|单通道，灰度图|PACKAGE排列，四通道，BGRA图像|
|CVT\_GRAY2PA\_RGBA 单通道，灰度图|PACKAGE排列，四通道，RGBA图像|
|CVT\_PA\_BGR2NV12|PACKAGE排列，三通道，BGR图像|NV12图像|
|CVT\_PA\_BGR2NV21|PACKAGE排列，三通道，BGR图像| NV21图像|
|CVT\_PA\_RGB2NV12|PACKAGE排列，三通道，RGB图像|NV12图像|
|CVT\_PA\_RGB2NV21|PACKAGE排列，三通道，RGB图像|NV21图像|
|CVT\_PA\_BGRA2NV12|PACKAGE排列，四通道，BGRA图像|NV12图像|
|CVT\_PA\_BGRA2NV21|PACKAGE排列，四通道，BGRA图像|NV21图像|
|CVT\_PA\_RGBA2NV12|PACKAGE排列，四通道，RGBA图像|NV12图像|
|CVT\_PA\_RGBA2NV21|PACKAGE排列，四通道，RGBA图像|NV21图像|
|CVT\_NV122PA\_RGB|NV12图像|PACKAGE排列，三通道，RGB图像|
|CVT\_NV212PA\_RGB|NV21图像|PACKAGE排列，三通道，RGB图像|
|CVT\_NV122PA\_BGR|NV12图像|PACKAGE排列，三通道，BGR图像|
|CVT\_NV212PA\_BGR|NV21图像|PACKAGE排列，三通道，BGR图像|
|CVT\_I4202PA\_BGR|I420图像|PACKAGE排列，三通道，BGR图像|
|CVT\_NV122PA\_BGRA|NV12图像|PACKAGE排列，四通道，BGRA图像|
|CVT\_NV212PA\_BGRA|NV21图像|PACKAGE排列，四通道，BGRA图像|
|CVT\_NV122PA\_RGBA|NV12图像|PACKAGE排列，四通道，RGBA图像|
|CVT\_NV212PA\_RGBA|NV21图像|PACKAGE排列，四通道，RGBA图像|
|CVT\_PA\_BGR2PL\_BGR|PACKAGE排列，三通道，BGR图像|PLANAR排列，三通道，BGR图像|
|CVT\_PL\_BGR2PA\_BGR|PLANAR排列，三通道，BGR图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_GRAY2PA\_BGR565|单通道，灰度图|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_BGR2PA\_BGR565|PACKAGE排列，三通道，BGR图像|PACKAGE排列，三通道，BGR图像（存储位数不一样）|
|CVT\_PA\_RGB2PA\_BGR565|PACKAGE排列，三通道，RGB图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_BGRA2PA\_BGR565|PACKAGE排列，四通道，BGRA图像|PACKAGE排列，三通道，BGR图像|
|CVT\_PA\_RGBA2PA\_BGR565|PACKAGE排列，四通道，RGBA图像|PACKAGE排列，三通道，BGR图像|

<br>

#### __`FlipType - 翻转类型枚举`__
```cpp
enum class FlipType {
    X = 0,  // 沿X轴翻转
    Y,      // 沿Y轴翻转
    XY,     // 沿XY轴翻转
};
```

<br>
<br>

### __函数__
#### __`cvt_color`__

**声明位置：**modules/img\_transform/color\_convert/interface/color\_convert.h

```cpp
int cvt_color(const Mat& src, Mat& dst, ColorConvertType cvt_type);
```

**说明：**该接口主要用于输入图像数据连续的情况下，进行颜色空间转换，支持单通道、三通道，支持u8、f32数据。

返回值为0表示转换成功，不为0则表示转换失败。

| 参数        | 类型             | 说明                                                  |
| ----------- | ---------------- | ----------------------------------------------------- |
| src         | Mat              | 输入图像的Mat对象，**务必确保其图像数据在内存中连续** |
| dst         | Mat              | 输出图像的Mat对象                                     |
| type        | FCVImageType     | 输出图像的图像数据类型，需指定                        |

**声明位置：**modules/img\_transform/color\_convert/interface/color\_convert.h

```cpp
int cvt_color(const Mat& src_y, Mat& src_u, Mat& src_v, Mat& dst, ColorConvertType cvt_type);
```

**说明：**该接口主要用于对通道分离的YUV图像进行颜色空间转换，目前仅支持转成RGB。

返回值为0表示转换成功，不为0则表示转换失败。

| 参数        | 类型             | 说明                                                    |
| ----------- | ---------------- | ----------------------------------------------------- |
| src_y         | Mat              | YUV图像的Y通道的Mat对象                                |
| src_u         | Mat              | YUV图像的U通道的Mat对象                                |
| src_v         | Mat              | YUV图像的V通道的Mat对象                                |
| dst           | Mat              | 输出图像的Mat对象                                      |
| cvt_type      | ColorConvertType | 颜色空间转换类型，仅支持CVT_I4202PA_BGR                  |

<br>

#### __`copy_make_border`__

**声明位置：**modules/img\_transform/copy\_make\_border/interface/copy\_make\_border.h

```cpp
int copy_make_border(
        Mat& src,
        Mat& dst,
        int top,
        int bottom,
        int left,
        int right,
        FcvBorderTypes border_type,
        const Scalar& value = Scalar());
```

**说明：**边界填充方法。

返回值为0表示成功，不为0则失败。

|参数|类型|说明|
|:--|:--|:--|
|src|Mat&|源图像，仅支持PACKAGE格式图像，支持int8、int16、int32、f32以及f64数据|
|dst|Mat&|目标图像|
|top|int|图像顶部需要填充的长度|
|bottom|int|图像底部需要填充的长度|
|left|int|图像左边需要填充的长度|
|right|int|图像右边需要填充的长度|
|border\_type|FcvBorderTypes|边界填充算法，目前只支持`BORDER_CONSTANT`|
|value|const Scalar&|填充的像素值，默认为全0|

<br>

#### __`crop`__

**声明文件：**modules/img\_transform/crop/interface/crop.h

```cpp
int crop(Mat& src, Mat& dst, Rect& drect);
```

**说明：**根据源图像内的指定子矩形框进行扣图（目前仅支持PACKAGE排列的RGB颜色空间数据，支持多通道1-4）

返回值为0表示裁剪成功，不为0表示裁剪失败。

| 参数  | 类型  | 说明            |
| ----- | ----- | --------------- |
| src   | Mat&  | 源图像Mat对象 |
| dst   | Mat&  | 输出图像Mat对象 |
| drect | Rect& | 裁剪矩形框对象  |

<br>

#### __`flip`__
**声明文件：**modules/img\_transform/flip/interface/flip.h

```cpp
int flip(const Mat& src, Mat& dst, FlipType type);
```

**说明：**图像翻转方法。

|参数|类型|说明|
|:--|:--|:--|
|src|Mat&|源图像，支持单通道，三通道，支持u8/f32|
|dst|Mat&|目标图像|
|type|FlipType|翻转类型，支持X轴、Y轴、XY轴翻转|

<br>

#### __`remap`__
**声明文件：**modules/img\_transform/remap/interface/remap.h

```cpp
int remap(
        const Mat& src,
        Mat& dst,
        const Mat& map1,
        const Mat& map2,
        InterpolationType inter_type = InterpolationType::INTER_LINEAR,
        FcvBorderTypes border_method = FcvBorderTypes::BORDER_CONSTANT,
        const Scalar border_value = {0});
```

**说明：**重映射计算。

|参数|类型|说明|
|:--|:--|:--|
|src|const Mat&|源图像，支持单通道、三通道，支持u8/f32|
|dst|Mat&|目标图像|
|map1|const Mat&|表示(x,y)坐标点或是x坐标，类型为GRAY\_S16或者GRAY\_F32|
|map2|const Mat&|表示y坐标|
|inter\_type|InterpolationType|使用的插值算法，仅支持双线性插值|
|border\_method|FcvBorderTypes|使用的边界填充算法，仅支持常量填充|
|border\_value|const Scalar|边界填充值|

<br>

#### __`resize`__
**声明文件：**modules/img\_transform/resize/interface/resize.h

```cpp
int resize(
        Mat& src,
        Mat& dst,
        const Size& dsize,
        double fx = 0,
        double fy = 0,
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);
```

**说明：**图像缩放方法。

|参数|类型|说明|
|:--|:--|:--|
|src|Mat|输入图像Mat对象，支持1,3,4通道|
|dst|Mat|输出图像Mat对象|
|dsize|Size|目标宽高size对象，若已同时设置fx与fy参数，此项可忽略|
|fx|double|横向缩放系数（fx=目标宽度/输入图像宽度），若已设置dsize，此项可忽略|
|fy|double|纵向缩放系数（fy=目前高度/输入图像高度），若已设置dsize，此项可忽略|
|interpolation|InterpolationType|缩放插值计算方式，默认为双线性插值|

<br>

#### __`transpose`__

**声明文件：**modules/img\_transform/rotation/interface/rotation.h

```cpp
int transpose(Mat& src, Mat& dst);
```

**说明：**对源矩阵进行矩阵转置后输出结果至目标矩阵

返回值为0表示转置成功，不为0表示转置失败。

| 参数 | 类型 | 说明                |
| ---- | ---- | ------------------- |
| src  | Mat& | 源图像Mat对象引用，支持多通道，支持u8/f32   |
| dst  | Mat& | 输出图像Mat对象引用 |

<br>

#### __`subtract`__
__声明文件：__modules/img\_transform/subtract/interface/subtract.h

**函数原型**

```cpp
int subtract(const Mat& src, Scalar scalar, Mat& dst);
```

说明：矩阵减法，各个通道像素值减去对应的scalar对应值

|参数|类型|说明|
|:--|:--|:--|
|src|const Mat&|原图像，仅支持f32，支持多通道|
|scalar|Scalar|各通道需要减去的值|
|dst|Mat&|目标图像|

<br>

#### __`get_affine_transform`__

**声明文件：**modules/img\_transform/warp\_affine/interface/warp\_affine.h

```cpp
Mat get_affine_transform(const Point2f src[], const Point2f dst[]);
```

**说明：**该接口主要根据输入的映射源点坐标集与目标坐标集，计算仿射变换的系数矩阵并返回。

返回值Mat不为空表示计算成功，为空表示计算失败。

| 参数 | 类型             | 说明               |
| ---- | ---------------- | ------------------ |
| src  | const Point2f [] | 源像素点对象集合   |
| dst  | const Point2f [] | 目标像素点对象集合 |

<br>

#### __`get_rotation_matrix_2D`__

**声明文件：**modules/img\_transform/warp\_affine/interface/warp\_affine.h

```cpp
Mat get_rotation_matrix_2D(Point2f center, double angle, double scale);
```

**说明：**该接口主要根据输入的中心点、旋转角度、缩放比例等参数计算仿射变换的系数矩阵并返回。

返回值Mat不为空表示计算成功，为空表示计算失败。

| 参数   | 类型    | 说明           |
| ------ | ------- | -------------- |
| center | Point2f | 中心点坐标对象 |
| angle  | double  | 旋转角度值     |
| scale  | double  | 缩放系数值     |

<br>

#### __`warp_affine`__

**声明文件：**modules/img\_transform/warp\_affine/interface/warp\_affine.h

```cpp
int warp_affine(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag = InterpolationType::INTER_LINEAR,
        FcvBorderTypes border_method = FcvBorderTypes::BORDER_CONSTANT,
        const Scalar borderValue = {0});
```

**说明：**执行从源图像到目标图像的仿射变换操作。

返回值为0表示变换成功，不为0表示变换失败。

|参数|类型|说明|
|:--|:--|:--|
|src|const Mat&|源图像Mat对象常量引用，支持多通道，支持u8/f32|
|dst|Mat&|输出图像Mat对象引用|
|m|Mat&| 变换系数矩阵**（目前仅支持：宽3，高2，单通道，float或double数据类型）** |
|flag|InterpolationType|插值方式，仅支持双线性插值|
|border_method|FcvBorderTypes|边界填充方式，仅支持常量填充|
|borderValue|const Scalar|填充的边界值|

<br>

#### __`get_perspective_transform`__

__声明文件：__modules/img\_transform/warp\_perspective/interface/warp\_perspective.h

```cpp
Mat get_perspective_transform(const Point2f src[], const Point2f dst[]);
```

__说明：__计算透视变换变换矩阵。

|参数|类型|说明|
|:--|:--|:--|
|src|Point2f[]|源点|
|dst|Point2f[]|目标点|

<br>

#### __`warp_perspective`__

__声明文件：__modules/img\_transform/warp\_perspective/interface/warp\_perspective.h

```cpp
int warp_perspective(
        const Mat& src,
        Mat& dst,
        Mat& m,
        InterpolationType flag = InterpolationType::INTER_LINEAR,
        FcvBorderTypes border_method = FcvBorderTypes::BORDER_CONSTANT,
        const Scalar border_value = {0});
```

__说明：__透视变换。

|参数|类型|说明|
|:--|:--|:--|
|src|const Mat& src|源图，支持多通道，支持u8/f32|
|dst|Mat&|目标图|
|m|Mat&|透视变换矩阵|
|flag|InterpolationType|插值变换，仅支持双线性插值|
|border\_method|FcvBorderTypes|边界填充方式，仅支持常量填充|
|border\_value|const Scalar|填充的值|

<br>

#### __`add_weighted`__

__声明文件：__modules/img\_transform/add\_weighted/interface/add_weigghted.h

```cpp
int add_weighted(
        Mat& src1,
        double alpha,
        Mat& src2,
        double beta,
        double gamma,
        Mat& dst);
```

__说明：__图像融合方法。

|参数|类型|说明|
|:--|:--|:--|
|src1|Mat& src|第一张输入图像，支持三通道|
|alpha|double|第一张输入权重|
|src2|Mat&|第二张输入图像，支持三通道|
|beta|double|第二张输入权重|
|gamma|double|图一和图二融合后添加的值|
|dst|Mat&|输出图像|

<br>

#### __`extract_channel`__

__声明文件：__modules/img\_transform/extract_channel/interface/extract_channel.h

```cpp
void extract_channel(
        Mat& src,
        Mat& dst,
        int index);
```

__说明：__原图中提取某一个指定的单通道。

|参数|类型|说明|
|:--|:--|:--|
|src|Mat&|输入图像，仅支持PKG_BGR_U8格式图像|
|dst|Mat&|输出图像|
|index|int|提取通道的索引位置|

<br>
