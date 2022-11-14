## core模块接口说明

### __命名空间__
#### __`默认命名空间：fcv`__

<br>

### __枚举类型__
#### __`FCVImageType - 图像类型枚举`__
__声明文件：__modules/core/base/interface/basic_types.h

```cpp
enum class FCVImageType {
    GRAY_U8 = 0,            // 单通道，unsigned char存储
    GRAY_U16,               // 单通道，unsigned short存储
    GRAY_S16,               // 单通道，signed short存储
    GRAY_S32,               // 单通道，int32存储
    GRAY_F32,               // 单通道，float32存储
    GRAY_F64,               // 单通道，double存储
    PLA_BGR_U8 = 20,        // 三通道，unsigned char存储，存储顺序：BBB...GGG...RRR...
    PLA_RGB_U8,             // 三通道，unsigned char存储，存储顺序：RRR...GGG...BBB...
    PKG_BGR_U8,             // 三通道，unsigned char存储，存储顺序：BGRBGR...
    PKG_RGB_U8,             // 三通道，unsigned char存储，存储顺序：RGBRGB...
    PLA_BGRA_U8,            // 四通道，unsigned char存储，存储顺序：BBB...GGG...RRR...AAA...
    PLA_RGBA_U8,            // 四通道，unsigned char存储，存储顺序：RRR...GGG...BBB...AAA...
    PKG_BGRA_U8,            // 四通道，unsigned char存储，存储顺序：BGRABGRA...
    PKG_RGBA_U8,            // 四通道，unsigned char存储，存储顺序：RGBARGBA...
    PLA_BGR_F32 = 40,       // 三通道，float存储，存储顺序：BBB...GGG...RRR...
    PLA_RGB_F32,            // 三通道，float存储，存储顺序：RRR...GGG...BBB...
    PKG_BGR_F32,            // 三通道，float存储，存储顺序：BGRBGR...
    PKG_RGB_F32,            // 三通道，float存储，存储顺序：RGBRGB...
    PLA_BGRA_F32,,          // 四通道，float存储，存储顺序：BBB...GGG...RRR...AAA...
    PLA_RGBA_F32,           // 四通道，float存储，存储顺序：RRR...GGG...BBB...AAA...
    PKG_BGRA_F32,           // 四通道，float存储，存储顺序：BGRABGRA...
    PKG_RGBA_F32,           // 四通道，float存储，存储顺序：RGBARGBA...
    PKG_BGR_F64,            // 三通道，double存储，存储顺序：BGRBGR...
    PKG_RGB_F64,            // 三通道，double存储，存储顺序：RGBRGB...
    PKG_BGRA_F64,           // 四通道，double存储，存储顺序：BGRABGRA...
    PKG_RGBA_F64,           // 四通道，double存储，存储顺序：RGBARGBA...
    PKG_BGR565_U8,          // 三通道，unsigned char存储，存储顺序：BGRBGR...
    PKG_RGB565_U8,          // 三通道，unsigned char存储，存储顺序：RGBRGB...
    NV12 = 60,              // YUV420SP类型，像素占比为Y:V:U=4:1:1，存储顺序：YYY...UVUV...
    NV21,                   // YVU420SP类型，像素占比为Y:U:V=4:1:1，存储顺序：YYY...VUVU...
    I420,                   // YUV420P类型，像素占比为Y:U:V=4:1:1，存储顺序：YYY...UUU...VVV...
};
```

<br>

