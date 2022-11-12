## img\_draw模块接口说明
### 枚举类型

enum class LineTypes {
    FILLED = -1,
    LINE_4 = 4,   //!< 4-connected line
    LINE_8 = 8,   //!< 8-connected line
    LINE_AA = 16  //!< antialiased line, not support now
};

<br>

### __函数__

#### __`circle`__

__声明文件：__modules/img\_draw/circle/interface/circle.h

```cpp
void circle(Mat& img,
        Point center,
        int radius,
        const Scalar& color,
        int thickness = 1,
        int lineType = 8,
        int shift = 0);
```

__说明：__绘制圆形。

|参数|类型|说明|
|:--|:--|:--|
|img|Mat&|源图像|
|center|Point|中心点坐标|
|radius|int|半径|
|color|const Scalar|颜色|
|thickness|int|正数，表示组成圆的线条的粗细程度。否则，表示圆是否被填充|
|line\_type|int|圆边界类型，支持LINE_4，LINE_8|
|shift|int|中心坐标和半径值中的小数位数|

<br>

#### __`line`__

__声明文件：__modules/img\_draw/line\_poly/interface/line\_poly.h

```cpp
void line(Mat& img,
        Point pt1,
        Point pt2,
        const Scalar& color,
        int thickness = 1,
        int lineType = LINE_8,
        int shift = 0);
```

__说明：__绘制直线。

|参数|类型|说明|
|:--|:--|:--|
|img|Mat&|源图像|
|pt1|Point|直线端点坐标|
|pt2|Point|直线端点坐标|
|color|const Scalar|颜色|
|thickness|int|正数，表示组成圆的线条的粗细程度。否则，表示圆是否被填充|
|line\_type|int|边界类型|
|shift|int|坐标点小数点位数|

<br>

#### __`fill_poly`__

__声明文件：__modules/img\_draw/line\_poly/interface/line\_poly.h

```cpp
void fill_poly(Mat& img,
        const Point** pts,
        const int* npts,
        int ncontours,
        const Scalar& color,
        int lineType = LINE_8,
        int shift = 0,
        Point offset = Point(0, 0));
```

__说明：__多边形绘制填充。

|参数|类型|说明|
|:--|:--|:--|
|img|Mat&|源图像|
|pts|const Point** pts|多边形曲线数组|
|npts|const int*|多边形顶点计数器数组|
|ncontours|int|曲线数|
|color|const Scalar|颜色|
|line\_type|int|边界类型|
|shift|int|坐标点小数点位数|
|offset|Point|轮廓点偏移量|

<br>

#### __`poly_lines`__

__声明文件：__modules/img\_draw/poly\_lines/interface/poly\_lines.h

```cpp
int poly_lines(Mat& img,
        const Point2l* v,
        int count,
        bool is_closed,
        const void* color,
        int thickness,
        LineTypes line_type,
        int shift);
```

__说明：__多边形绘制。

|参数|类型|说明|
|:--|:--|:--|
|img|Mat&|源图像|
|v|const Point2l*|多边形曲线数组|
|count|int|绘制多边形的个数|
|is\_closed|bool|表示多边形是否闭合, 1:闭合，0:不闭合|
|thickness|int|线条宽度|
|line\_type|int|边界类型|
|shift|int|坐标点小数点位数|

<br>