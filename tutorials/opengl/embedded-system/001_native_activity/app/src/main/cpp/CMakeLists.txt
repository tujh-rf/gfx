cmake_minimum_required( VERSION 3.22.1 FATAL_ERROR )

set( EXAMPLE_NAME a001_native_activity )

project( ${EXAMPLE_NAME} C CXX )

# native_app_glue
add_library( native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

# sample sources all-in-one
set( example_src
        main.cpp
)

set(CMAKE_SHARED_LINKER_FLAGS
        "${CMAKE_SHARED_LINKER_FLAGS} -u \
            ANativeActivity_onCreate")

# Android native application is a shared library
# loaded by Kotlin
add_library(
    ${EXAMPLE_NAME} SHARED
        ${example_src}
)

# native glue code path
target_include_directories(
    ${EXAMPLE_NAME} PRIVATE
        ${ANDROID_NDK}/sources/android/native_app_glue
)

# link sample with system libraries
target_link_libraries(
    ${EXAMPLE_NAME} PUBLIC
        native_app_glue

        EGL
        GLESv3

        jnigraphics
        android
        log
)
