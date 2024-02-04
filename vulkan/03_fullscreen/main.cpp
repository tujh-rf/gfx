/*
    Demonstration how to initialize Vulkan 1.3
 */

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

/* definition says GLFW do not create Vulkan prototypes - will be done by volk */
#define GLFW_INCLUDE_VULKAN
#define VK_NO_PROTOTYPES
#include <GLFW/glfw3.h>
#include <volk.h>

#define VK_CALL( func, err_msg )                    \
    {                                               \
        if( VK_SUCCESS != func )                    \
            throw std::runtime_error( err_msg );    \
    }

/* GLFW3 */
struct glfw_instance {
    bool            init        { false };      //!< flag of GLFW3 initialization
    GLFWwindow     *window      { nullptr };    //!< pointer to GLFW3 window
};

/* Vulkan instance */
struct vk_instance {
    VkInstance                  instance    { VK_NULL_HANDLE }; //!< pointer to Vulkan instance
    VkDebugUtilsMessengerEXT    messenger   { VK_NULL_HANDLE }; //!< pointer to Vulkan debug messenger extension object
};

/* Vulkan device */
struct vk_device {
    VkPhysicalDevice    gpu         { VK_NULL_HANDLE }; //!< pointer to the physical device - GPU
    VkDevice            vk_dev      { VK_NULL_HANDLE }; //!< pointer to Vulkan device object
    VkSurfaceKHR        surface     { VK_NULL_HANDLE }; //!< pointer to output surface

    uint32_t            graph_family_idx;               //!< index of graphical queue of the physical device
    uint32_t            pres_family_idx;                //!< index of presentation queue of the physical device

    VkQueue             graph_queue { VK_NULL_HANDLE }; //!< graphical queue of the Vulkan device
    VkQueue             pres_queue  { VK_NULL_HANDLE }; //!< presentation queue of the Vulkan device
};

/* Vulkan surface */
struct vk_swapchain {
    bool                            recreate        { false };                  //!< flag to trigger swapchain recreation
    VkSwapchainKHR                  swapchain       { VK_NULL_HANDLE };         //!< Vulkan swapchain
    VkFormat                        display_format  { VK_FORMAT_UNDEFINED };    //!< actual format of display
    VkExtent2D                      display_size;                               //!< actual size of display
    std::vector< VkImage >          images;                                     //!< images for render
    std::vector< VkImageView >      views;                                      //!< view of images for render
    std::vector< VkFramebuffer >    frames;                                     //!< frame buffers for every image view
};

/* Vulkan synchronization */
struct vk_sync {
    VkSemaphore image_available     { VK_NULL_HANDLE }; //!< semaphore to indicate image availability 
    VkSemaphore rendering_finished  { VK_NULL_HANDLE }; //!< semaphore to indicate end of render pass
    VkFence     gpu_fence           { VK_NULL_HANDLE }; //!< GPU free fence
};

/* Vulkan pipeline */
struct vk_pipeline {
    VkPipeline          pipe    { VK_NULL_HANDLE }; //!< Vulkan pipeline
    VkPipelineCache     cache   { VK_NULL_HANDLE }; //!< Vulkan pipeline cache
    VkPipelineLayout    layout  { VK_NULL_HANDLE }; //!< Vulkan pipeline layout
};

/* Vulkan render */
struct vk_render {
    VkRenderPass                    pass    { VK_NULL_HANDLE }; //!< render pass object
    VkCommandPool                   pool    { VK_NULL_HANDLE }; //!< memory pool for command buffers
    std::vector< VkCommandBuffer >  cmds;                       //!< array of command buffers
};

/* Application */
struct vkApp {
    glfw_instance   glfw;       //!< GLFW3 instance object
    vk_instance     vk;         //!< Vulkan instance object
    vk_device       device;     //!< Vulkan device object and parameters
    vk_swapchain    swap;       //!< Vulkan swapchain
    vk_sync         sync;       //!< Vulkan synchronization
    vk_pipeline     pipeline;   //!< Vulkan pipeline object
    vk_render       render;     //!< Vulkan render objects

    std::vector< const char* > vk_required_instance_extensions;
    std::vector< const char* > vk_required_layers;
    std::vector< const char* > vk_required_device_extensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
};

/* global variables */
vkApp g_app;

/* GLFW error handling */
static void glfw_error_callback( int error, const char* description ) {
    /* print error description */
    std::cerr << "GLFW Error: "
              << description
              << std::endl;
}

