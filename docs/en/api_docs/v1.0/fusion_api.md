## fusion\_api模块接口说明
### __函数__
#### __`nv12_to_resize_to_bgr`__

**声明文件：**modules/fusion\_api/y420sp\_to\_resize\_to\_bgr/interface/y420sp\_to\_resize\_to\_bgr.h

```cpp
int nv12_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size = Size(0, 0),
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);
```

**说明：**对输入NV12格式源图像，缩放至目标图像大小，并同时转换为PACKAGE\_BGR\_U8格式。

返回值为0表示运行成功，不为0表示运行失败

| 参数 | 类型 | 说明                |
| ---- | ---- | ------------------- |
| src  | Mat& | 源图像Mat对象引用，u8数据   |
| dst  | Mat& | 输出图像Mat对象引用，u8数据 |
| size  | Size | 指定输出图像的大小，与源图像大小不一致时会进行缩放 |
| interpolation |  InterpolationType| 使用的插值算法，仅支持双线性插值和最近邻插值|

<br>

#### __`nv21_to_resize_to_bgr`__

**声明文件：**modules/fusion\_api/y420sp\_to\_resize\_to\_bgr/interface/y420sp\_to\_resize\_to\_bgr.h

```cpp
FCV_API int nv21_to_resize_to_bgr(
        Mat& src,
        Mat& dst,
        Size size = Size(0, 0),
        InterpolationType interpolation = InterpolationType::INTER_LINEAR);
```

说明：对输入NV21格式源图像，缩放至目标图像大小，并同时转换为PACKAGE\_BGR\_U8格式

返回值为0表示运行成功，不为0表示运行失败

| 参数 | 类型 | 说明                |
| ---- | ---- | ------------------- |
| src  | Mat& | 源图像Mat对象引用，u8数据   |
| dst  | Mat& | 输出图像Mat对象引用，u8数据 |
| size  | Size | 指定输出图像的大小，与源图像大小不一致时会进行缩放 |
| interpolation |  InterpolationType| 使用的插值算法，仅支持双线性插值和最近邻插值|

<br>

#### __`normalize_to_submean_to_reorder`__

**声明文件：**modules/fusion\_api/normalize\_to\_submean\_to\_reorder/interface/normalize\_to\_submean\_to\_reorder.h

```cpp
int normalize_to_submean_to_reorder(
        const Mat& src,
        const std::vector<float>& mean_params,
        const std::vector<float>& std_params,
        const std::vector<uint32_t>& channel_reorder_index,
        Mat& dst,
        bool output_package = false);
```

说明：对输入PACKAGE\_BGR\_U8格式源图像，按次序进行如下操作：
①根据输入参数设定三个通道的减法系数mean\_params与除法系数std\_params；
②遍历所有像素，每个u8通道值转换为浮点数，然后先减去对应通道的减法系数，再除以对应通道的除法系数；
③将计算结果由PACKAGE分布转换为PLANAR分布；
④根据输入的channel\_reorder\_index参数对各个通道planar顺序进行重新排序。
⑤输出package或者planar格式数据

返回值为0表示运行成功，不为0表示运行失败

|参数|类型|说明|
|:--|:--|:--|
|src|Mat&|源图像Mat，支持bgr/rgb u8/f32数据|
|mean\_params|const std::vector<float>&|三通道减法系数|
|std\_params|const std::vector<float>&|三通道除法系数|
|channel\_reorder\_index|const std::vector<uint32_t>&|通道重排序索引，注：该参数支持为空对象引用（即不重排序），非空对象引用，则实际元素取值范围仅限[0, 1, 2]|
|dst|Mat&|输出图像Mat|
|output_package|bool|输出数据是否为package格式，false:planar, true:package|

<br>

#### __`bgr_to_rgba_with_mask`__

**声明文件：**modules/fusion\_api/bgr\_to\_rgba\_with\_mask/interface/bgr\_to\_rgba\_with\_mask.h

```cpp
int bgr_to_rgba_with_mask(Mat& src, Mat& mask, Mat& dst);
```

**说明：**输入bgr三通道package格式u8图像，与单通道u8图像（Alpha通道），输出rgba四通道package格式u8图像。

返回值为0表示运行成功，不为0表示运行失败

| 参数 | 类型 | 说明        |
| ---- | ---- | ----------- |
| src  | Mat& | 源图像Mat，仅支持三通道、u8、package格式数据 |
| mask | Mat& | Alpha通道   单通道u8|
| dst  | Mat& | 输出图像Mat |

<br>

#### __`split_to_memcpy`__

**声明文件：**modules/fusion\_api/split\_to\_memcpy/interface/split\_to\_memcpy.h

```cpp
int split_to_memcpy(const Mat& src, float* dst);
```

**说明：**输入bgr三通道package格式float图像，转换为planar形式，并拷贝至指定目标地址

返回值为0表示运行成功，不为0表示运行失败。

| 参数 | 类型   | 说明                                                   |
| ---- | ------ | ------------------------------------------------------ |
| src  | Mat&   | 源图像Mat，**注：仅支持三/四通道package格式float图像** |
| dst  | float* | 输出数据地址                                           |

<br>

#### __`bgra_to_resize_to_bgr`__

**声明文件：**modules/fusion\_api/bgra\_to\_resize\_to\_bgr/interface/bgra\_to\_resize\_to\_bgr.h

```cpp
int bgra_to_resize_to_bgr(
    Mat& src,
    Mat& dst,
    const Size& dsize,
    InterpolationType interpolation = InterpolationType::INTER_LINEAR);
```

**说明：**输入bgra四通道package格式图像，转换为bgr三通道package格式，并进行缩放。

返回值为0表示运行成功，不为0表示运行失败。

|参数|类型|说明|
|:--|:--|:--|
|src|Mat&|源图像，支持u8数据|
|dst|Mat&|目标图像，支持u8数据|
|dsize|const Size&|目标图像大小|
|interpolation|InterpolationType|插值方式，仅支持双线性插值|

<br>
