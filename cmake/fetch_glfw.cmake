include( FetchContent )

function( FETCH_GLFW )
    message( CHECK_START "fetching GLFW" )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            glfw
            GIT_REPOSITORY  https://github.com/glfw/glfw.git
            GIT_TAG         3.3.9
            GIT_SHALLOW     TRUE
        )

        # set GLFW options
        set( BUILD_SHARED_LIBS OFF )
        set( GLFW_BUILD_EXAMPLES OFF )
        set( GLFW_BUILD_TESTS OFF )
        set( GLFW_BUILD_DOCS OFF )
        set( GLFW_VULKAN_STATIC OFF )
        # experimental option for Linux - Wayland
        set( GLFW_USE_WAYLAND ON )

        FetchContent_MakeAvailable( glfw )

        if( VULKAN_HEADERS_FOUND )
            target_include_directories( glfw PRIVATE ${VULKAN_HEADERS_INCLUDE_DIR} )
        endif()

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( GLFW_FOUND true PARENT_SCOPE )
    set( GLFW_ROOT ${glfw_SOURCE_DIR} )
    set( GLFW_ROOT ${GLFW_ROOT} PARENT_SCOPE )
    set( GLFW_INCLUDE_DIR ${GLFW_ROOT}/include PARENT_SCOPE )
    set( GLFW_LIBRARY glfw )
    set( GLFW_LIBRARY ${GLFW_LIBRARY} PARENT_SCOPE )

endfunction()
