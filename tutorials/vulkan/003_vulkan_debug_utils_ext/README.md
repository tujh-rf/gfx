# Vulkan DebugUtils

If supported the application can receive debug messages from the Vulkan driver. To make it work the application needs to check if the layer VK_LAYER_KHRONOS_validation exist.
All messages will be send via special messenger object which also needs to be created and properly destroyed.
