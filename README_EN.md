<img width=100% src="https://user-images.githubusercontent.com/10016040/206948516-cc54bdac-6c48-4437-a891-bdbb5eba9595.png"/>

**English** | **[简体中文](README_CN.md)**

## 🔥 FlyCV —— High-performance Computer Vision System

![license](https://img.shields.io/hexpm/l/plug?color=%2300CD66&logo=apache&style=flat)
![GitHub star](https://img.shields.io/github/stars/paddlepaddle/flycv?color=%23FFB90F&logo=github)
![GitHub forks](https://img.shields.io/github/forks/paddlepaddle/flycv?color=pink&logo=github)
![GitHub issues](https://img.shields.io/github/issues/paddlepaddle/flycv?color=%2377BFFF&logo=github)
![main language](https://img.shields.io/github/languages/top/paddlepaddle/flycv?color=%23EEE685&logo=c%2B%2B)
![GitHub contributors](https://img.shields.io/github/contributors/paddlepaddle/flycv?color=orange&logo=github)
![Pull Request](https://img.shields.io/github/issues-pr/paddlepaddle/flycv?color=FFDAB9&logo=github)
![Commit activity](https://img.shields.io/github/commit-activity/m/paddlepaddle/flycv?color=F0FFFF&logo=github)
![Platforms](https://img.shields.io/static/v1?label=platforms&message=android|armlinux|windows|linux|ios&color=BBFFFF&logo=gnometerminal)

**FlyCV** is a **high-performance** computer image processing library. We are currently doing a lot of optimizations under the **ARM** architecture, which has better performance than other image processing libraries. At the same time, we also working on performance optimization of other CPUs and heterogeneous platforms, such as x86, RISC-V, GPU, DSP, etc. In addition to being faster, FlyCV provides more fine-grained compilation options, which makes it very **lightweight** in library size and can be **compiled on demand**. In addition, we also provide compile option support for customized namespaces, which can easily and quickly solve the problem of conflicts between the same dependent libraries.

Through the demo below, you can feel the speed and effect comparison between FlyCV and OpenCV.

<img width="32%" src="https://user-images.githubusercontent.com/10016040/203230955-e99ab754-b430-4a38-ba5b-cbcddda70bbc.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203231718-baf03baa-89f8-4fd8-b779-6e04c7ea0234.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203208952-26560bb6-9ab2-4243-acf1-2c4f8f5482c4.gif">

<img width="32%" src="https://user-images.githubusercontent.com/10016040/203209148-b00ed499-2695-4463-8c70-2d0fa642491a.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203209020-c7fafd21-b8e2-498e-8082-a4fa3d0241e9.gif">&nbsp;&nbsp;&nbsp;<img width="32%" src="https://user-images.githubusercontent.com/10016040/203208799-91196ae5-841d-4d27-b0cf-d1c51595c1f4.gif">

<br>

## :zap: Performance Evaluation

We used the benchmark framework to test on a variety of mobile phones. The following is the average acceleration ratio of each calculation of FlyCV and OpenCV at different resolutions of 6 mobile phones.

<div align=center>

<img width="33%" src="https://user-images.githubusercontent.com/10016040/210493794-59585f1d-16e8-4d73-bdd0-6ec5c3b00125.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495603-b1b7eb79-c53c-4414-bb14-6e37012e684b.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495677-a05126b2-07b4-4b91-b07f-67eed1ae0e4e.png">
  
<img width="33%" src="https://user-images.githubusercontent.com/10016040/210495748-4762c1c7-80fa-4fca-aebd-05ec0ef6e13b.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495792-acd20185-f394-4f36-8cd6-11d9e94793d9.png"><img width="33%" src="https://user-images.githubusercontent.com/10016040/210495843-2f1e2c36-88cb-43cc-99a5-c6bc2d658dc9.png">

</div>

<br>

## :closed_book: Quick Start

So come and experience it now. We have provided a few simple demos to help you quickly grasp the integration process.

See more details：[:book: Quick Start](https://github.com/PaddlePaddle/FlyCV/wiki/Quick-Start)。

<br>

## :notebook:  API Documents
To make it easier for everyone, our interface design is close to OpenCV. Howerver, there are some differences between the two.
Just see the interface documentation for more details: [📖 API Documents](https://github.com/PaddlePaddle/FlyCV/wiki/API-Documents)

<br>

## :notebook_with_decorative_cover: Compile Documents
We support most major operating systems, including android, arm linux, macos(x86 & arm), windows, ios. Relevant compilation scripts are provided for different platforms, which can easily compile source code on the local machine. Compared with OpenCV, in order to ensure extreme lightweight, we also provide coarse and fine-grained function module compilation options, which can truly support on-demand compilation, and can bring significant optimization in applications with very strict volume requirements.

You can compile from source by following the documents: [:book: Compile Manual](https://github.com/PaddlePaddle/FlyCV/wiki/Compile-manual)

We also provide precompiled libraries for different platforms, you can also download them directly: [Released Packages](https://github.com/PaddlePaddle/FlyCV/releases)

<br>

## :pushpin: ChangeLog
Check the change log to get the changes for each version: [ChangeLog](https://github.com/PaddlePaddle/FlyCV/wiki/ChangeLog)

<br>

## :two_men_holding_hands: Communication
<b>Ruliu:</b> Scan the QR code to chat with other members about ideas.

<div>
  Group of Wechat:
  <br/>
<img width=20% src="https://user-images.githubusercontent.com/10016040/218085479-bbef3d61-fa43-4e58-a513-56f924c3bb8c.png"/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
  <br/>
  <br/>
  Group of Ruliu:
  <br/>
<img width=20% src="https://user-images.githubusercontent.com/10016040/204523007-427e9423-ce0e-4c6b-806c-eae7d83936e6.png"/>
</div>

<br>

## :pencil2: Contributing
See the [CONTRIBUTING.md](./CONTRIBUTING.md) to get start with the contribution. Thanks for everyone's contributions.

<a href="https://github.com/paddlepaddle/flycv/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=paddlepaddle/flycv" />
</a>

<br>

## :copyright: License
[Apache-2.0](./LICENSE)
