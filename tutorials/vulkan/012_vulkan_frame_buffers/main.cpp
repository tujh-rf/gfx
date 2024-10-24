/*
    Vulkan 1.3 tutorial
    
    Frame buffers
 */

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <optional>
#include <algorithm>


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
            VkPhysicalDevice gpu           { VK_NULL_HANDLE }; /* pointer to the physical device, in this case - GPU */
            VkDevice         object        { VK_NULL_HANDLE }; /* pointer to the Vulkan device */
            VkQueue          graph_queue   { VK_NULL_HANDLE }; /* graphical queue of the Vulkan device */
            VkQueue          present_queue { VK_NULL_HANDLE }; /* presentation queue of the Vulkan device */

            uint32_t         graph_family_idx;   /* index in the graphical queue of the physical device */
            uint32_t         present_family_idx; /* index of the presentation queue of the physical device */

            std::vector< const char* > require_extensions {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            }; /* list of required extensions */
        } device;
        struct {
            bool             recreate           { false };                       /* flag to trigger the swapchain recreation */
            VkSwapchainKHR   object             { VK_NULL_HANDLE };              /* pointer to the Vulkan swapchain */
            VkExtent2D       display_size;                                       /* actual display size */
            VkFormat         display_format     { VK_FORMAT_UNDEFINED };         /* actual format of display pixels */
            VkColorSpaceKHR  display_colorspace { VK_COLOR_SPACE_MAX_ENUM_KHR };
            VkPresentModeKHR present_mode       { VK_PRESENT_MODE_MAX_ENUM_KHR };
            VkRenderPass     render_pass        { VK_NULL_HANDLE };

            std::vector< VkImage >       images; /* images where the graphics will render */
            std::vector< VkImageView >   views;  /* views for the images to present on the screen */
            std::vector< VkFramebuffer > frames; /* frame buffers for every image view */
        } swapchain;
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
        "Vulkan 1.3 - Tutorial - Frame Buffers",
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

/* Vulkan device
 */

static bool vk_create_device( vkApp &app ) {
    if( app.vulkan.device.gpu == VK_NULL_HANDLE )
        return false;

    /* store only unique family indexes
     */
    std::set< uint32_t > queue_families {
        app.vulkan.device.graph_family_idx,
        app.vulkan.device.present_family_idx
    };

    /* create queues for each family
     */
    float queue_prio = 1.0f;

    std::vector< VkDeviceQueueCreateInfo > device_queue_create_infos;
    for( uint32_t queue_family: queue_families ) {
        VkDeviceQueueCreateInfo device_queue_create_info {
            .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queue_family,
            .queueCount       = 1,
            .pQueuePriorities = &queue_prio
        };

        device_queue_create_infos.push_back( device_queue_create_info );
    }

    VkDeviceCreateInfo device_create_info {
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount    = static_cast< uint32_t > ( device_queue_create_infos.size() ),
        .pQueueCreateInfos       = device_queue_create_infos.data(),
        .enabledLayerCount       = static_cast< uint32_t > ( app.vulkan.instance.require_layers.size() ),
        .ppEnabledLayerNames     = app.vulkan.instance.require_layers.data(),
        .enabledExtensionCount   = static_cast< uint32_t > ( app.vulkan.device.require_extensions.size() ),
        .ppEnabledExtensionNames = app.vulkan.device.require_extensions.data()
    };

    VK_CALL( vkCreateDevice( app.vulkan.device.gpu,
                            &device_create_info,
                             nullptr,
                            &app.vulkan.device.object ),
             "Cannot create Vulkan device" );

    /* call volk to correct pointers to Vulkan functions
     */
    volkLoadDevice( app.vulkan.device.object );

    /* store Vulkan queues
     */
    vkGetDeviceQueue( app.vulkan.device.object,
                      app.vulkan.device.graph_family_idx,
                      0,
                     &app.vulkan.device.graph_queue );
    vkGetDeviceQueue( app.vulkan.device.object,
                      app.vulkan.device.present_family_idx,
                      0,
                     &app.vulkan.device.present_queue );

    return true;
}