/* Vulkan error handling */
static VKAPI_ATTR VkBool32 VKAPI_CALL vk_error_callback(
          VkDebugUtilsMessageSeverityFlagBitsEXT    msg_severity,
          VkDebugUtilsMessageTypeFlagsEXT           msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT     *pcallback_data,
          void                                     *puser_data
) {
    std::cerr << "Vulkan error: "
              << pcallback_data->pMessage
              << std::endl;

    return VK_FALSE; /* should always be VK_FALSE */
}

/* key pressing
    ESC - close application
 */
static void key_callback( GLFWwindow *window, int key, int scancode, int action, int mods ) {
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GLFW_TRUE );
}

/* trigger swapchain and frame buffers recreation */
static void framebuffer_resize_callback( GLFWwindow *window, int width, int height ) {
    /* don't need to save width and height now because both need to be compared
       with Vulkan surface capabilities -> will be don during swapchain creation */
    g_app.swap.recreate = true;
}

/*
    Vulkan
 */

/* Vulkan instance */
static void vk_create_instance() {
    if( g_app.vk_required_instance_extensions.empty() )
        throw std::runtime_error( "Vulkan instance cannot be created" );

    /* application information */
    VkApplicationInfo app_info {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,   /* type of the structure MUST be set ALWAYS */
        .pApplicationName   = "Vulkan application",
        .applicationVersion = VK_MAKE_API_VERSION( 0, 1, 0, 0 ),
        .pEngineName        = "Vulkan samples",
        .engineVersion      = VK_MAKE_API_VERSION( 0, 1, 0, 0 ),
        .apiVersion         = VK_API_VERSION_1_3    /* Vulkan v1.3 */
    };

#ifdef _DEBUG
    g_app.vk_required_layers.push_back( "VK_LAYER_KHRONOS_validation" );

    /* capture only errors */
    VkDebugUtilsMessengerCreateInfoEXT debug_util_info {
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT              \
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT    \
                                | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &vk_error_callback
    };

#endif /* _DEBUG */

    /* Vulkan instance */
    VkInstanceCreateInfo instance_create_info {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#ifdef _DEBUG
        .pNext                   = static_cast< void* > ( &debug_util_info ),   /* link to VkDebugUtilsMessengerCreateInfoEXT */
#endif /* _DEBUG */
        .pApplicationInfo        = &app_info,                                   /* link ot VkApplicationInfo */
        .enabledLayerCount       = static_cast< uint32_t > ( g_app.vk_required_layers.size() ),
        .ppEnabledLayerNames     = g_app.vk_required_layers.data(),
        .enabledExtensionCount   = static_cast< uint32_t > ( g_app.vk_required_instance_extensions.size() ),
        .ppEnabledExtensionNames = g_app.vk_required_instance_extensions.data(),
    };

    /* create Vulkan instance */
    VK_CALL( vkCreateInstance( &instance_create_info, nullptr, &g_app.vk.instance ),
             "Cannot create Vulkan instance" );

    /* initialize pointers to functions for this instance */
    volkLoadInstanceOnly( g_app.vk.instance );

#ifdef _DEBUG

    /* create debug messenger object */
    VK_CALL( vkCreateDebugUtilsMessengerEXT( g_app.vk.instance, &debug_util_info, nullptr, &g_app.vk.messenger ),
             "Cannot create Vulkan debug messenger" );

#endif /* _DEBUG */

    /* in Vulkan device and surface are not linked directly */
    VK_CALL( glfwCreateWindowSurface( g_app.vk.instance, g_app.glfw.window, nullptr, &g_app.device.surface ),
             "Cannot create window surface" );
}
static void vk_cleanup_instance() {
    if( g_app.device.surface != VK_NULL_HANDLE ) {
        vkDestroySurfaceKHR( g_app.vk.instance, g_app.device.surface, nullptr );
        g_app.device.surface = VK_NULL_HANDLE;
    }
    if( g_app.vk.messenger != VK_NULL_HANDLE ) {
        vkDestroyDebugUtilsMessengerEXT( g_app.vk.instance, g_app.vk.messenger, nullptr );
        g_app.vk.messenger = VK_NULL_HANDLE;
    }
    if( g_app.vk.instance != VK_NULL_HANDLE ) {
        vkDestroyInstance( g_app.vk.instance, nullptr );
        g_app.vk.instance = VK_NULL_HANDLE;
    }
}