#### __`InterpolationType - 插值类型`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
enum class InterpolationType {
    INTER_NEAREST = 0,          // 最近邻插值
    INTER_LINEAR,               // 双线性插值
    INTER_CUBIC,                // 立方插值
    INTER_AREA,                 // 区域插值
    WARP_INVERSE_MAP = 16       
};
```

<br>

#### __`BorderTypes - 边缘填充类型`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
enum class BorderTypes {
    BORDER_CONSTANT    = 0, //!< `iiiiii|abcdefgh|iiiiiii`  with some specified `i`
    BORDER_REPLICATE   = 1, //!< `aaaaaa|abcdefgh|hhhhhhh`
    BORDER_REFLECT     = 2, //!< `fedcba|abcdefgh|hgfedcb`
    BORDER_WRAP        = 3, //!< `cdefgh|abcdefgh|abcdefg`
    BORDER_REFLECT_101 = 4, //!< `gfedcb|abcdefgh|gfedcba`
    BORDER_TRANSPARENT = 5  //!< `uvwxyz|abcdefgh|ijklmno`
};
```

<br>

#### __`NormTypes - 范数类型`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
enum class NormTypes {
    NORM_INF       = 0,    // the max value of absolute value of src, for example: max =(|-1|, |2|) = 2
    NORM_L1,               // the sum of absolute value of src, for example: sum = |-1| + |2| = 3
    NORM_L2                // the Euclidean distance of src, for example: sum = sqrt((-1)^2 + (2)^2) = 5
};
```

<br>
<br>

### __类__
#### __`Size_`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
template<class T>
class Size_ {
public:
    Size_();
    Size_(int width, int height);
    Size_(const Size_& sz);

    void set_width(T width);
    void set_height(T height);

    T width() const;
    T height() const;
};

typedef Size_<int> SizeI;
typedef Size_<int64_t> SizeL;
typedef Size_<float> SizeF;
typedef Size_<double> SizeD;
typedef SizeI Size;
```
__说明：__Size_模板类用于存储图像的宽高信息，目前支持32位及64位的整型或浮点存储。

`Size_(int width, int height)`：构造函数。
`Size_(const Size_& sz)`：拷贝构造函数。
`set_width(T width)`：设置宽度。
`set_height(T width)`：设置高度。
`width()`：获取宽度。
`height()`：获取高度。

<br>

#### __`Point_`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
template<class T>
class Point_ {
public:
    Point_();
    Point_(T x, T y);
    Point_(const Point_& p) = default;

    Point_& operator=(const Point_& p) = default;
    ~Point_() = default;

    void set_x(T x);
    void set_y(T y);

    T x() const;
    T y() const;
};

typedef Point_<int> Point;
typedef Point Point2i;
typedef Point_<float> Point2f;
typedef Point_<double> Point2d;
```

__说明：__Point_模板类存储坐标点数据的模板类。

`Point_()`：默认构造函数。
`Point_(T x, T y)`：构造函数。
__x__：坐标x。
__y__：坐标y。

`Point_(const Point_& p) = default`：拷贝构造函数（使用默认生成）。
`Point_& operator=(const Point_& p) = default`：赋值运算（使用默认生成）。
`set_x(T x)`：设置x坐标的值。
`set_y(T y)`：设置y坐标的值。
`x()`：获取x坐标的值。
`y()`：获取y坐标的值。

<br>

#### __`Rect_`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
template<class T>
class Rect_ {
public:
    Rect_();
    Rect_(T x, T y, T width, T height);
    Rect_(const Rect_& rectangle) = default;
    ~Rect_() = default;

    Rect_& operator=(const Rect_& rectangle) = default;

    void set_x(T x);
    void set_y(T y);
    void set_width(T width);
    void set_height(T height);

    T x() const;
    T y() const;
    T width() const;
    T height() const;
};

typedef Rect_<int> RectI;
typedef Rect_<float> RectF;
typedef Rect_<double> RectD;
typedef RectI Rect;
```

__说明：__Rect_模板类用于存储矩形框信息，支持32位整数型、32位浮点型、64位浮点型数据，主要包含左上角点在图像中的横坐标`_x`，纵坐标`_y`，宽度值`_width`，高度值`_height`。

