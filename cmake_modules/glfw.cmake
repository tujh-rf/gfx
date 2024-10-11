include( FetchContent )

function( FETCH_GLFW )
    message( CHECK_START "fetching GLFW" )

    set( GLFW_PROJ glfw )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            ${GLFW_PROJ}
            GIT_REPOSITORY  https://github.com/glfw/glfw.git
            GIT_TAG         3.4
            GIT_SHALLOW     TRUE
        )

        # set GLFW options
        set( BUILD_SHARED_LIBS OFF )
        set( GLFW_BUILD_EXAMPLES OFF )
        set( GLFW_BUILD_TESTS OFF )
        set( GLFW_BUILD_DOCS OFF )
        set( GLFW_VULKAN_STATIC OFF )

        FetchContent_MakeAvailable( ${GLFW_PROJ} )

        if( VULKAN_HEADERS_FOUND )
            target_include_directories( ${GLFW_PROJ} PRIVATE ${VULKAN_HEADERS_INCLUDE_DIR} )
        endif()

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( GLFW_FOUND true PARENT_SCOPE )
    set( GLFW_ROOT ${glfw_SOURCE_DIR} PARENT_SCOPE )
    set( GLFW_INCLUDE_DIR ${glfw_SOURCE_DIR}/include PARENT_SCOPE )
    set( GLFW_LIBRARY ${GLFW_PROJ} PARENT_SCOPE )

endfunction()
