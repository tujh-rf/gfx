# Vulkan DebugUtils

It is importand not only use graphical API but also read the debug output from it. Vulkan provides the posibility to read debug messages from the driver via special layer which is called VK_LAYER_KHRONOS_validation.

The layer must be properly created before use and of course destroyed after.

Also the level of debug messages might be set differently, in the tutorials the only debug level will be used.

---
