
# it is not needed to use find_package(OpenGL) here,
# Glad will load the OpenGL in the runtime 

# OpenGL 2.1 Fixed Functional pipeline
if( OPENGL2_TUTORIAL )
    add_subdirectory( fixed-pipeline )
endif()

# OpenGL 4.6 Programmable pipeline
if( OPENGL4_TUTORIAL )
    add_subdirectory( programmable-pipeline )
endif()