static bool vk_cleanup_device( vkApp &app ) {
    if( app.vulkan.device.object != VK_NULL_HANDLE ) {
        app.vulkan.device.graph_queue   = VK_NULL_HANDLE;
        app.vulkan.device.present_queue = VK_NULL_HANDLE;

        vkDestroyDevice( app.vulkan.device.object, nullptr );
        app.vulkan.device.object = VK_NULL_HANDLE;
    }

    return true;
}

/* Vulkan swapchain
 */

inline VkExtent2D vk_calculate_display_extent( vkApp &app,
                                               VkSurfaceCapabilitiesKHR surf_caps ) {
    VkExtent2D result;
    int frame_width, frame_height;

    /* read the framebuffer size and calculate display width and height for it
     */

    glfwGetFramebufferSize( app.glfw.window, &frame_width, &frame_height );

    result.width  = std::clamp( static_cast< uint32_t > ( frame_width ),
                                surf_caps.minImageExtent.width,
                                surf_caps.maxImageExtent.width );
    result.height = std::clamp( static_cast< uint32_t > ( frame_height ),
                                surf_caps.minImageExtent.height,
                                surf_caps.maxImageExtent.height );

    return result;
}

inline uint32_t vk_calculate_number_swapchain_images( VkSurfaceCapabilitiesKHR surf_caps ) {
    uint32_t result = surf_caps.minImageCount + 1;
    if( surf_caps.maxImageCount && ( result > surf_caps.maxImageCount ) )
        result = surf_caps.maxImageCount;

    return result;
}

inline VkSurfaceFormatKHR vk_select_display_format( vkApp &app,
                                                    VkFormat format ) {
    VkSurfaceFormatKHR wrong_result {
        .format     = VK_FORMAT_UNDEFINED,
        .colorSpace = VK_COLOR_SPACE_MAX_ENUM_KHR
    };

    /* get amount of display format
     */
    uint32_t surf_format_count = 0;
    if( vkGetPhysicalDeviceSurfaceFormatsKHR( app.vulkan.device.gpu,
                                              app.vulkan.surface.object,
                                             &surf_format_count,
                                              nullptr ) != VK_SUCCESS ) {
        std::cerr
            << "VK_CALL error: "
                << "Fail to read amount of surface formats"
                << std::endl;
    }
    if( !surf_format_count ) {
        std::cerr
            << "Surface doesn't support any graphical formats"
                << std::endl;
        return wrong_result;
    }

    /* read display formats and find the index of requested format
     */
    std::vector< VkSurfaceFormatKHR > surf_formats( surf_format_count );
    if( vkGetPhysicalDeviceSurfaceFormatsKHR( app.vulkan.device.gpu,
                                                   app.vulkan.surface.object,
                                                  &surf_format_count,
                                                   surf_formats.data() ) != VK_SUCCESS ) {
        std::cerr
            << "VK_CALL error: "
                << "Cannot read formats of the surface"
                << std::endl;
        return wrong_result;
    }

    uint32_t surf_format_idx;
    for( surf_format_idx = 0; surf_format_idx < surf_format_count; ++surf_format_idx ) {
        if( surf_formats[surf_format_idx].format == format )
            return surf_formats[surf_format_idx];
    }

    std::cerr
        << "VK_CALL error: "
            << "Surface doesn't support desirable format "
            << format
            << std::endl;
    return wrong_result;
}

