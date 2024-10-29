cmake_minimum_required( VERSION 3.22.1 FATAL_ERROR )

set( EXAMPLE_NAME a003_initialization )

project( ${EXAMPLE_NAME} C CXX )

# search GameActivity package
find_package( game-activity REQUIRED CONFIG )

# sample sources all-in-one
set( example_src
        main.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u \
    Java_com_google_androidgamesdk_GameActivity_initializeNativeCode")

# Android native application is a shared library
# loaded by Kotlin
add_library(
    ${EXAMPLE_NAME} SHARED
        ${example_src}
)

# link sample with system libraries
target_link_libraries(
    ${EXAMPLE_NAME} PUBLIC
        # game activity
        game-activity::game-activity_static

        EGL
        GLESv3
    
        jnigraphics
        android
        log
)
