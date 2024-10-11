include( FetchContent )

function( FETCH_VOLK )
    message( CHECK_START "fetching volk" )

    set( VOLK_PROJ volk )

    list( APPEND CMAKE_MESSAGE_INDENT "  " )

        FetchContent_Declare(
            ${VOLK_PROJ}
            GIT_REPOSITORY  https://github.com/zeux/volk.git
            GIT_TAG         vulkan-sdk-1.3.290.0
            GIT_SHALLOW     TRUE
        )

        # set volk options
        set( VOLK_STATIC_DEFINES ON )
        set( VOLK_PULL_IN_VULKAN OFF )

        FetchContent_MakeAvailable( ${VOLK_PROJ} )

        if( VULKAN_HEADERS_FOUND )
            target_include_directories( ${VOLK_PROJ} PRIVATE ${VULKAN_HEADERS_INCLUDE_DIR} )
        endif()

    list( POP_BACK CMAKE_MESSAGE_INDENT )

    message( CHECK_PASS "done" )

    set( VOLK_FOUND true PARENT_SCOPE )
    set( VOLK_ROOT ${volk_SOURCE_DIR} PARENT_SCOPE )
    set( VOLK_INCLUDE_DIR ${volk_SOURCE_DIR} PARENT_SCOPE )
    set( VOLK_LIBRARY ${VOLK_PROJ} PARENT_SCOPE )

endfunction()
