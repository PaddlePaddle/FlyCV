## math\_operator模块接口说明

### 函数

#### __`get_l2`__

__声明文件：__modules/math\_operator/vector\_operator/interface/vector\_operator.h

__函数原型__

```cpp
float get_l2(int dimension, float* vector);
```

__说明：__计算给定数组的平方和。

|参数|类型|说明|
|:--|:--|:--|
|dimension|int|向量长度|
|vector|float*|数据指针|

<br>

#### __`dot_vectors`__

__声明文件：__modules/math\_operator/vector\_operator/interface/vector\_operator.h

__函数原型__

```cpp
void dot_vectors(int dimension, float* v0, float* v1, float* dot);
```

__说明：__向量点乘。

|参数|类型|说明|
|:--|:--|:--|
|dimension|int|向量长度|
|v0|float*|输入数据指针|
|v1|float*|输入数据指针|
|dot|float*|输出数据指针|

<br>