`Rect_()`：默认构造函数。
`Rect_(T x, T y, T width, T height)`：构造函数。
`Rect_(const Rect_& rectangle) = default`：拷贝构造函数（使用默认生成）。
`Rect_& operator=(const Rect_& rectangle) = default`：赋值运算（使用默认生成）。
`set_x(T x)`：设置左上角顶点x坐标的值。
`set_y(T y)`：设置左上角顶点y坐标的值。
`set_width(T width)`：设置矩形的宽度。
`set_height(T height)`：设置矩形的高度。
`T x() const`：获取矩形左上角顶点x的坐标值。
`T y() const`：获取矩形左上角顶点y的坐标值。
`T width() const`：获取矩形的宽度。
`T height() const`：获取矩形的高度。

<br>

#### __`RotatedRect`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
class RotatedRect {
public:
    RotatedRect();
    RotatedRect(const float& center_x, const float& center_y,
            const float& width, const float& height, const float& angle);
    RotatedRect(const Point2f& center, const Size2f& size, const float& angle);

    RotatedRect(const RotatedRect&) = default;
    ~RotatedRect() = default;

    void set_center(const Point2f& center);
    void set_center_x(const float& center_x);
    void set_center_y(const float& center_y);
    void set_size(const Size2f& size);
    void set_width(const float& width);
    void set_height(const float& height);
    void set_angle(const float& angle);
    void points(std::vector<Point2f>& pts);

    Point2f center() const;
    float center_x() const;
    float center_y() const;
    Size2f size() const;
    float width() const;
    float height() const;
    float angle() const;
};
```

__说明：__RotatedRect类用于存储带有角度信息的矩形框数据，`只支持float类型数据`。

`RotatedRect()`：默认构造函数。
`RotatedRect(const float& center_x, const float& center_y,
        const float& width, const float& height, const float& angle)`：多参构造函数。
__center\_x__：旋转矩形的中心点x坐标值。
__center\_y__：旋转矩形的中心点y坐标值。
__width__：旋转矩形的宽度。
__height__：旋转矩形的高度。
__angle__：旋转矩形的角度（角度制），与x轴平行的方向为角度为0，逆时针旋转角度为负，顺时针旋转角度为正。

`RotatedRect(const Point2f& center, const Size2f& size, const float& angle)`：多参构造函数。
`RotatedRect(const RotatedRect&) = default`：拷贝构造函数（使用默认生成）。
`~RotatedRect() = default`：析构函数（使用默认生成）。
`void set_center(const Point2f& center)`：设置旋转矩形中心点坐标。
`void set_center_x(const float& center_x)`：设置旋转矩形中心点x坐标。
`void set_center_y(const float& center_y)`：设置旋转矩形中心点y坐标。
`void set_size(const Size2f& size)`：设置旋转矩形宽高。
`void set_width(const float& width)`：设置旋转矩形宽度。
`void set_height(const float& height)`：设置旋转矩形高度。
`void set_angle(const float& angle)`：设置旋转矩形角度。
`void points(std::vector<Point2f>& pts)`：返回旋转矩形的四个顶点坐标。
`Point2f center() const`：获取旋转矩形中心点坐标。
`float center_x() const`：获取旋转矩形中心点x坐标值。
`float center_y() const`：获取旋转矩形中心点y坐标值。
`Size2f size() const`：获取旋转矩形宽高。
`float width() const`：获取旋转矩形宽度。
`float height() const`：获取旋转矩形高度。
`float angle() const`：获取旋转矩形角度。

<br>

#### __`Scalar_`__
__声明文件：__modules/core/base/interface/basic_types.h
```cpp
template<class T>
class Scalar_ {
public:
    //! default constructor
    Scalar_();
    Scalar_(T v0, T v1, T v2 = 0, T v3 = 0);
    Scalar_(T v0);

    Scalar_(const Scalar_& s);

    Scalar_& operator= (const Scalar_& s);

    T& operator[] (int index);
    const T& operator[] (int index) const;

    int set_val(int index, T val);
    const T* val() const;

    //! returns a scalar with all elements set to v0
    static Scalar_<T> all(T v0);
};

