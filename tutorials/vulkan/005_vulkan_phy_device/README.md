# Physical device

Vulkan supports different type of the physical devices, like CPUs, integrated or dedicated GPUs and so on. Before start using any of this devices the application must choose the most suitable for the application needs. For example the application may reject to run on integrated GPU and require the only dedicated graphical card. Or the application might require the graphical card with actual output because some of dedicated GPUs installed in the server might not have this feature and be used only for calculations without visualization.

Tutorial will test available options of the physical devices available in the operation system for the Vulkan API and will choose the first GPU with the output feature.

---
