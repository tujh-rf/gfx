
# OpenGL tutorials
if( SUPPORT_OPENGL )
    add_subdirectory( opengl )
endif()

# Vulkan tutorial
if( SUPPORT_VULKAN )
    add_subdirectory( vulkan )
endif()

# DirectX
if( SUPPORT_DIRECTX )
    add_subdirectory( directx )
endif()