typedef Scalar_<double> Scalar;
```

__说明：__存储标量数据的模板类。存储数据上限为4，通常对应图像数据的不同channel参与计算。

`Scalar_()`：默认构造函数。
`Scalar_(T v0, T v1, T v2 = 0, T v3 = 0)`：多参构造函数
`Scalar_(T v0)`：单参构造函数。
`Scalar_(const Scalar_& s)`：拷贝构造函数。
`Scalar_& operator= (const Scalar_& s)`：重载赋值运算符。
`T& operator[] (int index)`：重载[]运算符，支持下标取值。
`const T& operator[] (int index) const`：重载[]运算符，支持下标取值（针对常量）。
`int set_val(int index, T val)`：设置索引位置的值。
`const T* val() const`：返回标量数据的指针。
`static Scalar_<T> all(T v0)`：用给定的值初始化标量数组。

<br>

#### __`Mat`__

__声明文件：__modules/core/mat/interface/mat.h
```cpp
class Mat {
public:
    Mat();
    Mat(int width, int height, FCVImageType type, void* data, int stride = 0);
    Mat(int width, int height, FCVImageType type, std::vector<uint64_t*>& phy_addrs,
            std::vector<uint64_t*>& vir_addrs, int stride = 0);
    Mat(int width, int height, FCVImageType type, int stride = 0,
            PlatformType platform = PlatformType::CPU);
    Mat(Size size, FCVImageType type, int stride = 0, PlatformType platform = PlatformType::CPU);
    ~Mat();

    int width() const;
    int height() const;
    Size2i size() const;
    int channels() const;
    int stride() const;
    FCVImageType type() const;
    int type_byte_size() const;
    uint64_t total_byte_size() const;
    bool empty() const;

    void* data() const;
    uint64_t* phy_data() const;
    uint64_t* vir_data() const;

    Mat clone() const;

    template<typename T>
    T& at(int x, int y, int c = 0);

    template<typename T>
    const T& at(int x, int y, int c = 0) const;

    template<typename T>
    T* ptr(int x, int y, int c = 0);

    template<typename T>
    const T* ptr(int x, int y, int c = 0) const;

    void convert_to(Mat& dst, FCVImageType dst_type, double alpha = 1.0, double beta = 0.0) const;
    void copy_to(Mat& dst) const;
    void copy_to(Mat& dst, Mat& mask) const;
    void copy_to(Mat& dst, Rect& rect) const;
    double dot(Mat& m) const;
    bool invert(Mat& dst) const;
};
```

__说明：__图像数据类。采用引用计数，内部分配的数据空间无需手动释放；外部传入的数据需要进行外部自行释放。

<br>

`Mat()`：默认构造函数。
`Mat(int width, int height, FCVImageType type, void* data, int stride = 0)`：多参构造函数。
__width__：图像宽度。
__height__：图像高度。
__type__：图像类型。
__data__：图像数据指针。
__stride__：对齐宽度（字节数）。

`Mat(int width, int height, FCVImageType type, std::vector<uint64_t*>& phy_addrs),<br>std::vector<uint64_t*>& vir_addrs, int stride = 0)`：多参构造函数。
__phy\_addrs__：物理地址指针数组。
__vir\_addrs__：虚拟地址指针数组。

`Mat(int width, int height, FCVImageType type, int stride = 0, PlatformType platform = PlatformType::CPU);`：多参构造函数。
__platform__：使用的计算核心。

`Mat(Size size, FCVImageType Type, int stride = 0, PlatformType platform = PlatformType::CPU)`：多参构造函数。
__size__：图像宽高。

`~Mat()`：析构函数。
`int width() const`：返回图像宽度。
`int height() const`：返回图像高度。
`Size2i size() const`：返回图像宽高。
`int channels() const`：返回图像通道数。
`FCVImageType type() const`：返回图像类型。
`bool empty() const`：返回图像是否为空。
`void* data() const`：返回图像数据地址。
`uint64_t* phy_data() const`：返回图像物理数据地址。
`uint64_t* vir_data() const`：返回图像虚拟数据地址。
`Mat clone() const`：深度克隆图像数据。
`T& at(int x, int y, int c = 0)`：返回指定坐标像素值的引用。
__x__：横坐标。
__y__：纵坐标。
__c__：通道。

`const T& at(int x, int y, int c = 0) const`：返回指定坐标像素的常量引用。
`T* ptr(int x, int y, int c = 0)`：返回指定坐标的指针。
`const T* ptr(int x, int y, int c = 0) const`：返回指定坐标的数据常量指针。
`void convert_to(Mat& dst, FCVImageType dst_type, double scale = 1.0, double shift = 0.0) const`：缩放、平移，并转化为指定数据类型。
__dst__：目的矩阵。
__dst\_type__：指定目的矩阵的类型。
__scale__：缩放系数。
__shift__：平移值。

`void copy_to(Mat& dst) const`：数据拷贝到指定目标。
`void copy_to(Mat& dst, Mat& mask) const`：数据依据遮罩拷贝到指定目标。
`void copy_to(Mat& dst, Rect& rect) const`：根据给定的矩形框将数据拷贝到指定目标。
`double dot(Mat& m) const`：矩阵点乘。
`bool invert(Mat& dst) const`：矩阵求逆。

<br>
<br>

### __函数__
#### __`fcv_round`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h
__函数原型__
```cpp
inline int fcv_round(doubel value);
```

__说明：__浮点类型数据四舍五入转为整型。

<br>

#### __`fcv_floor`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
inline int fcv_floor(double value);
```

