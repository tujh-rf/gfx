/*
    Demonstration how to initialize OpenGL ES 3.0 on Android System
 */

#include <iostream>
#include <stdexcept>
#include <optional>
#include <vector>

#include <jni.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES3/gl3.h>

extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.h>
}

#define EGL_CALL( func, err_msg )                   \
    {                                               \
        if( EGL_TRUE != func )                      \
            throw std::runtime_error( err_msg );    \
    }

/* application data */
struct glesApp {
    bool         activity_init  { false };  //!<
    android_app *native_ptr     {nullptr }; //!<

    EGLDisplay  display { EGL_NO_DISPLAY }; //!<
    EGLSurface  surface { EGL_NO_SURFACE }; //!<
    EGLContext  context { EGL_NO_CONTEXT }; //!<
};

/* global variables */
glesApp g_app;

/* initialize OpenGL ES */
static void init_gles() {
    if( !g_app.native_ptr )
        throw std::runtime_error( "Native application pointer is NULL" );
    if( !g_app.native_ptr->window )
        throw  std::runtime_error( "Native Window is not set properly" );

    /* default display on Android usually is what is needed */
    g_app.display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    EGL_CALL( eglInitialize( g_app.display, nullptr, nullptr ),
              "Cannot initialize Embedded System Graphic Library" );

    EGLint config_attr_list[] = {
            /* OpenGL ES 3.x */
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            /* BGR888 native_buffer_format */
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    EGLint config_num;
    EGL_CALL( eglChooseConfig( g_app.display, config_attr_list, nullptr, 0, &config_num ),
              "Failed to read count of available configurations" );

    if( !config_num )
        throw std::runtime_error( "No configurations are available" );

    std::vector< EGLConfig > supported_confs( config_num );
    EGL_CALL( eglChooseConfig( g_app.display, config_attr_list, supported_confs.data(), supported_confs.size(), &config_num ),
              "Failed to enumerate available configurations" );

    std::optional< EGLConfig > config;
    for ( EGLConfig &conf: supported_confs ) {
        EGLint red, green, blue, depth;
        if (   eglGetConfigAttrib( g_app.display, conf, EGL_RED_SIZE, &red )
            && eglGetConfigAttrib( g_app.display, conf, EGL_GREEN_SIZE, &green )
            && eglGetConfigAttrib( g_app.display, conf, EGL_BLUE_SIZE, &blue )
            && eglGetConfigAttrib( g_app.display, conf, EGL_DEPTH_SIZE, &depth ) ) {
                config = conf;
                break;
        }
    }
    if( !config.has_value() )
        throw std::runtime_error( "Cannot find supported configuration" );

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBufferGeometry() */
    EGLint native_buffer_format;
    EGL_CALL( eglGetConfigAttrib( g_app.display, config.value(), EGL_NATIVE_VISUAL_ID, &native_buffer_format ),
              "Failed to read the visual id attribute from the chosen configuration" );

    g_app.surface = eglCreateWindowSurface( g_app.display, config.value(), g_app.native_ptr->window, nullptr );
    if( g_app.surface == EGL_NO_SURFACE )
        throw std::runtime_error( "Failed to create EGL surface" );

    EGLint context_attr_list[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_NONE
    };
    g_app.context = eglCreateContext( g_app.display, config.value(), nullptr, context_attr_list );
    if( g_app.context == EGL_NO_CONTEXT )
        throw std::runtime_error( "Failed to create the render context" );

    EGL_CALL( eglMakeCurrent( g_app.display, g_app.surface, g_app.surface, g_app.context ),
              "Failed to change the current context" );
    EGL_CALL( eglSwapInterval( g_app.display, 1 ),
              "Failed to change swap interval" );

    /* use blue color to cleanup the background of the scene */
    glClearColor( 0.0f, 0.3f, 0.6f, 1.0f );
}

static void cleanup_gles() {
    EGL_CALL( eglMakeCurrent( g_app.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT ),
              "Failed to reset the current context" );

    if( g_app.context != EGL_NO_CONTEXT ) {
        EGL_CALL( eglDestroyContext(g_app.display, g_app.context ),
                  "Failed to destroy the render context" );
        g_app.context = EGL_NO_CONTEXT;
    }

    if( g_app.surface != EGL_NO_SURFACE ) {
        EGL_CALL( eglDestroySurface( g_app.display, g_app.surface ),
                  "Failed to destroy the EGL surface" );
        g_app.surface = EGL_NO_SURFACE;
    }

    if( g_app.display != EGL_NO_DISPLAY ) {
        EGL_CALL( eglTerminate( g_app.display ),
                  "Failed to terminate the connection to the display" );
        g_app.display = EGL_NO_DISPLAY;
    }
}

static void draw() {
    EGLint surface_width;
    EGL_CALL( eglQuerySurface( g_app.display, g_app.surface, EGL_WIDTH, &surface_width ),
              "Failed to read surface surface_width" );

    EGLint surface_height;
    EGL_CALL( eglQuerySurface( g_app.display, g_app.surface, EGL_HEIGHT, &surface_height ),
              "Failed to read surface surface_height" );

    /* synchronize viewport with surface size */
    glViewport( 0, 0, surface_width, surface_height );

    /* clean window */
    glClear( GL_COLOR_BUFFER_BIT );

    /**/
    EGL_CALL( eglSwapBuffers( g_app.display, g_app.surface ),
              "Failed to swap buffers on the surface");
}

static inline void alooper_poll() {
    int events;
    android_poll_source *psource;
    if( ALooper_pollOnce( 0, nullptr, &events, reinterpret_cast< void** > ( &psource ) ) ) {
        if( psource )
            psource->process( g_app.native_ptr, psource );
    }
}

void handle_cmd(android_app *app_ptr, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            try {
                init_gles();
            }
            catch( const std::exception &ex ) {
                std::cerr << "Error:"
                          << std::endl
                          << ex.what()
                          << std::endl
                          << "continue is not possible"
                          << std::endl;

                cleanup_gles();

                return;
            }
            g_app.activity_init = true;
            break;
        case APP_CMD_TERM_WINDOW:
            g_app.activity_init = false;
            cleanup_gles();
            break;
        default:
            break;
    }
}

void android_main(struct android_app *app_ptr) {
    g_app.native_ptr = app_ptr;

    // Register an event handler for Android events
    app_ptr->onAppCmd = handle_cmd;

    do {
        if( g_app.activity_init )
            draw();

        alooper_poll();
    } while( !g_app.native_ptr->destroyRequested );
}