inline VkPresentModeKHR vk_select_presentation_mode( vkApp &app,
                                                     VkPresentModeKHR mode ) {
    /* get amount of presentation modes
     */
    uint32_t present_mode_count = 0;
    if( vkGetPhysicalDeviceSurfacePresentModesKHR( app.vulkan.device.gpu,
                                                   app.vulkan.surface.object,
                                                  &present_mode_count,
                                                   nullptr ) != VK_SUCCESS ) {
        std::cerr
            << "VK_CALL error: "
                << "Fail to read amount of presentation modes for the physical device"
                << std::endl;
        return VK_PRESENT_MODE_MAX_ENUM_KHR;
    }
    if( !present_mode_count )
        return VK_PRESENT_MODE_MAX_ENUM_KHR;

    /* read presentation modes
     */
    std::vector< VkPresentModeKHR > present_modes( present_mode_count );
    if( vkGetPhysicalDeviceSurfacePresentModesKHR( app.vulkan.device.gpu,
                                                   app.vulkan.surface.object,
                                                  &present_mode_count,
                                                   present_modes.data() ) != VK_SUCCESS ) {
        std::cerr
            << "VK_CALL error: "
                << "Cannot read presentation modes for the physical device"
                << std::endl;
        return VK_PRESENT_MODE_MAX_ENUM_KHR;
    }

    for( const auto &available_mode: present_modes ) {
        if( available_mode == mode )
            return available_mode;
    }

    return VK_PRESENT_MODE_MAX_ENUM_KHR;
}

static bool vk_create_swapchain( vkApp &app ) {
    if( app.vulkan.surface.object == VK_NULL_HANDLE )
        return false;
    if( app.vulkan.device.object == VK_NULL_HANDLE )
        return false;

    VkSurfaceCapabilitiesKHR surf_caps;
    VK_CALL( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( app.vulkan.device.gpu,
                                                        app.vulkan.surface.object,
                                                       &surf_caps ),
             "Cannot read surface capabilities for the physical device" );

    /* calculate display extent
     */
    VkExtent2D display_extent = vk_calculate_display_extent( app, surf_caps );
    app.vulkan.swapchain.display_size = display_extent;

    /* calculate amount of images in the swapchain
     */
    uint32_t image_count = vk_calculate_number_swapchain_images( surf_caps );

    /* check for the support of VK_FORMAT_B8G8R8A8_UNORM display format
     */
    if( app.vulkan.swapchain.display_format == VK_FORMAT_UNDEFINED ) {
        VkSurfaceFormatKHR display_format = vk_select_display_format( app, VK_FORMAT_B8G8R8A8_UNORM );
        if( display_format.format == VK_FORMAT_UNDEFINED )
            return false;

        app.vulkan.swapchain.display_format     = display_format.format;
        app.vulkan.swapchain.display_colorspace = display_format.colorSpace;
    }

    /* check for support of VK_PRESENT_MODE_FIFO_KHR display presentation mode
     */
    if( app.vulkan.swapchain.present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR ) {
        VkPresentModeKHR display_present_mode = vk_select_presentation_mode( app, VK_PRESENT_MODE_FIFO_KHR );
        if( display_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR )
            return false;

        app.vulkan.swapchain.present_mode = display_present_mode;
    }

    /* Create Swapchain
     */
    VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface          = app.vulkan.surface.object,
        .minImageCount    = image_count,
        .imageFormat      = app.vulkan.swapchain.display_format,
        .imageColorSpace  = app.vulkan.swapchain.display_colorspace,
        .imageExtent      = display_extent,
        .imageArrayLayers = 1,
        .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform     = surf_caps.currentTransform,
        .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode      = app.vulkan.swapchain.present_mode,
        .clipped          = VK_TRUE,
        .oldSwapchain     = VK_NULL_HANDLE
    };
    if( app.vulkan.device.graph_family_idx != app.vulkan.device.present_family_idx ) {
        uint32_t family_idxs[] = { app.vulkan.device.graph_family_idx,
                                   app.vulkan.device.present_family_idx };

        swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices   = family_idxs;
    }
    else {
        uint32_t family_idxs[] = { app.vulkan.device.graph_family_idx };

        swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_create_info.queueFamilyIndexCount = 1;
        swapchain_create_info.pQueueFamilyIndices   = family_idxs;
    }

    VK_CALL( vkCreateSwapchainKHR( app.vulkan.device.object,
                                  &swapchain_create_info,
                                   nullptr,
                                  &app.vulkan.swapchain.object ),
             "Cannot create swapchain" );

    return true;
}

