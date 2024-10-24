/*
    Vulkan 1.3 tutorial
    
    DebugUtils Extension
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

static const char khronos_validation_layer_name[] = "VK_LAYER_KHRONOS_validation";

/* application data
 */
struct vkApp {
    struct {
        bool         init   { false };
        GLFWwindow  *window { nullptr };
    } glfw;
    struct {
        struct {
            bool                     enable    { false };
            VkDebugUtilsMessengerEXT messenger { VK_NULL_HANDLE };
        } debug;
        struct {
            VkInstance object { VK_NULL_HANDLE };

            std::vector< const char* > required_extensions;
            std::vector< const char* > require_layers;
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

static VKAPI_ATTR VkBool32 VKAPI_CALL vk_error_callback(
          VkDebugUtilsMessageSeverityFlagBitsEXT severity,
          VkDebugUtilsMessageTypeFlagsEXT        type,
    const VkDebugUtilsMessengerCallbackDataEXT  *data,
          void                                  *user_data
) {
    std::cerr
        << "Vulkan error: "
            << data->pMessage
                << std::endl;

    /* should always return VK_FALSE
     */
    return VK_FALSE;
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
        "Vulkan 1.3 - Tutorial - DebugUtilsEXT",
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

    /* read the actual supported version
     */
    uint32_t actual_version = 0;
    VK_CALL( vkEnumerateInstanceVersion( &actual_version ),
             "Cannot get the actual supported version of Vulkan" );
    std::cout
        << "Actual Vulkan version: "
            << VK_API_VERSION_MAJOR( actual_version )
            << '.'
            << VK_API_VERSION_MINOR( actual_version )
            << '.'
            << VK_API_VERSION_PATCH( actual_version )
            << std::endl;

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

    /* Request debug layer support if needed
     */
    if( app.vulkan.debug.enable )
        app.vulkan.instance.require_layers.push_back( khronos_validation_layer_name );

    /* Prepare debug callback to accept only errors
     */
    VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_info {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT       \
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT \
                            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &vk_error_callback
    };

    VkInstanceCreateInfo vk_instance_create_info {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &vk_app_info,
        .enabledLayerCount       = static_cast< uint32_t > ( app.vulkan.instance.require_layers.size() ),
        .ppEnabledLayerNames     = app.vulkan.instance.require_layers.data(),
        .enabledExtensionCount   = static_cast< uint32_t > ( app.vulkan.instance.required_extensions.size() ),
        .ppEnabledExtensionNames = app.vulkan.instance.required_extensions.data()
    };

    /* link DebugUtilsMessenger extension to VkInstanceCreateInfo if needed
     */
    if( app.vulkan.debug.enable )
        vk_instance_create_info.pNext = static_cast< void* > ( &vk_debug_utils_info );

    /* Create Vulkan instance
     */
    VK_CALL( vkCreateInstance( &vk_instance_create_info,
                                nullptr,
                               &app.vulkan.instance.object ),
             "Cannot create Vulkan instance" );

    /* Initialize all pointer to Vulkan functions
     */
    volkLoadInstanceOnly( app.vulkan.instance.object );

    /* Instance object has information about debug messenger,
     * but messenger itself must be created
     */
    if( app.vulkan.debug.enable ) {
        VK_CALL( vkCreateDebugUtilsMessengerEXT( app.vulkan.instance.object,
                                                &vk_debug_utils_info,
                                                 nullptr,
                                                &app.vulkan.debug.messenger ),
                 "Cannot create Vulkan debug messenger" );
    }

    return true;
}

static bool vk_cleanup_instance( vkApp &app ) {
    if( app.vulkan.debug.messenger != VK_NULL_HANDLE ) {
        vkDestroyDebugUtilsMessengerEXT( app.vulkan.instance.object,
                                         app.vulkan.debug.messenger,
                                         nullptr );
        app.vulkan.debug.messenger = VK_NULL_HANDLE;
    }
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

    /* Check for VK_EXT_debug_utils support
     */
    uint32_t property_count = 0;
    VK_CALL( vkEnumerateInstanceLayerProperties( &property_count, nullptr ),
             "Fail to read count of layer properties of the vulkan instance" );

    std::vector< VkLayerProperties > available_properties( property_count );
    VK_CALL( vkEnumerateInstanceLayerProperties( &property_count, available_properties.data() ),
             "Cannot enumerate layer properties of the vulkan instance" );

    bool layer_found { false };
    const std::string validation_layer_name( khronos_validation_layer_name );
    for( const auto &layer_property: available_properties ) {
        const std::string layer_name( layer_property.layerName );
        if( validation_layer_name == layer_name ) {
            layer_found = true;
            break;
        }
    }

    /* VK_EXT_debug_utils is supported -> enable debug messages
     */
    if( layer_found ) {
        app.vulkan.debug.enable = true;
        app.vulkan.instance.required_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
    }

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