/* Vulkan device */
static void vk_create_device() {
    if( g_app.vk.instance == VK_NULL_HANDLE )
        throw std::runtime_error( "Vulkan instance doesn't exist" );

    uint32_t phy_dev_count = 0;
    VK_CALL( vkEnumeratePhysicalDevices( g_app.vk.instance, &phy_dev_count, nullptr ),
             "Failed to read the count of physical devices" );

    std::vector< VkPhysicalDevice > available_phy_dev( phy_dev_count );
    VK_CALL( vkEnumeratePhysicalDevices( g_app.vk.instance, &phy_dev_count, available_phy_dev.data() ),
             "Cannot enumerate physical devices" );

    /* search for suitable physical device - GPU */
    for( const auto &phy_dev: available_phy_dev ) {
        /* 1.
            find device with all required extensions */

        uint32_t ext_prop_count = 0;
        VK_CALL( vkEnumerateDeviceExtensionProperties( phy_dev, nullptr, &ext_prop_count, nullptr ),
                 "Failed to read the count of extension properties" );

        std::vector< VkExtensionProperties > available_extensions( ext_prop_count );
        VK_CALL( vkEnumerateDeviceExtensionProperties( phy_dev, nullptr, &ext_prop_count, available_extensions.data() ),
                 "Cannot enumerate extension properties for the physical device" );

        std::set< std::string > required_extensions( g_app.vk_required_device_extensions.begin(), g_app.vk_required_device_extensions.end() );
        for( const auto &extension: available_extensions )
            required_extensions.erase( std::string( extension.extensionName ) );

        if( !required_extensions.empty() )
            continue;

        /* 2.
            search for graphics and presentation families of the phy. device,
            in common case this two families might not be equal */

        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( phy_dev, &queue_family_count, nullptr );

        if( !queue_family_count )
            continue;

        std::vector< VkQueueFamilyProperties > queue_family_prop( queue_family_count );
        vkGetPhysicalDeviceQueueFamilyProperties( phy_dev, &queue_family_count, queue_family_prop.data() );

        std::optional< uint32_t > graph_family;
        std::optional< uint32_t > pres_family;
        for( size_t queue_family_idx = 0; queue_family_idx < queue_family_count; ++queue_family_idx ) {
            /* VK_QUEUE_GRAPHICS_BIT - GPU */
            if( queue_family_prop[queue_family_idx].queueFlags & VK_QUEUE_GRAPHICS_BIT )
                graph_family = static_cast< uint32_t > ( queue_family_idx );

            VkBool32 presentation_support = VK_FALSE;
            VK_CALL( vkGetPhysicalDeviceSurfaceSupportKHR( phy_dev, static_cast< uint32_t > ( queue_family_idx ), g_app.device.surface, &presentation_support ),
                     "Failed to read surface support of the physical device" );

            /* surface support presentation - graphical output */
            if( presentation_support == VK_TRUE )
                pres_family = static_cast< uint32_t > ( queue_family_idx );

            /* device with all requirements found */
            if( graph_family.has_value() && pres_family.has_value() )
                break;
        }

        /* current physical device doesn't support graphics and presentation families */
        if( ( !graph_family.has_value() ) || ( !pres_family.has_value() ) )
            continue;

        /* 3.
            suitable physical device found - create Vulkan device */

        float queue_prio = 1.0f;

        uint32_t graph_family_idx = graph_family.value();
        uint32_t pres_family_idx  = pres_family.value();

        /* small trick to collect only unique indexes */
        std::set< uint32_t > unique_queue_families { graph_family_idx, pres_family_idx };

        std::vector< VkDeviceQueueCreateInfo > dev_queue_create_infos;
        for( uint32_t queue_family: unique_queue_families ) {
            VkDeviceQueueCreateInfo dev_queue_info {
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queue_family,
                .queueCount       = 1,
                .pQueuePriorities = &queue_prio
            };

            dev_queue_create_infos.push_back( dev_queue_info );
        }

        VkDeviceCreateInfo dev_create_info {
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount    = static_cast< uint32_t > ( dev_queue_create_infos.size() ),
            .pQueueCreateInfos       = dev_queue_create_infos.data(),
            .enabledLayerCount       = static_cast< uint32_t > ( g_app.vk_required_layers.size() ),
            .ppEnabledLayerNames     = g_app.vk_required_layers.data(),
            .enabledExtensionCount   = static_cast< uint32_t > ( g_app.vk_required_device_extensions.size() ),
            .ppEnabledExtensionNames = g_app.vk_required_device_extensions.data()
        };

        VK_CALL( vkCreateDevice( phy_dev, &dev_create_info, nullptr, &g_app.device.vk_dev ),
                 "Cannot create Vulkan device" );

        /* load pointers to functions for this Vulkan device */
        volkLoadDevice( g_app.device.vk_dev );

        /* save suitable device */
        g_app.device.gpu = phy_dev;

        g_app.device.graph_family_idx = graph_family_idx;
        g_app.device.pres_family_idx  = pres_family_idx;

        vkGetDeviceQueue( g_app.device.vk_dev, graph_family_idx, 0, &g_app.device.graph_queue );
        vkGetDeviceQueue( g_app.device.vk_dev, pres_family_idx, 0, &g_app.device.pres_queue );

        break;
    }
}
static void vk_cleanup_device() {
    if( g_app.device.vk_dev != VK_NULL_HANDLE ) {
        g_app.device.graph_queue = VK_NULL_HANDLE;
        g_app.device.pres_queue  = VK_NULL_HANDLE;

        vkDestroyDevice( g_app.device.vk_dev, nullptr );
        g_app.device.vk_dev = VK_NULL_HANDLE;
    }
}

