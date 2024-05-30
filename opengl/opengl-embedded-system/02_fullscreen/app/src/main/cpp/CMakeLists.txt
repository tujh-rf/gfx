cmake_minimum_required( VERSION 3.22.1 FATAL_ERROR )

set( EXAMPLE_NAME a02_fullscreen )

project( ${EXAMPLE_NAME} C CXX )

# search GameActivity package
find_package(game-activity REQUIRED CONFIG)

# application glue source file
set( native_app_glue_src
        native_app_glue.cpp
)
# game activity glue file
set( native_activity_src
    native_game_activity.cpp
)
# sample sources all-in-one
set( example_src
    ${native_app_glue_src}
    ${native_activity_src}
    main.cpp
)

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
