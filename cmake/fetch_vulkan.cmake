include( FetchContent )

function( FETCH_VULKAN_HEADERS )
    message( CHECK_START "fetching vulkan headers" )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            vulkan_headers
            GIT_REPOSITORY  https://github.com/KhronosGroup/Vulkan-Headers.git
            GIT_TAG         vulkan-sdk-1.3.290.0
            GIT_SHALLOW     TRUE
        )

        set( VULKAN_HEADERS_ENABLE_MODULE OFF )

        FetchContent_MakeAvailable( vulkan_headers )

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( VULKAN_HEADERS_FOUND true PARENT_SCOPE )
    set( VULKAN_HEADERS_ROOT ${vulkan_headers_SOURCE_DIR} )
    set( VULKAN_HEADERS_ROOT ${VULKAN_HEADERS_ROOT} PARENT_SCOPE )
    set( VULKAN_HEADERS_INCLUDE_DIR ${VULKAN_HEADERS_ROOT}/include PARENT_SCOPE )
    set( VULKAN_HEADERS_LIBRARY vulkan_headers )
    set( VULKAN_HEADERS_LIBRARY ${VULKAN_HEADERS_LIBRARY} PARENT_SCOPE )

endfunction()

function( FETCH_VOLK )
    message( CHECK_START "fetching volk" )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            volk
            GIT_REPOSITORY  https://github.com/zeux/volk.git
            GIT_TAG         vulkan-sdk-1.3.290.0
            GIT_SHALLOW     TRUE
        )

        # set volk options
        set( VOLK_STATIC_DEFINES ON )
        set( VOLK_PULL_IN_VULKAN OFF )

        FetchContent_MakeAvailable( volk )

        if( VULKAN_HEADERS_FOUND )
            target_include_directories( volk PRIVATE ${VULKAN_HEADERS_INCLUDE_DIR} )
        endif()

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( VOLK_FOUND true PARENT_SCOPE )
    set( VOLK_ROOT ${volk_SOURCE_DIR} )
    set( VOLK_ROOT ${VOLK_ROOT} PARENT_SCOPE )
    set( VOLK_INCLUDE_DIR ${VOLK_ROOT} PARENT_SCOPE )
    set( VOLK_LIBRARY volk )
    set( VOLK_LIBRARY ${VOLK_LIBRARY} PARENT_SCOPE )

endfunction()
