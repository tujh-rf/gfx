# Vulkan 1.3 tutorial

include( volk )

fetch_volk()

add_subdirectory( 001_vulkan_glfw_window )
add_subdirectory( 002_vulkan_instance )
add_subdirectory( 003_vulkan_debug_utils_ext )
add_subdirectory( 004_vulkan_surface )
add_subdirectory( 005_vulkan_phy_device )
add_subdirectory( 006_vulkan_device )
add_subdirectory( 007_vulkan_swapchain )
add_subdirectory( 008_vulkan_swapchain_images )
add_subdirectory( 009_vulkan_image_views )
add_subdirectory( 010_vulkan_swapchain_recreation )
add_subdirectory( 011_vulkan_render_pass )
add_subdirectory( 012_vulkan_frame_buffers )
add_subdirectory( 013_vulkan_sync_objects )
add_subdirectory( 014_vulkan_cmd_buffer )
add_subdirectory( 015_vulkan_initialization )
add_subdirectory( 016_vulkan_simple_fullscreen )
