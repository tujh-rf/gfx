/*
    Direct3D9 tutorial
    
    Direct3D9 initialization
 */

#include <cstdlib>
#include <iostream>

/* GLFW
 */
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

/* Direct3D9
 */
#define WIN32_LEAN_AND_MEAN
#ifdef _DEBUG
#   define D3D_DEBUG_INFO
#endif /* debug */

#include <windows.h>
#include <d3d9.h>


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
struct d3dApp {
    struct {
        bool         init   { false };
        GLFWwindow  *window { nullptr };
    } glfw;
    struct {
        LPDIRECT3D9       instance { nullptr };
        LPDIRECT3DDEVICE9 device   { nullptr };
        D3DPRESENT_PARAMETERS d3dpp;

        /* Direct3D9 own the device in exclusive mode and from time
           to time this mode might be lost, so device must be restored
         */
        bool              device_lost { false };
    } d3d9;
};

/* callbacks
 */

static void glfw_error_callback( int error, const char* description ) {
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

static bool init_glfw( d3dApp &app ) {
    /* setup error callback first to catch all errors
     */
    (void)glfwSetErrorCallback( glfw_error_callback );

    /* init the library
     */
    if( glfwInit() == GLFW_FALSE )
        return false;

    app.glfw.init = true;

    return true;
}

static bool cleanup_glfw( d3dApp &app ) {
    if( !app.glfw.init )
        return true;

    /* cleanup GLFW resources
     */
    glfwTerminate();
    app.glfw.init = false;

    return true;
}

static bool init_window( d3dApp &app ) {
    if( !app.glfw.init )
        return false;

    /* don't create any graphical context
     */
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    app.glfw.window = glfwCreateWindow(
        window_width,
        window_height,
        "Direct3D9 - Tutorial - Initialization", /* window title */
        nullptr,                                 /* actual monitor - only for full screen applications */
        nullptr                                  /* window to share context with */
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

static bool cleanup_window( d3dApp &app ) {
    if( !app.glfw.window )
        return true;

    /* destroy GLFW window
     */
    glfwDestroyWindow( app.glfw.window );
    app.glfw.window = nullptr;

    return true;
}

/* Direct3D9
 */

static bool init_d3d9( d3dApp &app ) {
    if( !app.glfw.window )
        return false;

    /* Create Direct3D9 instance
     */
    app.d3d9.instance = Direct3DCreate9( D3D_SDK_VERSION );
    if( !app.d3d9.instance )
        return false;

    HRESULT res;
    D3DDISPLAYMODE d3ddm;

    /* Read current parameters of the display
       and test for the 16 Stencil format support
     */
    if( FAILED( res = app.d3d9.instance->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, /* default GPU everywhere */
                                                               &d3ddm ) ) )
        return false;

    if( FAILED( res = app.d3d9.instance->CheckDeviceFormat( D3DADAPTER_DEFAULT,
                                                            D3DDEVTYPE_HAL, /* hardware should support D3D9 */
                                                                /* in window mode it is needed to take the actual pixel format */
                                                            d3ddm.Format,
                                                            D3DUSAGE_DEPTHSTENCIL,
                                                            D3DRTYPE_SURFACE,
                                                            D3DFMT_D16 ) ) )
        return false;

    /* Test hardware support for the vertex processing
      (was needed to run the application on the hardware without D3D9 support)
     */
    D3DCAPS9 d3dcaps;
    if( FAILED( res = app.d3d9.instance->GetDeviceCaps( D3DADAPTER_DEFAULT,
                                                        D3DDEVTYPE_HAL,
                                                        &d3dcaps ) ) )
        return false;

    DWORD behavior_flags = 0;
    if( d3dcaps.VertexProcessingCaps ) {
        behavior_flags |= D3DCREATE_HARDWARE_VERTEXPROCESSING;
    }
    else
        behavior_flags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;

    /* Create Direcr3DDevice9
     */
    memset( &app.d3d9.d3dpp, 0, sizeof( D3DPRESENT_PARAMETERS ) );
    app.d3d9.d3dpp.BackBufferFormat       = d3ddm.Format;
    app.d3d9.d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD; /* no multisampling for now */
    app.d3d9.d3dpp.Windowed               = TRUE;
    app.d3d9.d3dpp.EnableAutoDepthStencil = TRUE;
    app.d3d9.d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    app.d3d9.d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_IMMEDIATE;

    if( FAILED( res = app.d3d9.instance->CreateDevice( D3DADAPTER_DEFAULT,
                                                       D3DDEVTYPE_HAL,
                                                       glfwGetWin32Window( app.glfw.window ),
                                                       behavior_flags,
                                                      &app.d3d9.d3dpp,
                                                      &app.d3d9.device ) ) )
        return false;

    return true;
}

static bool cleanup_d3d9( d3dApp &app ) {
    if( app.d3d9.device ) {
        app.d3d9.device->Release();
        app.d3d9.device = nullptr;
    }
    if( app.d3d9.instance ) {
        app.d3d9.instance->Release();
        app.d3d9.instance = nullptr;
    }
    return true;
}

static bool init( d3dApp &app ) {
    TUTORIAL_CALL( init_glfw( app ) );
    TUTORIAL_CALL( init_window( app ) );
    TUTORIAL_CALL( init_d3d9( app ) );

    return true;
}

static bool cleanup( d3dApp &app ) {
    TUTORIAL_CALL( cleanup_d3d9( app ) );
    TUTORIAL_CALL( cleanup_window( app ) );
    TUTORIAL_CALL( cleanup_glfw( app ) );

    return true;
}

static void draw( d3dApp &app ) {
    if( !app.d3d9.device )
        return;

    HRESULT res;

    if( app.d3d9.device_lost ) {
        if( FAILED( res = app.d3d9.device->TestCooperativeLevel() ) ) {
            /* device lost and cannot be restored
             */
            if( D3DERR_DEVICELOST == res )
                return;
            /* device lost but might be restored
             */
            if( D3DERR_DEVICENOTRESET == res ) {
                if( FAILED( app.d3d9.device->Reset( &app.d3d9.d3dpp ) ) )
                    /* failed to reset device and return to the exclusive mode
                     */
                    return;
            }
        }

        app.d3d9.device_lost = false;
    }

    if( FAILED ( app.d3d9.device->Clear( 0,
                                         nullptr,
                                         D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                         D3DCOLOR_COLORVALUE( 0.0f, 0.3f, 0.6f, 1.0f ),
                                         1.0f,
                                         0 ) ) )
        return;

    if( FAILED( app.d3d9.device->BeginScene() ) )
        return;

    if( FAILED( app.d3d9.device->EndScene() ) )
        return;

    res = app.d3d9.device->Present( nullptr, nullptr, NULL, nullptr );
    if( D3DERR_DEVICELOST == res )
        app.d3d9.device_lost = true;

}

int main() {
    d3dApp app;

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
