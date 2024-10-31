# Computer graphic programming

This repository provides very basic information regarding different graphical APIs, tutorials to start working with those APis and set of samples which might be interesting to developers who are studying programming of the computer graphic.

To be able to build and run tutorials and samples next software will be needed:

* CMake, version not older that 3.25.
* C and C++ compilers (GCC/Clang for Linux, MSVC for Microsoft Windows and Clang for MacOS and iOS), it is possible to use other compilers but it wasn't tested.
* git, will be needed to download additional software packages from the Github.
* System libraries for the graphical development then it is needed (libX11, libXrand for example for the Linux platform and of course other)
* For OpenGLES tutorials and samples also the Android Studio will be needed.
* For DirectX tutorials and samples Windows SDK will be needed because there is no way to download it properly in the preparation stage.

In the `cmake_modules` folder additional modules might be found to export software packages from Github during the setup of the project.

`extensions` folder contains third party libraries which cannot be easily taken from any of public repositories and must be stored in the current repo. One of those libraries is [Glad](extensions/glad/README.md)

[Tutorials](tutorials/README.md)

---
