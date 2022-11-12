## imgcodes模块API说明
### __函数__
#### __`imread`__

__声明文件：__modules/imgcodecs/interface/imgcodecs.h

```cpp
Mat imread(const std::string& file_name, int flags = 0);
```

**说明：**读取指定路径的图像文件，**文件格式及后缀仅支持jpg或jpeg**，返回Mat对象（**仅支持PKG_BGR_U8或GRAY_U8**）

返回对象非空表示运行成功，为空则表示运行失败

| 参数      | 类型               | 说明             |
| --------- | ------------------ | ---------------- |
| file_name | const std::string& | 待读取文件路径   |
| flags     | int                | 保留字段，可忽略 |

#### __`imwrite`__

__声明文件：__modules/imgcodecs/interface/imgcodecs.h

```cpp
bool imwrite(const std::string& file_name, Mat& img, int quality = 95);
```

**说明：**将指定Mat对象写为图像文件，**文件格式及后缀仅支持jpg或jpeg**，输入Mat对象**仅支持PKG_BGR_U8或GRAY_U8**

返回true表示运行成功，false则表示运行失败

| 参数      | 类型               | 说明                        |
| --------- | ------------------ | --------------------------- |
| file_name | const std::string& | 待写入文件名                |
| img       | Mat&               | 输出图像Mat对象             |
| quality   | int                | 图像压缩质量，值范围[0~100] |

#### __`imdecode`__

__声明文件：__modules/imgcodecs/interface/imgcodecs.h

```cpp
Mat imdecode(const uint8_t* buf, const size_t size, int flags = 0);
```

**说明：**对输入标准JPEG或PNG数据，进行解码，返回Mat对象（**仅支持PKG_BGR_U8或GRAY_U8**）

返回对象非空表示运行成功，为空则表示运行失败

| 参数  | 类型           | 说明             |
| ----- | -------------- | ---------------- |
| buf   | const uint8_t* | 原始数据地址     |
| size  | const size_t   | 原始数据长度     |
| flags | int            | 保留字段，可忽略 |

#### __`imencode`__

__声明文件：__modules/imgcodecs/interface/imgcodecs.h

```cpp
bool imencode(const std::string& ext, const Mat& img, std::vector<uint8_t>& buf, int quality = 95);
```

**说明：**将指定Mat对象编码为标准JPEG或PNG格式数据，通过ext后缀进行格式指定，输入Mat对象**仅支持PKG_BGR_U8或GRAY_U8**

返回true表示运行成功，false则表示运行失败

| 参数    | 类型                  | 说明                                                         |
| ------- | --------------------- | ------------------------------------------------------------ |
| ext     | const std::string&    | 编码格式后缀，`.jpg`或`.jpeg`表明为JPEG编码，`.png`表明为PNG编码 |
| img     | Mat&                  | 输出图像Mat对象                                              |
| buf     | std::vector<uint8_t>& | 结果数据                                                     |
| quality | int                   | 图像压缩质量，值范围[0~100]（注：仅影响JPEG格式编码）        |
