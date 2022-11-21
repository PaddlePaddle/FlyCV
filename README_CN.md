<img width=100% src="docs/assets/flycv.png"/>

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

**FlyCV** 是一个**高性能**的计算机图像处理库。我们目前在 **ARM** 架构下做了很多优化，相比其他图像处理库性能更为出色。同时也在开展其他CPU和异构平台的性能优化工作，例如x86，RISC-V，GPU，DSP等。除了速度更快之外，FlyCV提供了更加细粒度的编译选项控制，这使得我们在库体积上**非常轻量**，可以**按需编译** 。另外我们在编译阶段还提供了**自定义命名空间**的选项支持，可以方便快速地解决相同依赖库冲突的问题。

接下来通过几个例子简单体验一下FlyCV和OpenCV在性能和效果上的差异。

<img width="30%" src="https://user-images.githubusercontent.com/10016040/202954327-7308ef82-72bb-4610-9288-5d38243a8da3.gif">&nbsp;&nbsp;&nbsp;<img width="30%" src="https://user-images.githubusercontent.com/10016040/202953776-9b1efc84-7d81-4a75-9cb6-0bf01cbd5594.gif">&nbsp;&nbsp;&nbsp;<img width="30%" src="https://user-images.githubusercontent.com/10016040/202955356-7220b530-95bb-4779-ad3b-653071397a03.gif">

<img width="30%" src="https://user-images.githubusercontent.com/10016040/202991405-6434503a-c825-43e0-98ca-ebb9e77ea766.gif">&nbsp;&nbsp;&nbsp;<img width="30%" src="https://user-images.githubusercontent.com/10016040/202958543-da2b0668-dd59-452d-93bb-b2cfb65befb8.gif">&nbsp;&nbsp;&nbsp;<img width="30%" src="https://user-images.githubusercontent.com/10016040/202991013-5792ad89-1c67-4fb6-a0a1-609baa858e86.gif">

可以看到FlyCV与OpenCV的执行结果基本对齐，但速度明显快于OpenCV。

## :notebook: 文档

### API 文档

为了便于大家集成使用，我们的接口设计和OpenCV非常接近，比较方便历史的一些产品进行迁移，但也存在一些差别，可以通过下面的接口文档了解更多的细节：

[📖 API 文档](https://github.com/PaddlePaddle/FlyCV/wiki/API-Documents)

### 源码编译

我们支持大多数主流的操作系统，包括android、armlinux、macos（x86 & arm）、windows，以及ios。

根据下面的文档可以从源码进行编译：
<br>

[:book: 编译手册](https://github.com/PaddlePaddle/FlyCV/wiki/Compile-manual)

### 预编译库

当然你也可以直接下载我们已经编译好的一些库：[预编译库下载]()

## :pushpin: 升级日志

可以通过我们的升级日志查看每个版本的变化：[升级日志](docs/en/CHANGELOG.md)

## :two_men_holding_hands: 技术交流

扫描下面的二维码加入我们的技术交流群，一起探讨计算机视觉领域的图像处理方法和优化工作。

<div>
<img width=20% src="docs/assets/wechat.png"/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<img width=20% src="docs/assets/ruliu_comunication.png"/>
</div>

## :pencil2: 贡献代码

非常欢迎大家一起来贡献代码，我们提供了详细的文档以便于大家可以快速集成，具体细节可以查阅 [CONTRIBUTING.md](./CONTRIBUTING.md) ，在这里感谢每一个人的努力付出。

<a href="https://github.com/paddlepaddle/flycv/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=paddlepaddle/flycv" />
</a>

## :copyright: License
[Apache-2.0](./License)