static bool vk_cleanup_swapchain( vkApp &app ) {
    if( app.vulkan.swapchain.object != VK_NULL_HANDLE ) {
        vkDestroySwapchainKHR( app.vulkan.device.object,
                               app.vulkan.swapchain.object,
                               nullptr );
        app.vulkan.swapchain.object = VK_NULL_HANDLE;
    }

    return true;
}

/* Vulkan swapchain images
 */
static bool vk_get_swapchain_images( vkApp &app ) {
    if( app.vulkan.swapchain.object == VK_NULL_HANDLE )
        return false;

    uint32_t image_count = 0;
    VK_CALL( vkGetSwapchainImagesKHR( app.vulkan.device.object,
                                      app.vulkan.swapchain.object,
                                     &image_count,
                                      nullptr ),
             "Fail to read amount of images in the swapchain" );

    app.vulkan.swapchain.images.resize( image_count );
    VK_CALL( vkGetSwapchainImagesKHR( app.vulkan.device.object,
                                      app.vulkan.swapchain.object,
                                     &image_count,
                                      app.vulkan.swapchain.images.data() ),
             "Cannot read pointers to images from the swapchain" );

    return true;
}

static bool vk_cleanup_swapchain_images( vkApp &app ) {
    app.vulkan.swapchain.images.clear();

    return true;
}

/* Vulkan image views
 */
static bool vk_create_image_views( vkApp & app ) {
    if( app.vulkan.swapchain.images.empty() )
        return false;

    for( const auto &image: app.vulkan.swapchain.images ) {
        VkImageViewCreateInfo view_create_info {
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image      = image,
            .viewType   = VK_IMAGE_VIEW_TYPE_2D,
            .format     = app.vulkan.swapchain.display_format,
            .components = { .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                            .a = VK_COMPONENT_SWIZZLE_IDENTITY },
            .subresourceRange = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  .baseMipLevel   = 0,
                                  .levelCount     = 1,
                                  .baseArrayLayer = 0,
                                  .layerCount     = 1 }
        };

        VkImageView view = VK_NULL_HANDLE;
        VK_CALL( vkCreateImageView( app.vulkan.device.object,
                                   &view_create_info,
                                    nullptr,
                                   &view ),
                 "Cannot create the view for the image in the swapchain" );

        app.vulkan.swapchain.views.push_back( view );
    }

    return true;
}

static bool vk_cleanup_image_views( vkApp &app ) {
    for( const auto &view: app.vulkan.swapchain.views )
        vkDestroyImageView( app.vulkan.device.object,
                            view,
                            nullptr );
    app.vulkan.swapchain.views.clear();

    return true;
}

/* Vulkan Render Pass
 */