说明：双精度浮点型数据类型向下取整（例如：-0.3转为-1，10.4转为10）

<br>

#### __`fcv_ceil`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
inline int fcv_ceil(double value)；
```

说明：双精度浮点型数据类型向上取整（例如：-0.3转为0，10.4转为11）

<br>

#### __`fcv_clamp`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
template<class T, class D>
static inline constexpr T fcv_clamp(const T& val, const D& min, const D& max);
```

说明：将随机值固定在限定区间，低于下限则返回下限，超出上限则返回上限。

|参数|类型|说明|
|:--|:--|:--|
|val|const T&|给定的值|
|min|const D&|区间下限|
|max|const D&|区间上限|

<br>

#### __`fcv_cast_s16`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
template<class T>
static inline constexpr short fcv_cast_s16(const T& val);
```

说明：将给定的值限制在signed short可表示的范围之内。

<br>

#### __`fcv_cast_u16`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
template<class T>
static inline constexpr unsigned short fcv_cast_u16(const T& val);
```

说明：将给定的值限制在unsigned short可表示的范围之内。

<br>

#### __`fcv_cast_s8`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
template<class T>
static inline constexpr signed char fcv_cast_s8(const T& val);
```

说明：将给定的值限制在signed char可表示的范围之内。

<br>

#### __`fcv_cast_u8`__
__声明文件：__modules/core/basic_math/interface/basic\_math.h

**函数原型**

```cpp
template<class T>
static inline constexpr unsigned char fcv_cast_u8(const T& val);
```

说明：将给定的值限制在unsigned char可表示的范围之内。

<br>

#### __`parallel_run`__
__声明文件：__modules/core/parallel/interface/parallel.h

**函数原型**

```cpp
int parallel_run(
        const Range& range,
        const ParallelTask& task,
        int nstripes = -1);
```

说明：多线程并行运行接口函数。

<br>

#### __`set_thread_num`__
__声明文件：__modules/core/parallel/interface/parallel.h

**函数原型**

```cpp
void set_thread_num(int num);
```

说明：设置线程池线程数。

<br>

#### __`get_thread_num`__
__声明文件：__modules/core/parallel/interface/parallel.h

**函数原型**

```cpp
int get_thread_num();
```

说明：获取线程池线程数。

<br>