/* Vulkan Swapchain */
static void vk_create_swapchain() {
    if( g_app.device.surface == VK_NULL_HANDLE )
        throw std::runtime_error( "Vulkan surface doesn't exist" );

    if( g_app.device.gpu == VK_NULL_HANDLE )
        throw std::runtime_error( "Suitable physical device was not found" );

    if( g_app.device.vk_dev == VK_NULL_HANDLE )
        throw std::runtime_error( "Vulkan device doesn't exist" );

    VkSurfaceCapabilitiesKHR surf_caps {};
    VK_CALL( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( g_app.device.gpu, g_app.device.surface, &surf_caps ),
             "Cannot read surface capabilities for choosen physical device" );

    VkExtent2D display_extent;
    int width, height;
    glfwGetFramebufferSize( g_app.glfw.window, &width, &height );

    display_extent.width  = std::clamp( static_cast< uint32_t > ( width ),
                                        surf_caps.minImageExtent.width,
                                        surf_caps.maxImageExtent.width );
    display_extent.height = std::clamp( static_cast< uint32_t > ( height ),
                                        surf_caps.minImageExtent.height,
                                        surf_caps.maxImageExtent.height );

    /* calculate the number of images for the swapchain */
    uint32_t image_count = surf_caps.minImageCount + 1;
    if( surf_caps.maxImageCount && ( image_count > surf_caps.maxImageCount ) )
        image_count = surf_caps.maxImageCount;

    /* determinate display format */
    uint32_t surf_formats_count = 0;
    VK_CALL( vkGetPhysicalDeviceSurfaceFormatsKHR( g_app.device.gpu, g_app.device.surface, &surf_formats_count, nullptr ),
             "Failed to read the count of surface formats" );
    if( !surf_formats_count )
        throw std::runtime_error( "Surface doesn't support any graphical formats" );

    std::vector< VkSurfaceFormatKHR > surf_formats( surf_formats_count );
    VK_CALL( vkGetPhysicalDeviceSurfaceFormatsKHR( g_app.device.gpu, g_app.device.surface, &surf_formats_count, surf_formats.data() ),
             "Cannot read formats of the surface" );

    uint32_t surf_format_idx;
    for( surf_format_idx = 0; surf_format_idx < surf_formats_count; ++surf_format_idx) {
        if( surf_formats[surf_format_idx].format == VK_FORMAT_B8G8R8A8_UNORM )
            break;
    }
    if( surf_format_idx == surf_formats_count )
        throw std::runtime_error( "Surface doesn't support VK_FORMAT_B8G8R8A8_UNORM format" );

    VkSurfaceFormatKHR display_format = surf_formats[surf_format_idx];

    /* determinate display presentation mode */
    uint32_t surf_pres_modes_count = 0;
    VK_CALL( vkGetPhysicalDeviceSurfacePresentModesKHR( g_app.device.gpu, g_app.device.surface, &surf_pres_modes_count, nullptr ),
             "Failed to read present modes of the device" );
    if( !surf_pres_modes_count )
        throw std::runtime_error( "Surface doesn't support present modes" );

    std::vector< VkPresentModeKHR > surf_present_modes( surf_pres_modes_count );
    VK_CALL( vkGetPhysicalDeviceSurfacePresentModesKHR( g_app.device.gpu, g_app.device.surface, &surf_pres_modes_count, surf_present_modes.data() ),
             "Failed to read present modes of the device" );

    VkPresentModeKHR display_present_mode = VK_PRESENT_MODE_MAX_ENUM_KHR;
    for( const auto &available_present_mode: surf_present_modes ) {
        if( available_present_mode == VK_PRESENT_MODE_FIFO_KHR ) {
            display_present_mode = available_present_mode;
            break;
        }
    }
    if( display_present_mode == VK_PRESENT_MODE_MAX_ENUM_KHR )
        throw std::runtime_error( "Surface doesn't support VK_PRESENT_MODE_FIFO_KHR mode" );

    VkSwapchainCreateInfoKHR swapchain_create_info {
        .sType              = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface            = g_app.device.surface,
        .minImageCount      = image_count,
        .imageFormat        = display_format.format,
        .imageColorSpace    = display_format.colorSpace,
        .imageExtent        = display_extent,
        .imageArrayLayers   = 1,
        .imageUsage         = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .preTransform       = surf_caps.currentTransform,
        .compositeAlpha     = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode        = display_present_mode,
        .clipped            = VK_TRUE,
        .oldSwapchain       = VK_NULL_HANDLE
    };

    /* image sharing mode is different when graph and present families are the same */
    if( g_app.device.graph_family_idx != g_app.device.pres_family_idx ) {
        uint32_t queue_family_idxs[] = { g_app.device.graph_family_idx, g_app.device.pres_family_idx };

        swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_create_info.queueFamilyIndexCount = 2;
        swapchain_create_info.pQueueFamilyIndices   = queue_family_idxs;
    } else
        swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CALL( vkCreateSwapchainKHR( g_app.device.vk_dev, &swapchain_create_info, nullptr, &g_app.swap.swapchain ),
             "Cannot create swapchain" );

    g_app.swap.display_format = display_format.format;
    g_app.swap.display_size   = display_extent;

    /* get swapchain images */
    uint32_t swapchain_images_count = 0;
    VK_CALL( vkGetSwapchainImagesKHR( g_app.device.vk_dev, g_app.swap.swapchain, &swapchain_images_count, nullptr ),
             "Failed to read count on images in the swapchain" );

    g_app.swap.images.resize( swapchain_images_count );
    VK_CALL( vkGetSwapchainImagesKHR( g_app.device.vk_dev, g_app.swap.swapchain, &swapchain_images_count, g_app.swap.images.data() ),
             "Failed to read count on images in the swapchain" );

    /* create image view for every image */
    for( const auto &image: g_app.swap.images ) {
        VkImageViewCreateInfo view_create_info {
            .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image      = image,
            .viewType   = VK_IMAGE_VIEW_TYPE_2D,
            .format     = display_format.format,
            .components = {
                .r  = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g  = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b  = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a  = VK_COMPONENT_SWIZZLE_IDENTITY
            },
            .subresourceRange = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
                .levelCount     = 1,
                .baseArrayLayer = 0,
                .layerCount     = 1
            }
        };

        VkImageView image_view = VK_NULL_HANDLE;
        VK_CALL( vkCreateImageView( g_app.device.vk_dev, &view_create_info, nullptr, &image_view ),
                 "Cannot create image view for the image in swapchain" );

        g_app.swap.views.push_back( image_view );
    }

    /* render pass object is needed for frame buffers */
    VkAttachmentDescription color_attachment {
        .format         = display_format.format,
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
        .pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount   = 1,
        .pColorAttachments      = &color_attachment_ref
    };
    VkSubpassDependency subpass_dep {
        .srcSubpass     = VK_SUBPASS_EXTERNAL,
        .dstSubpass     = 0,
        .srcStageMask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask   = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask  = 0,
        .dstAccessMask  = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };
    VkRenderPassCreateInfo pass_create_info {
        .sType  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount    = 1,
        .pAttachments       = &color_attachment,
        .subpassCount       = 1,
        .pSubpasses         = &subpass,
        .dependencyCount    = 1,
        .pDependencies      = &subpass_dep
    };
    VK_CALL( vkCreateRenderPass( g_app.device.vk_dev, &pass_create_info, nullptr, &g_app.render.pass ),
             "Cannot create Vulkan render pass" );

    /* create frame buffer for every image view */
    for( const auto &image_view: g_app.swap.views ) {
        VkImageView attachments[] = { image_view };

        VkFramebufferCreateInfo fbuffer_create_info {
            .sType              = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass         = g_app.render.pass,
            .attachmentCount    = 1,
            .pAttachments       = attachments,
            .width              = display_extent.width,
            .height             = display_extent.height,
            .layers             = 1
        };
        VkFramebuffer fbuffer = VK_NULL_HANDLE;
        VK_CALL( vkCreateFramebuffer( g_app.device.vk_dev, &fbuffer_create_info, nullptr, &fbuffer ),
                 "Cannot create frame buffer for the image view" );

        g_app.swap.frames.push_back( fbuffer );
    }

    g_app.swap.recreate = false;
}
static void vk_cleanup_swapchain() {
    for( const auto &fbufer: g_app.swap.frames )
        vkDestroyFramebuffer( g_app.device.vk_dev, fbufer, nullptr );
    g_app.swap.frames.clear();

    if( g_app.render.pass != VK_NULL_HANDLE ) {
        vkDestroyRenderPass( g_app.device.vk_dev, g_app.render.pass, nullptr );
        g_app.render.pass = VK_NULL_HANDLE;
    }

    for( const auto &image_view: g_app.swap.views )
        vkDestroyImageView( g_app.device.vk_dev, image_view, nullptr );
    g_app.swap.views.clear();

    if( g_app.swap.swapchain != VK_NULL_HANDLE ) {
        vkDestroySwapchainKHR( g_app.device.vk_dev, g_app.swap.swapchain, nullptr );
        g_app.swap.swapchain = VK_NULL_HANDLE;
    }
}
static void vk_recreate_swapchain() {
    int window_width, window_height;
    do
    {
        glfwGetFramebufferSize( g_app.glfw.window, &window_width, &window_height );
        glfwWaitEvents();
    } while (window_width == 0 || window_height == 0);

    vkDeviceWaitIdle( g_app.device.vk_dev );

    vk_cleanup_swapchain();
    vk_create_swapchain();
}

