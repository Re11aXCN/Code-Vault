OpenGL核心本身只是一种规范，算起来，和CPP规范差不多

是访问GPU的API，每个显卡制造商都会有OpenGL的实现，OpenGL是跨平台的

Metal是Mac的实现，DirectX是微软的实现，但都不是跨平台

**详细见csdn**

[现代图形API综合比较：Vulkan | DirectX | Metal | WebGPU](https://blog.csdn.net/shebao3333/article/details/137948965)

Vulkan 学起来比较难，OpenGL更合适更友好



因为Direct3D 11确实是一个创建图形API的好方法



传统的DpenGL更像是一套程序，换句话说，你基本上画了个三角形，并且，你说你想要光源，那么，你怎么把光源加到这个三角形上呢，基本上是这样的，你说光源等手true，你将激活OpenGL的光源

所以传统OpenGL和现代OpenGL之间最大的区别就是着色器，**着色器是程序，它是在你GPU上运行的代码**，我们想要更为精确的控制显卡运行，我们可能要将大部分代码从CPU转到GPU上，因为它在GPU上运行更快