
# read the folder name as the target name
get_filename_component( project_name ${CMAKE_CURRENT_SOURCE_DIR} NAME )
string( REPLACE " " "_" project_name ${project_name} )
set( targets ${project_name} )

file( GLOB project_sources "*.cpp" )

add_executable( ${project_name}
    ${project_sources}
)
add_dependencies( ${project_name}
    ${GLFW_LIBRARY}
    ${VOLK_LIBRARY}
)

# path to GLFW library, Glad headers and OpenGL headers
target_include_directories( ${project_name}
    PRIVATE
        ${VULKAN_HEADERS_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${VOLK_INCLUDE_DIR}
)
# link executable with GLFW and Glad static libraries
target_link_libraries( ${project_name}
    ${GLFW_LIBRARY}
    ${VOLK_LIBRARY}
)
