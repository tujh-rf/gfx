/*
    Vulkan 1.3 tutorial
    
    Vulkan Instance
 */

#include <cstdlib>
#include <iostream>
#include <vector>

/* don't load Vulkan, will be done by volk
 */
#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h>
#include <volk.h>


/* Vulkan error check
 */
#define VK_CALL( func, err_msg ) { \
    if( VK_SUCCESS != func ) {     \
        std::cerr                  \
            << "VK_CALL error: "   \
                << err_msg         \
                << std::endl;      \
        return false;              \
    }                              \
}

/* Tutorial vulkan function call
 */
#define TUTORIAL_CALL( func ) { \
    if( !func )                 \
        return false;           \
}

/* global constants
 */
static const int window_width  = 800;
static const int window_height = 600;

/* application data
 */
struct vkApp {
    struct {
        bool         init   { false };
        GLFWwindow  *window { nullptr };
    } glfw;
    struct {
        struct {
            VkInstance object { VK_NULL_HANDLE };

            std::vector< const char* > required_extensions;
        } instance;
    } vulkan;
};

/* callbacks
 */

static void glfw_error_callback( int error,
                                 const char* description ) {
    std::cerr
        << "GLFW error: "
            << description
                << std::endl;
}

static void key_callback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
) {
    /* ESC - close application
    */
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GLFW_TRUE );
}

/* GLFW library
 */

static bool init_glfw( vkApp &app ) {
    /* setup error callback first to catch all errors
     */
    (void)glfwSetErrorCallback( glfw_error_callback );

    /* init the library
     */
    if( glfwInit() == GLFW_FALSE )
        return false;

    /* Prepare the list of require extensions
     */
    uint32_t glfw_require_extension_count = 0;
    const char **glfw_require_extensions =
        glfwGetRequiredInstanceExtensions( &glfw_require_extension_count );

    for( uint32_t i = 0; i < glfw_require_extension_count; ++i )
        app.vulkan.instance.required_extensions.push_back( glfw_require_extensions[i] );

    app.glfw.init = true;

    return true;
}

static bool cleanup_glfw( vkApp &app ) {
    if( !app.glfw.init )
        return true;

    /* cleanup GLFW resources
     */
    glfwTerminate();
    app.glfw.init = false;

    return true;
}

/* GLFW window
 */

static bool init_window( vkApp &app ) {
    if( !app.glfw.init )
        return false;

    /* don't create any graphical context
     */
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    app.glfw.window = glfwCreateWindow(
        window_width,
        window_height,
        "Vulkan 1.3 - Tutorial - Vulkan Instance",
        nullptr,
        nullptr
    );
    if( !app.glfw.window )
        return false;

    /* setup user pointer
     */
    glfwSetWindowUserPointer(
        app.glfw.window,
       &app
    );

    /* acquire the keyboard
     */
    glfwSetKeyCallback(
        app.glfw.window,
        key_callback
    );

    return true;
}

static bool cleanup_window( vkApp &app ) {
    if( !app.glfw.window )
        return true;

    /* destroy GLFW window
     */
    glfwDestroyWindow( app.glfw.window );
    app.glfw.window = nullptr;

    return true;
}

/* Vulkan instance
 */

static bool vk_create_instance( vkApp &app ) {
    if( !app.glfw.window )
        return false;

    /* fullfil application information
     */
    VkApplicationInfo vk_app_info {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO, /* sType MUST be always set */
        .pApplicationName   = "Vulkan tutorial application",
        .applicationVersion = VK_MAKE_API_VERSION( 0, 1, 0, 0 ),
        .pEngineName        = "Vulkan tutorials",
        .engineVersion      = VK_MAKE_API_VERSION( 0, 1, 0, 0 ),
        .apiVersion         = VK_API_VERSION_1_3  /* require Vulkan 1.3 */
    };

    VkInstanceCreateInfo vk_instance_create_info {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &vk_app_info,
        .enabledExtensionCount   = static_cast< uint32_t > ( app.vulkan.instance.required_extensions.size() ),
        .ppEnabledExtensionNames = app.vulkan.instance.required_extensions.data()
    };

    /* Create Vulkan instance
     */
    VK_CALL( vkCreateInstance( &vk_instance_create_info,
                                nullptr,
                               &app.vulkan.instance.object ),
             "Cannot create Vulkan instance" );

    /* Initialize all pointer to Vulkan functions
     */
    volkLoadInstanceOnly( app.vulkan.instance.object );

    return true;
}

static bool vk_cleanup_instance( vkApp &app ) {
    if( app.vulkan.instance.object != VK_NULL_HANDLE ) {
        vkDestroyInstance( app.vulkan.instance.object,
                           nullptr );
        app.vulkan.instance.object = VK_NULL_HANDLE;
    }

    return true;
}

/* common Vulkan functions
 */

static bool init_vulkan( vkApp &app ) {
    if( !app.glfw.window )
        return false;

    /* Call volk to load Vulkan
     */
    VK_CALL( volkInitialize(),
             "Cannot initialize Vulkan loader" );

    TUTORIAL_CALL( vk_create_instance( app ) );

    return true;
}

static bool cleanup_vulkan( vkApp &app ) {
    TUTORIAL_CALL( vk_cleanup_instance( app ) );

    return true;
}

static bool init( vkApp &app ) {
    TUTORIAL_CALL( init_glfw( app ) );
    TUTORIAL_CALL( init_window( app ) );
    TUTORIAL_CALL( init_vulkan( app ) );

    return true;
}

static bool cleanup( vkApp &app ) {
    TUTORIAL_CALL( cleanup_vulkan( app ) );
    TUTORIAL_CALL( cleanup_window( app ) );
    TUTORIAL_CALL( cleanup_glfw( app ) );

    return true;
}

static void draw( vkApp &app ) {
    /* nothing for now */
}

int main() {
    vkApp app;

    if( !init( app ) ) {
        std::cerr
            << "Cannot initialize the application"
                << std::endl;
        return EXIT_FAILURE;
    }

    /* render loop
     */
    while( glfwWindowShouldClose( app.glfw.window ) == GLFW_FALSE ) {
        /* draw the context of the window
         */
        draw( app );

        /* proceed keyboard and mouse
         */
        glfwPollEvents();
    }

    if( !cleanup( app ) ) {
        std::cerr
            << "Cleanup failed"
                << std::endl;
        return EXIT_FAILURE;
    }

    std::cout
        << "Job is done!"
            << std::endl;
    return EXIT_SUCCESS;
}