static bool vk_create_render_pass( vkApp &app ) {
    if( app.vulkan.device.object == VK_NULL_HANDLE )
        return false;

    VkAttachmentDescription color_attachment {
        .format         = app.vulkan.swapchain.display_format,
        .samples        = VK_SAMPLE_COUNT_1_BIT,
        .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };
    VkAttachmentReference color_attachment_ref {
        .attachment = 0,
        .layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkSubpassDescription subpass {
        .pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments    = &color_attachment_ref
    };
    VkSubpassDependency subpass_dep {
        .srcSubpass    = VK_SUBPASS_EXTERNAL,
        .dstSubpass    = VK_FALSE,
        .srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_FALSE,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    VkRenderPassCreateInfo render_pass_create_info {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 1,
        .pAttachments    = &color_attachment,
        .subpassCount    = 1,
        .pSubpasses      = &subpass,
        .dependencyCount = 1,
        .pDependencies   = &subpass_dep
    };
    VK_CALL( vkCreateRenderPass( app.vulkan.device.object,
                                &render_pass_create_info,
                                 nullptr,
                                &app.vulkan.swapchain.render_pass ),
        "Cannot create Vulkan render pass" );

    return true;
}

static bool vk_cleanup_render_pass( vkApp &app ) {
    if( app.vulkan.device.object == VK_NULL_HANDLE )
        return false;

    if( app.vulkan.swapchain.render_pass != VK_NULL_HANDLE ) {
        vkDestroyRenderPass( app.vulkan.device.object,
            app.vulkan.swapchain.render_pass,
            nullptr );
        app.vulkan.swapchain.render_pass = VK_NULL_HANDLE;
    }

    return true;
}

/* Vulkan Frame Buffers
 */

static bool vk_create_frame_buffers( vkApp &app ) {
    for( const auto &image_view: app.vulkan.swapchain.views ) {
        VkImageView attachments[] = { image_view };
        VkFramebufferCreateInfo fbuffer_create_info {
            .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass      = app.vulkan.swapchain.render_pass,
            .attachmentCount = 1,
            .pAttachments    = attachments,
            .width           = app.vulkan.swapchain.display_size.width,
            .height          = app.vulkan.swapchain.display_size.height,
            .layers          = 1
        };
        VkFramebuffer fbuffer = VK_NULL_HANDLE;

        VK_CALL( vkCreateFramebuffer( app.vulkan.device.object,
                                     &fbuffer_create_info,
                                      nullptr,
                                     &fbuffer ),
           "Cannot create frame buffer for the image view" );
        app.vulkan.swapchain.frames.push_back( fbuffer );
    }

    return true;
}

static bool vk_cleanup_frame_buffers( vkApp &app ) {
    for( const auto &fbuffer: app.vulkan.swapchain.frames )
        vkDestroyFramebuffer( app.vulkan.device.object,
                              fbuffer,
                              nullptr );
    app.vulkan.swapchain.frames.clear();
    return true;
}

/* Recreate Swapchain
 */

static bool vk_recreate_swapchain( vkApp &app ) {
    int window_width, window_height;

    do {
        glfwGetFramebufferSize( app.glfw.window,
                               &window_width,
                               &window_height );
        glfwWaitEvents();
    } while( window_width == 0 || window_height == 0  );

    vkDeviceWaitIdle( app.vulkan.device.object );

    /* remove all previous objects
     */
    TUTORIAL_CALL( vk_cleanup_frame_buffers( app ) );
    TUTORIAL_CALL( vk_cleanup_render_pass( app ) );
    TUTORIAL_CALL( vk_cleanup_image_views( app ) );
    TUTORIAL_CALL( vk_cleanup_swapchain_images( app ) );
    TUTORIAL_CALL( vk_cleanup_swapchain( app ) );

    /* create new objects
     */
    TUTORIAL_CALL( vk_create_swapchain( app ) );
    TUTORIAL_CALL( vk_get_swapchain_images( app ) );
    TUTORIAL_CALL( vk_create_image_views( app ) );
    TUTORIAL_CALL( vk_create_render_pass( app ) );
    TUTORIAL_CALL( vk_create_frame_buffers( app ) );

    app.vulkan.swapchain.recreate = false;

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

    TUTORIAL_CALL( vk_create_device( app ) );
    TUTORIAL_CALL( vk_create_swapchain( app ) );

    TUTORIAL_CALL( vk_get_swapchain_images( app ) );

    TUTORIAL_CALL( vk_create_image_views( app ) );
    TUTORIAL_CALL( vk_create_render_pass( app ) );
    TUTORIAL_CALL( vk_create_frame_buffers( app ) );

    app.vulkan.swapchain.recreate = false;

    return true;
}

static bool cleanup_vulkan( vkApp &app ) {
    TUTORIAL_CALL( vk_cleanup_frame_buffers( app ) );
    TUTORIAL_CALL( vk_cleanup_render_pass( app ) );
    TUTORIAL_CALL( vk_cleanup_image_views( app ) );
    TUTORIAL_CALL( vk_cleanup_swapchain_images( app ) );
    TUTORIAL_CALL( vk_cleanup_swapchain( app ) );
    TUTORIAL_CALL( vk_cleanup_device( app ) );
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
