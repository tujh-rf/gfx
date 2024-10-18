# Physical device

Before start working with Vulkan it is needed to choose the actual physical device.
Because Vulkan provides the opportunity to operate with all type of devices, like CPUs and GPUs installed in the system, not all of them are suitable for the graphical output.
First of all the application should take into account only graphical devices - exclude CPUs. Second step will be to check if the GPU has actual graphical output. Some GPUs doesn't have any graphical ports for the screen connections because they are using for math calculations and not for graphics.
