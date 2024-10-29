# OpenGL (Open Graphic Library)

[OpenGL](https://www.opengl.org) is a cross-platform and cross-language API supported by [Khronos](https://www.khronos.org/OpenGL), however in this tutorials C and C++ will be used.

Main feature of the OpenGL library is that the library operates with internal state machine. In the time when the library was created (first release was made on 30 June 1992) this had a lot of benefits. And still this is the easiest way to start development of graphical application without detailed knowledge how the graphical pipeline works.

However, nowadays this feature has more disadvantages because not allows to control every step of the graphical pipeline.

Because of that, Khronos developed new graphical API to replace OpenGL - [Vulkan](../vulkan/README.md).

Last version of OpenGL is 4.6 and was released 31 July 2017 and this version will be used in all tutorials.

> Apple decided that OpenGL was deprecated on their platforms, however it is still possible to use OpenGL version 4.1 on Apple devices, but correct behavior is not guarantee. This is why OpenGL tutorials and samples marked as non available for MacOS.

OpeGL on this moment has few different implementations:

* [OpenGL with Fixed Functional Pipeline](fixed-pipeline/README.md) (OpenGL 1.0 - 3.0 Core)
* [OpenGL with Programmable Pipeline](programmable-pipeline/README.md) (OpenGL 3.0 Core - 4.6 Core)
* [OpenGL for Embedded Systems](embedded-system/README.md)

---