/* Vulkan synchronization */
static void vk_create_sync() {
    if( g_app.device.vk_dev == VK_NULL_HANDLE )
        throw std::runtime_error( "Vulkan device doesn't exist" );

    VkSemaphoreCreateInfo semaphore_create_info {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    VK_CALL( vkCreateSemaphore( g_app.device.vk_dev, &semaphore_create_info, nullptr, &g_app.sync.image_available ),
             "Cannot create semaphore to indicate availability of the surface image" );
    VK_CALL( vkCreateSemaphore( g_app.device.vk_dev, &semaphore_create_info, nullptr, &g_app.sync.rendering_finished ),
             "Cannot create semaphore to indicate the end of the frame rendering" );

    VkFenceCreateInfo fence_create_info {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    VK_CALL( vkCreateFence( g_app.device.vk_dev, &fence_create_info, nullptr, &g_app.sync.gpu_fence ),
             "Cannot create Vulkan fence for device object" );
}
static void vk_cleanup_sync() {
    vkDestroyFence( g_app.device.vk_dev, g_app.sync.gpu_fence, nullptr );
    g_app.sync.gpu_fence = VK_NULL_HANDLE;

    vkDestroySemaphore( g_app.device.vk_dev, g_app.sync.rendering_finished, nullptr );
    g_app.sync.rendering_finished = VK_NULL_HANDLE;

    vkDestroySemaphore( g_app.device.vk_dev, g_app.sync.image_available, nullptr );
    g_app.sync.image_available = VK_NULL_HANDLE;
}

/* Vulkan command buffer */
static void vk_create_cmd_buffers() {
    VkCommandPoolCreateInfo pool_create_info {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags              = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex   = g_app.device.graph_family_idx
    };
    VK_CALL( vkCreateCommandPool( g_app.device.vk_dev, &pool_create_info, nullptr, &g_app.render.pool ),
             "Cannot create command pool for the device" );

    VkCommandBufferAllocateInfo allocate_info {
        .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool        = g_app.render.pool,
        .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast< uint32_t > ( g_app.swap.images.size() )
    };
    g_app.render.cmds.resize( g_app.swap.images.size() );
    VK_CALL( vkAllocateCommandBuffers( g_app.device.vk_dev, &allocate_info, g_app.render.cmds.data() ),
             "Cannot allocate command buffers" );
}
static void vk_cleanup_cmd_buffers() {
    vkFreeCommandBuffers( g_app.device.vk_dev, g_app.render.pool, static_cast< uint32_t > ( g_app.swap.images.size() ), g_app.render.cmds.data() );
    g_app.render.cmds.clear();

    vkDestroyCommandPool( g_app.device.vk_dev, g_app.render.pool, nullptr );
    g_app.render.pool = VK_NULL_HANDLE;
}

/* record command buffer each frame */
static void vk_record_cmd_buffer( const size_t idx ) {
    VkClearColorValue clear_color = {
        { 0.0f, 0.3f, 0.6f, 1.0f }
    };
    VkImageSubresourceRange subres_range {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = 1,
        .baseArrayLayer = 0,
        .layerCount     = 1
    };
    VkCommandBufferBeginInfo cmd_begin_info {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    VkImageMemoryBarrier present_to_clear_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = g_app.device.pres_family_idx,
        .dstQueueFamilyIndex = g_app.device.pres_family_idx,
        .image = g_app.swap.images[idx],
        .subresourceRange = subres_range
    };
    VkImageMemoryBarrier clear_to_preset_barrier {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .srcQueueFamilyIndex = g_app.device.pres_family_idx,
        .dstQueueFamilyIndex = g_app.device.pres_family_idx,
        .image = g_app.swap.images[idx],
        .subresourceRange = subres_range
    };
    VkViewport viewport {
        .x = 0,
        .y = 0,
        .width = static_cast< float > ( g_app.swap.display_size.width ),
        .height = static_cast< float > ( g_app.swap.display_size.height ),
        .minDepth = 0.0f,
        .maxDepth = 1.0f
    };
    VkRect2D scissor {
        .offset {
            .x = 0,
            .y = 0
        },
        .extent = g_app.swap.display_size
    };
    VK_CALL( vkBeginCommandBuffer( g_app.render.cmds[idx], &cmd_begin_info ),
                "Cannot start record to the command buffer" );

        vkCmdSetViewport( g_app.render.cmds[idx], 0, 1, &viewport );
        vkCmdSetScissor( g_app.render.cmds[idx], 0, 1, &scissor );

        vkCmdPipelineBarrier( g_app.render.cmds[idx],
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              0,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              1,
                             &present_to_clear_barrier );
        vkCmdClearColorImage( g_app.render.cmds[idx],
                              g_app.swap.images[idx],
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                             &clear_color,
                              1,
                             &subres_range );
        vkCmdPipelineBarrier( g_app.render.cmds[idx],
                              VK_PIPELINE_STAGE_TRANSFER_BIT,
                              VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                              0,
                              0,
                              nullptr,
                              0,
                              nullptr,
                              1,
                             &clear_to_preset_barrier );

    VK_CALL( vkEndCommandBuffer( g_app.render.cmds[idx] ),
                "Cannot finish record to the command buffer" );
}

/*
    Sample
 */

/* initialize GLFW */
static void init_glfw() {
    /* setup error callback first to print all possible errors */
    glfwSetErrorCallback( glfw_error_callback );

    if( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "Cannot initialize GLFW" );

    g_app.glfw.init = true;

    /* prepare list of required Vulkan extensions */
    uint32_t glfw_required_extensions_count = 0;
    const char **glfw_required_extensions = glfwGetRequiredInstanceExtensions( &glfw_required_extensions_count );

    for( uint32_t i = 0; i < glfw_required_extensions_count; ++i )
        g_app.vk_required_instance_extensions.push_back( glfw_required_extensions[i] );
}

/* initialize render target window */
static void init_window() {
    if( !g_app.glfw.init )
        throw std::runtime_error( "GLFW is not initialized" );

    /* don't need to create OpenGL context */
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    /* read current screen resolution */
    GLFWmonitor *primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *current_monitore_mode = glfwGetVideoMode( primary_monitor );
    int window_width = current_monitore_mode->width;
    int window_height = current_monitore_mode->height;

    /* create render target window */
    g_app.glfw.window = glfwCreateWindow(
        window_width,
        window_height,
        "Vulkan 1.3 - Full screen application", /* window title */
        primary_monitor,                        /* actual monitor to switch on full screen */
        nullptr                                 /* window to share content with */
    );

    if( !g_app.glfw.window )
        throw std::runtime_error( "Cannot create the window object" );

    /* setup keyboard callback */
    glfwSetKeyCallback( g_app.glfw.window, key_callback );
}

/* initialize Vulkan */
static void init_vulkan() {
    if ( !g_app.glfw.window )
        throw std::runtime_error( "GLFW window is not created" );

    /* load Vulkan via Volk */
    VK_CALL( volkInitialize(), "Cannot initialize Vulkan loader" );

    /* in examples, debug extension is required - IRL this is needed only in debug builds
        search for validation layer extension
     */
    uint32_t property_count = 0;
    VK_CALL( vkEnumerateInstanceLayerProperties( &property_count, nullptr ), "Failed to read the count of layer properties" );

    std::vector< VkLayerProperties > available_properties( property_count );
    VK_CALL( vkEnumerateInstanceLayerProperties( &property_count, available_properties.data() ), "Cannot enumerate layer properties" );

#ifdef _DEBUG

    bool layer_found = false;
    const std::string validation_layer_name( "VK_LAYER_KHRONOS_validation" );
    for( const auto &layer_property: available_properties ) {
        const std::string layer_name( layer_property.layerName );
        if( validation_layer_name == layer_name ) {
            layer_found = true;
            break;
        }
    }
    if( !layer_found )
        throw std::runtime_error( "Khronos validation layer doesn't found" );

    /* add debug extension to the list of required extensions */
    g_app.vk_required_instance_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );

#endif /* _DEBUG */

    vk_create_instance();
    vk_create_device();
    vk_create_swapchain();
    vk_create_sync();
    vk_create_cmd_buffers();
}

/* cleanup Vulkan */
static void cleanup_vulkan() {
    if( g_app.device.vk_dev != VK_NULL_HANDLE )
        VK_CALL( vkDeviceWaitIdle( g_app.device.vk_dev ),
                 "Vulkan device wait failed" );

    vk_cleanup_cmd_buffers();
    vk_cleanup_sync();
    vk_cleanup_swapchain();
    vk_cleanup_device();
    vk_cleanup_instance();
}

/* clean GLFW */
static void cleanup_glfw() {
    if( g_app.glfw.window ) {
        glfwDestroyWindow( g_app.glfw.window );
        g_app.glfw.window = nullptr;
    }
    if( g_app.glfw.init ) {
        glfwTerminate();
        g_app.glfw.init = false;
    }
}

/* draw the scene */
static void draw() {
    VK_CALL( vkResetFences( g_app.device.vk_dev, 1, &g_app.sync.gpu_fence ),
             "Failed to reset GPU fence" );

    uint32_t image_idx;
    VkResult res = vkAcquireNextImageKHR( g_app.device.vk_dev, g_app.swap.swapchain, UINT64_MAX, g_app.sync.image_available, VK_NULL_HANDLE, &image_idx );

    if( res == VK_ERROR_OUT_OF_DATE_KHR ) {
        vk_recreate_swapchain();
    } else if( ( res != VK_SUCCESS ) && ( res != VK_SUBOPTIMAL_KHR ) )
        throw std::runtime_error( "Failed to acquire swapchain image" );

    vk_record_cmd_buffer( image_idx );

    VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
    VkSubmitInfo submit_info {
        .sType                  = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount     = 1,
        .pWaitSemaphores        = &g_app.sync.image_available,
        .pWaitDstStageMask      = &wait_dst_stage_mask,
        .commandBufferCount     = 1,
        .pCommandBuffers        = &g_app.render.cmds[image_idx],
        .signalSemaphoreCount   = 1,
        .pSignalSemaphores      = &g_app.sync.rendering_finished
    };
    VK_CALL( vkQueueSubmit( g_app.device.pres_queue, 1, &submit_info, g_app.sync.gpu_fence ),
             "Failed to submit draw command buffer" );
    VK_CALL( vkWaitForFences( g_app.device.vk_dev, 1, &g_app.sync.gpu_fence, VK_TRUE, UINT64_MAX ),
             "Failed to wait GPU fence" );

    VkPresentInfoKHR present_info {
        .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores    = &g_app.sync.rendering_finished,
        .swapchainCount     = 1,
        .pSwapchains        = &g_app.swap.swapchain,
        .pImageIndices      = &image_idx
    };
    res = vkQueuePresentKHR( g_app.device.pres_queue, &present_info );
    if( res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || g_app.swap.recreate ) {
        vk_recreate_swapchain();
    } else if( res != VK_SUCCESS )
        throw std::runtime_error( "Failed to submit present command buffer" );
}

/* application entry point */
int main() {
    try {
        init_glfw();
        init_window();
        init_vulkan();
    }
    catch( const std::exception &ex ) {
        std::cerr << "Error:"
                  << std::endl
                  << ex.what()
                  << std::endl
                  << "continue is not possible"
                  << std::endl;

        cleanup_vulkan();
        cleanup_glfw();

        return EXIT_FAILURE;
    }

    /* render loop */
    while( !glfwWindowShouldClose( g_app.glfw.window ) ) {
        draw();

        /* proceed keyboard and mouse */
        glfwPollEvents();
    }

    cleanup_vulkan();
    cleanup_glfw();

    return EXIT_SUCCESS;
}
