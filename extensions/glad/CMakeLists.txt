
set( GLAD_PROJ glad )
set( target ${GLAD_PROJ} )

set( GLAD_FOUND true PARENT_SCOPE )
set( GLAD_ROOT ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE )
set( GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE )
set( GLAD_LIBRARY ${GLAD_PROJ} PARENT_SCOPE )


set( glad_sources
    "${CMAKE_CURRENT_SOURCE_DIR}/src/glad.c"
)

add_library( ${GLAD_PROJ}
    STATIC
        EXCLUDE_FROM_ALL
        ${glad_sources}
)
target_include_directories( ${GLAD_PROJ}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
