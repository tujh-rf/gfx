# Vulkan Instance

Before to start working with Vulkan the instance of the Vulkan must be created. Here is a key difference between the OPenGL and Vulkan. For OpenGL the only one global instance exist and for Vulkan is possible to create few independent instances.

To start using Vulkan instance is needed to compare the list of supported extensions with requirements. To make this simpler in the tutorial the predefined list from GLFW library is in use.

---
