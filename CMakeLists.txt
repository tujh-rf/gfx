cmake_minimum_required( VERSION 3.20 FATAL_ERROR )

project( gfx_samples C CXX )

# add folder for project specific includes
list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake" )

# include CMake settings for the project
include( samples_settings )

# collect all binaries into one folder
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin" )

# setup assets
set( ASSETS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/assets )
configure_file( ${ASSETS_DIR}/assets.hpp.in ${ASSETS_DIR}/assets.hpp )

# OpenGL samples
if( SUPPORT_OPENGL )
    add_subdirectory( opengl )
endif()
