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

# operation system detection
string( TOLOWER "${CMAKE_SYSTEM_NAME}" CMAKE_SYSTEM_NAME_LOWER )
if( "${CMAKE_SYSTEM_NAME_LOWER}" STREQUAL "windows" )
    set( TARGET_SYSTEM_IS_WINDOWS TRUE )
elseif( "${CMAKE_SYSTEM_NAME_LOWER}" STREQUAL "darwin" )
    set( TARGET_SYSTEM_IS_APPLE TRUE )
elseif( "${CMAKE_SYSTEM_NAME_LOWER}" STREQUAL "linux" )
    set( TARGET_SYSTEM_IS_LINUX TRUE )
endif()

if( TARGET_SYSTEM_IS_WINDOWS OR TARGET_SYSTEM_IS_LINUX OR TARGET_SYSTEM_IS_APPLE )
    set( SUPPORT_OPENGL TRUE )
endif()
