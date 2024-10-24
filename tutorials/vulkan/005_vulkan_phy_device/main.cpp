/*
    Vulkan 1.3 tutorial
    
    Pick the physical device
 */

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <optional>


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
        GLFWwindow  *window { nullptr }; /* pointer to GLFW window */
    } glfw;
    struct {
        struct {
            bool                     enable    { false }; /* flag that DebugUtils extension is supported by driver */
            VkDebugUtilsMessengerEXT messenger { VK_NULL_HANDLE }; /* DebugUtils messenger */
        } debug;
        struct {
            VkInstance object { VK_NULL_HANDLE }; /* pointer to Vulkan instance */

            std::vector< const char* > required_extensions; /* list of required extensions */
            std::vector< const char* > require_layers;      /* list of require layers */
        } instance;
        struct {
            VkSurfaceKHR object { VK_NULL_HANDLE }; /* pointer to Vulkan surface */
        } surface;
        struct {
            VkPhysicalDevice gpu { VK_NULL_HANDLE }; /* pointer to the physical device, in this case - GPU */
            uint32_t         graph_family_idx;   /* index in the graphical queue of the physical device */
            uint32_t         present_family_idx; /* index of the presentation queue of the physical device */

            std::vector< const char* > require_extensions {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            }; /* list of required extensions */
        } device;
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
        "Vulkan 1.3 - Tutorial - Physical device",
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
    VK_CALL( vkCreateInstance( &vk_instance_create_info, nullptr, &app.vulkan.instance.object ),
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

/* Vulkan Surface
 */

static bool vk_create_surface( vkApp &app ) {
    if( !app.glfw.window )
        return false;
    if( app.vulkan.instance.object == VK_NULL_HANDLE )
        return false;

    /* Vulkan surface is independent from device or window system
     */
    VK_CALL( glfwCreateWindowSurface( app.vulkan.instance.object,
                                      app.glfw.window,
                                      nullptr,
                                     &app.vulkan.surface.object ),
             "Cannot create window surface" );

    return true;
}

static bool vk_cleanup_surface( vkApp &app ) {
    if( app.vulkan.surface.object != VK_NULL_HANDLE ) {
        vkDestroySurfaceKHR( app.vulkan.instance.object,
                             app.vulkan.surface.object,
                             nullptr );
        app.vulkan.surface.object = VK_NULL_HANDLE;
    }

    return true;
}

/* Vulkan Physical device
 */

inline bool vk_test_dev_extensions( vkApp &app, VkPhysicalDevice dev ) {
    if( app.vulkan.instance.object == VK_NULL_HANDLE )
        return false;
    if( dev == VK_NULL_HANDLE )
        return false;

    uint32_t extensions_cout = 0;
    VK_CALL( vkEnumerateDeviceExtensionProperties( dev,
                                                   nullptr,
                                                  &extensions_cout,
                                                   nullptr ),
             "Fail to read amount of extensions supported by the physical device" );

    /* Read all available extensions for the chosen device
     */
    std::vector< VkExtensionProperties > available_extensions( extensions_cout );
    VK_CALL( vkEnumerateDeviceExtensionProperties( dev,
                                                   nullptr,
                                                  &extensions_cout,
                                                   available_extensions.data() ),
             "Cannot enumerate extensions supported by the physical device" );

    /* Easiest way is to fill a set with required extensions
     * and remove supported by device from it
     */
    std::set< std::string > required_extensions (
        app.vulkan.device.require_extensions.begin(),
        app.vulkan.device.require_extensions.end()
    );
    for( const auto &extension: available_extensions )
        required_extensions.erase( std::string( extension.extensionName ) );

    /* all require extensions are supported 
     */
    if( required_extensions.empty() )
        return true;

    return false;
}

inline bool vk_test_dev_families( vkApp &app,
                                  VkPhysicalDevice dev,
                                  uint32_t &graph_family_idx,
                                  uint32_t &present_family_idx ) {
    if( app.vulkan.instance.object == VK_NULL_HANDLE )
        return false;
    if( dev == VK_NULL_HANDLE )
        return false;

    uint32_t queue_families_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties( dev,
                                             &queue_families_count,
                                              nullptr );
    if( !queue_families_count )
        return false;

    /* Read all properties
     */
    std::vector< VkQueueFamilyProperties > queue_family_properties( queue_families_count );
    vkGetPhysicalDeviceQueueFamilyProperties( dev,
                                             &queue_families_count,
                                              queue_family_properties.data() );

    std::optional< uint32_t > graph_family;
    std::optional< uint32_t > present_family;
    for( size_t idx = 0; idx < queue_families_count; ++idx ) {
        /* detect GPU by testing VK_QUEUE_GRAPHICS_BIT
         */
        if( queue_family_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT )
            graph_family = static_cast< uint32_t > ( idx );

        /* detect graphical output by requesting the presentation support from the surface
         */
        VkBool32 presentation_support = VK_FALSE;
        VK_CALL( vkGetPhysicalDeviceSurfaceSupportKHR( dev,
                                                       static_cast< uint32_t > ( idx ),
                                                       app.vulkan.surface.object,
                                                      &presentation_support ),
                 "Fail to read the presentation support for the physical device from the surface" );

        if( presentation_support == VK_TRUE )
            present_family = static_cast< uint32_t > ( idx );

        /* here is a tricky part - in common case this two indexes might be different
         */
        if( graph_family.has_value() && present_family.has_value() )
            break;
    }

    /* nothing was found
     */
    if( !graph_family.has_value() || !present_family.has_value() )
        return false;

    /* save family indexes
     */
    graph_family_idx   = graph_family.value();
    present_family_idx = present_family.value();

    return true;
}

static bool vk_select_phy_device( vkApp &app ) {
    if( app.vulkan.instance.object == VK_NULL_HANDLE )
        return false;

    uint32_t device_count = 0;
    VK_CALL( vkEnumeratePhysicalDevices( app.vulkan.instance.object,
                                        &device_count,
                                         nullptr ),
             "Failed to read amount of physical devices" );

    /* read information about all physical devices in the system
     */
    std::vector< VkPhysicalDevice > available_devices( device_count );
    VK_CALL( vkEnumeratePhysicalDevices( app.vulkan.instance.object,
                                        &device_count,
                                         available_devices.data() ),
             "Cannot enumerate physical devices" );

    /* search for the suitable physical device
     */
    for( const auto &device: available_devices ) {
        /* first check:
         *  confirm that physical device support all required extensions
         */
        if( !vk_test_dev_extensions( app, device ) )
            continue;

        /* second check:
         *  confirm that physical device is GPU and has graphical output
         */
        uint32_t graph_family_idx, present_family_idx;
        if( !vk_test_dev_families( app, device, graph_family_idx, present_family_idx ) )
            continue;

        /* suitable device found
         */
        app.vulkan.device.gpu = device;
        app.vulkan.device.graph_family_idx   = graph_family_idx;
        app.vulkan.device.present_family_idx = present_family_idx;

        break;
    }

    if( app.vulkan.device.gpu == VK_NULL_HANDLE )
        return false;

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

    /* Check VK_EXT_debug_utils support
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
    TUTORIAL_CALL( vk_create_surface( app ) );

    TUTORIAL_CALL( vk_select_phy_device( app ) );

    return true;
}

static bool cleanup_vulkan( vkApp &app ) {
    TUTORIAL_CALL( vk_cleanup_surface( app ) );
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
