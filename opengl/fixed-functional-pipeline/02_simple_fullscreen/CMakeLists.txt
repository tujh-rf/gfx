set( project_name 02_ogl21_simple_fullscreen )
set( targets ${project_name} )

file( GLOB project_sources "*.cpp" )
source_group( "Source" ${project_sources} )

add_executable( ${project_name} ${project_sources} )
add_dependencies( ${project_name} ${GLFW_LIBRARY} ${GLAD_LIBRARY} )

# path to GLFW library, Glad headers and OpenGL headers
target_include_directories( ${project_name} PRIVATE ${GLFW_INCLUDE_DIR} ${GLAD_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR} )
# link executable with GLFW and Glad static libraries
target_link_libraries( ${project_name} ${GLFW_LIBRARY} ${GLAD_LIBRARY} )
