# don't need to call find_package(OpenGL) because Glad will load OpenGL in runtime

# OpenGL loader
add_subdirectory( glad )

# add OpenGL Fixed Functional Pipeline samples (OpenGL 2.1)
if( SAMPLES_OPENGL2 )
    add_subdirectory( fixed-functional-pipeline )
endif()

# add OpenGL Programmable Pipeline samples (OpenGL 4.1)
if( SAMPLES_OPENGL4 )
    add_subdirectory( programmable-pipeline )
endif()
