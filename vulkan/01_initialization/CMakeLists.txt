set( project_name 01_vulkan_initialization )
set( targets ${project_name} )

file( GLOB project_sources "*.cpp" )
source_group( "Source" ${project_sources} )

add_executable( ${project_name} ${project_sources} )
add_dependencies( ${project_name} ${GLFW_LIBRARY} )

# path to GLFW library and volk header
target_include_directories( ${project_name} PRIVATE ${VULKAN_HEADERS_INCLUDE_DIR} ${GLFW_INCLUDE_DIR} ${VOLK_INCLUDE_DIR} )
# link executable with GLFW
target_link_libraries( ${project_name} PRIVATE ${GLFW_LIBRARY} ${VOLK_LIBRARY} )
