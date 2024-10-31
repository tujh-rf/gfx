cmake_minimum_required( VERSION 3.25 FATAL_ERROR )

project( gfx
    C CXX
)

# cmake modules folder
list( APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake_modules"
)

# C++ options
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

# force to release build if nothing else is defined
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Force to set Release build" FORCE)
endif()
add_compile_definitions(
    $<IF:$<CONFIG:DEBUG>,_DEBUG,NDEBUG>
)

# set MSVC warning C4996 to "off"
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# place all artefacts into one folder
# NOTE: (please don't do this in real projects)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" )

# list of supported APIs
if( LINUX OR WIN32 )
    set( SUPPORT_VULKAN true )
endif()
if( LINUX OR WIN32 )
    set( SUPPORT_OPENGL true )
endif()
if( WIN32 )
    set( SUPPORT_DIRECTX true )
endif()


# options
option( GFX_TUTORIALS "All graphical API tutorials" ON )
option( GFX_SAMPLES "All graphical API samples" ON )
if( SUPPORT_OPENGL )
    option( OPENGL2_TUTORIAL "OpenGL2 Fixed Functional pipeline tutorial" ON )
    option( OPENGL2_SAMPLES "OpenGL2 Fixed Functional pipeline samples" OFF )
    option( OPENGL4_TUTORIAL "OpenGL4 Programmable pipeline tutorial" ON )
    option( OPENGL2_SAMPLES "OpenGL4 Programmable pipeline samples" OFF )
endif()

# if Vulkan is suppoted by the target system - fetch latest headers first
# this is needed to use non system vulkan headers for all targets on any target system
if( SUPPORT_VULKAN )
    include( vulkan_headers )
    fetch_vulkan_headers()
endif()

# OpenGL tutorials and samples will use Glad for runtime initialization of OpenGL
# and the same way Vulkan tutorials and samples will use Volk loader
# the main difference is that Glad source files will be stored in the repository
# and Volk will be downloaded from GitHub
add_subdirectory( extensions )

# fetch GLFW from GitHub
include( glfw )
fetch_glfw()

# tutorials
if( GFX_TUTORIALS )
    add_subdirectory( tutorials )
endif()
