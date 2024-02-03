cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

project( gfx_samples C CXX )

# add folder for project specific includes
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" )

# include CMake settings for the project
include( samples_settings )
include( fetch_vulkan )
include( fetch_glfw )

#
option( SAMPLES_OPENGL2 "Build OpenGL 2.1 samples" ON )
option( SAMPLES_OPENGL4 "Build OpenGL 4.1 samples" ON )
option( SAMPLES_VULKAN "Build Vulkan samples" ON )

# collect all binaries into one folder
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" )

# setup assets
set( ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets )
configure_file( ${ASSETS_DIR}/assets.hpp.in ${ASSETS_DIR}/assets.hpp )

# fetch vulkan headers first if Vulkan is supported
# to use Vulkan headers from the dependency and not headers installed
# in the system, this needs to be executed before fetching other dependencies
if( SUPPORT_VULKAN )
    fetch_vulkan_headers()
endif()

# fetch GLFW - window and input manager
fetch_glfw()

# OpenGL samples
if( SUPPORT_OPENGL )
    add_subdirectory( opengl )
endif()

# Vulkan samples
if( SUPPORT_VULKAN )
    add_subdirectory( vulkan )
endif()
