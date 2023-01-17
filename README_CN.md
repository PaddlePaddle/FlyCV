<img width=100% src="https://user-images.githubusercontent.com/10016040/206948516-cc54bdac-6c48-4437-a891-bdbb5eba9595.png"/>

**[English](README_EN.md)** | **简体中文**

## 🔥 FlyCV —— 高性能计算机图像系统

![license](https://img.shields.io/hexpm/l/plug?color=%2300CD66&logo=apache&style=flat)
![GitHub star](https://img.shields.io/github/stars/paddlepaddle/flycv?color=%23FFB90F&logo=github)
![GitHub forks](https://img.shields.io/github/forks/paddlepaddle/flycv?color=pink&logo=github)
![GitHub issues](https://img.shields.io/github/issues/paddlepaddle/flycv?color=%2377BFFF&logo=github)
![main language](https://img.shields.io/github/languages/top/paddlepaddle/flycv?color=%23EEE685&logo=c%2B%2B)
![GitHub contributors](https://img.shields.io/github/contributors/paddlepaddle/flycv?color=orange&logo=github)
![Pull Request](https://img.shields.io/github/issues-pr/paddlepaddle/flycv?color=FFDAB9&logo=github)
![Commit activity](https://img.shields.io/github/commit-activity/m/paddlepaddle/flycv?color=F0FFFF&logo=github)
![Platforms](https://img.shields.io/static/v1?label=platforms&message=android|armlinux|windows|linux|ios&color=BBFFFF&logo=gnometerminal)

**FlyCV** 是一个**高性能**的计算机图像处理库。

FlyCV已在 **ARM** 架构下做了很多优化，相比其他图像处理库性能更为出色。
同时，也开展其他CPU和异构平台的性能优化工作，例如x86，RISC-V，GPU，DSP等。

除了速度更快之外，FlyCV提供了更加细粒度的编译选项控制，使得在库体积上**非常轻量**，可以**按需编译** 。
另外，在编译阶段，还提供了**自定义命名空间**的选项支持，可以方便快速地解决相同依赖库冲突的问题。

接下来通过几个例子，对比一下FlyCV和OpenCV的性能和效果：

<img width="32%" src="https://user-images.githubusercontent.com/10016040/203230955-e99ab754-b430-4a38-ba5b-cbcddda70bbc.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203231718-baf03baa-89f8-4fd8-b779-6e04c7ea0234.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203208952-26560bb6-9ab2-4243-acf1-2c4f8f5482c4.gif">

<img width="32%" src="https://user-images.githubusercontent.com/10016040/203209148-b00ed499-2695-4463-8c70-2d0fa642491a.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203209020-c7fafd21-b8e2-498e-8082-a4fa3d0241e9.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203208799-91196ae5-841d-4d27-b0cf-d1c51595c1f4.gif">

可以看到，在执行效果基本一致的情况下，FlyCV的速度明显快于OpenCV。

<br>

## :zap: 性能评测

FlyCV使用benchmark测评框架，在多款机型上进行了测评，以下是FlyCV和OpenCV在6款高中低端机型上、不同分辨率下的各算子平均加速比。

<div align=center>

<img width="33%" src="https://user-images.githubusercontent.com/10016040/210493794-59585f1d-16e8-4d73-bdd0-6ec5c3b00125.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495603-b1b7eb79-c53c-4414-bb14-6e37012e684b.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495677-a05126b2-07b4-4b91-b07f-67eed1ae0e4e.png">
  
<img width="33%" src="https://user-images.githubusercontent.com/10016040/210495748-4762c1c7-80fa-4fca-aebd-05ec0ef6e13b.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495792-acd20185-f394-4f36-8cd6-11d9e94793d9.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495843-2f1e2c36-88cb-43cc-99a5-c6bc2d658dc9.png">

</div>

<br>

## :closed_book: 快速开始

立刻体验一下吧，可通过几个非常简单的demo示例，帮助大家快速掌握集成使用的流程。

具体可以参见文档：[:book: 快速使用](https://github.com/PaddlePaddle/FlyCV/wiki/%E5%BF%AB%E9%80%9F%E5%BC%80%E5%A7%8B)

在代码库的samples目录下，提供了armlinux和android平台的调用demo。

<br>

## :notebook: API 文档

为了便于大家集成使用，FlyCV的接口设计和OpenCV非常接近，可以方便的进行迁移。

但也存在一些使用上的差别，可以通过下面的接口文档了解更多的细节：[📖 API 文档](https://github.com/PaddlePaddle/FlyCV/wiki/API%E6%96%87%E6%A1%A3)

<br>

## :notebook_with_decorative_cover: 编译文档

FlyCV支持大多数主流的操作系统，包括android、armlinux、macos（x86 & arm）、windows，以及ios。

针对不同平台都提供了相关的编译脚本，可以很方便的在本机进行源码编译。相比OpenCV，为了保证**极致轻量化**，我们还提供粗、细粒度两种功能模块编译选项，能够真正支持按需编译，在体积要求非常严格的应用中可以带来大幅优化。

根据下面的文档可以从源码进行编译：[:book: 编译手册](https://github.com/PaddlePaddle/FlyCV/wiki/%E7%BC%96%E8%AF%91%E6%89%8B%E5%86%8C)

我们也提供了不同平台预编译完成的通用库，可以直接进行下载使用：[预编译库](https://github.com/PaddlePaddle/FlyCV/releases)

**注：** 本地环境可能差别较大，如果无法运行可以尝试从源码编译。

<br>

## :pushpin: 升级日志

查看每个版本的变化：[升级日志](https://github.com/PaddlePaddle/FlyCV/releases)

<br>

## :two_men_holding_hands: 技术交流

扫描下面的二维码，加入我们的技术交流群，一起探讨计算机视觉领域的图像处理方法和优化工作。

<div>
  微信群：
<img width=20% src="https://aiwork.cdn.bcebos.com/flycv.png"/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <br/>
  如流群：
<img width=20% src="https://user-images.githubusercontent.com/10016040/204523007-427e9423-ce0e-4c6b-806c-eae7d83936e6.png"/>
</div>

<br>

:) 请点击页面右上角star收藏，实时关注项目的最新进展，感谢您助力FlyCV成长。

## :pencil2: 贡献代码

非常欢迎大家一起来贡献代码，我们提供了详细的文档，可以进行快速集成，具体细节可以查阅 [如何贡献](./CONTRIBUTING.md) ，在这里感谢每一个人的努力付出，共建一个优秀的视觉图像处理系统。

<a href="https://github.com/paddlepaddle/flycv/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=paddlepaddle/flycv" />
</a>

<br>

## :copyright: License
[Apache-2.0](./LICENSE)
