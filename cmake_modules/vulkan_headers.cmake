include( FetchContent )

function( FETCH_VULKAN_HEADERS )
    message( CHECK_START "fetching Vulkan headers" )

    set( VULKAN_HEADERS_PROJ vulkan_headers )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            ${VULKAN_HEADERS_PROJ}
            GIT_REPOSITORY  https://github.com/KhronosGroup/Vulkan-Headers.git
            GIT_TAG         vulkan-sdk-1.3.290.0
            GIT_SHALLOW     TRUE
        )

        set( VULKAN_HEADERS_ENABLE_MODULE OFF )

        FetchContent_MakeAvailable( ${VULKAN_HEADERS_PROJ} )

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( VULKAN_HEADERS_FOUND true PARENT_SCOPE )
    set( VULKAN_HEADERS_ROOT ${vulkan_headers_SOURCE_DIR} PARENT_SCOPE )
    set( VULKAN_HEADERS_INCLUDE_DIR ${vulkan_headers_SOURCE_DIR}/include PARENT_SCOPE )
    set( VULKAN_HEADERS_LIBRARY ${VULKAN_HEADERS_PROJ} PARENT_SCOPE )

endfunction()
