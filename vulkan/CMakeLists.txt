
include( fetch_vulkan )

# don't need to call find_package(Vulkan) because volk will load Vulkan in runtime
fetch_volk()

# add Vulkan samples
if( SAMPLES_VULKAN )
    add_subdirectory( 01_initialization )
endif()
