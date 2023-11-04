# for this samples OpenGL is required
find_package( OpenGL REQUIRED )

include( fetch_glfw )

# fetch 3rd party libraries from GitHub
fetch_glfw()

# OpenGL loader
add_subdirectory( glad )

# add OpenGL Fixed Functional Pipeline samples (OpenGL 2.1)
add_subdirectory( fixed-functional-pipeline )

# add OpenGL Programmable Pipeline samples (OpenGL 4.1)
add_subdirectory( programmable-pipeline )
