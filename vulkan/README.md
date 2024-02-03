# Vulkan

Cross-platform API an open standard for low-level low-overhead 3D graphics.

First release was made on 16 February 2016. Current version of Vulkan API is 1.3.

Vulkan designed provide a single API for desktop and mobile graphics and support wide variety of GPUs, modern multi-core CPUs and operation systems.

[GLFW](https://www.glfw.org) from the version 3.2 has Vulkan runtime loader so no additional libraries are needed.

P.S. In case of error like:

> Khronos validation layer doesn't found

or

> Vulkan error: loader_validate_layers: Layer 0 does not exist in the list of available layers

try to build samples in Release mode (without validation layer)
