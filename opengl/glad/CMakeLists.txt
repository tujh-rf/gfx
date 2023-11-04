set( project_name glad )
set( targets ${project_name} )

set( GLAD_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} )
set( GLAD_ROOT_DIR ${GLAD_ROOT_DIR} PARENT_SCOPE )
set( GLAD_INCLUDE_DIR ${GLAD_ROOT_DIR}/include )
set( GLAD_INCLUDE_DIR ${GLAD_INCLUDE_DIR} PARENT_SCOPE )
set( GLAD_SOURCE_DIR ${GLAD_ROOT_DIR}/src )
set( GLAD_LIBRARY glad )
set( GLAD_LIBRARY ${GLAD_LIBRARY} PARENT_SCOPE )

set( glad_sources "${GLAD_SOURCE_DIR}/glad.c" )
source_group( "glad" ${glad_sources} )

add_library( ${project_name} STATIC ${glad_sources} )
target_include_directories( ${project_name} PRIVATE ${GLAD_INCLUDE_DIR} )
