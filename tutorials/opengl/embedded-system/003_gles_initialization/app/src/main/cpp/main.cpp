/*
    OpenGLES 3.2 tutorial
    
    OpenGL 3.2 Initialization
 */

#include <iostream>
#include <vector>
#include <optional>

/* Platform specific headers
 */
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES3/gl3.h>
extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.h>
}


/* Android log tag
 */
static const char log_tag[] = "GLES Tutorial";

/* GLES Error check
 */
#define GLES_CALL( func, err_msg ) { \
    if( EGL_TRUE != func ) { \
        __android_log_print( ANDROID_LOG_ERROR, log_tag, "%s", err_msg ); \
        return false; \
    } \
}

/* Tutorial GLES function call
 */
#define TUTORIAL_CALL( func ) { \
    if( !func )                 \
        return false;           \
}

/* application data
 */
struct glesApp {
    bool         activity_init { false }; /* flag that Android activity created */
    android_app *native_ptr    { nullptr }; /* pointer to the Android native activity data */

    EGLDisplay  display { EGL_NO_DISPLAY }; /* pointer to the device display */
    EGLSurface  surface { EGL_NO_SURFACE }; /* pointer to the GLES surface */
    EGLContext  context { EGL_NO_CONTEXT }; /* pointer to the GLES context */
};

/* initialize EGL
 */

static bool gles_init_egl( glesApp &app ) {
    if( !app.native_ptr )
        return false;
    if( !app.native_ptr->window )
        return false;

    /* usually Android device has the only one display
     */
    app.display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
    GLES_CALL( eglInitialize( app.display,
                              nullptr,
                              nullptr ),
               "Fail to initialize GLES" );

    /* Order is important
     */
    EGLint config_attr_list[] = {
        /* OpenGL ES 3.x */
        EGL_RENDERABLE_TYPE,
        EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,
        EGL_WINDOW_BIT,
        /* BGR888 native_buffer_format */
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint config_count = 0;
    GLES_CALL( eglChooseConfig( app.display,
                                config_attr_list,
                                nullptr,
                                0,
                               &config_count ),
               "Fail to read amount of available display configurations" );
    if( !config_count ) {
        __android_log_print( ANDROID_LOG_ERROR, log_tag, "No display configurations are available" );
        return false;
    }

    std::vector< EGLConfig > available_confs( config_count );
    GLES_CALL( eglChooseConfig( app.display,
                                config_attr_list,
                                available_confs.data(),
                                static_cast< EGLint > ( available_confs.size() ),
                               &config_count ),
               "Cannot enumerate available display configurations" );

    std::optional< EGLConfig > config;
    for( const auto &conf: available_confs ) {
        EGLint blue, green, red, depth;
        if(    eglGetConfigAttrib( app.display, conf, EGL_BLUE_SIZE,  &blue )
            && eglGetConfigAttrib( app.display, conf, EGL_GREEN_SIZE, &green )
            && eglGetConfigAttrib( app.display, conf, EGL_RED_SIZE,   &red )
            && eglGetConfigAttrib( app.display, conf, EGL_DEPTH_SIZE, &depth ) ) {
            if( ( blue == 8 ) && ( green == 8 ) && ( red == 8 ) && ( depth == 24 ) ) {
                config = static_cast< EGLConfig > ( conf );
                break;
            }
        }
    }
    if( !config.has_value() ) {
        __android_log_print( ANDROID_LOG_ERROR, log_tag, "No supported display configuration found" );
        return false;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBufferGeometry()
     */
    EGLint native_buffer_format;
    GLES_CALL( eglGetConfigAttrib( app.display,
                                   config.value(),
                                   EGL_NATIVE_VISUAL_ID,
                                  &native_buffer_format ),
               "Cannot get the visual ID attribute for the choosen display configuration" );

    app.surface = eglCreateWindowSurface( app.display,
                                          config.value(),
                                          app.native_ptr->window,
                                          nullptr );
    if( app.surface == EGL_NO_SURFACE ) {
        __android_log_print( ANDROID_LOG_ERROR, log_tag, "Cannot create GLES window surface" );
        return false;
    }

    EGLint context_attr_list[] = {
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_NONE
    };
    app.context = eglCreateContext( app.display,
                                    config.value(),
                                    nullptr,
                                    context_attr_list );
    if( app.context == EGL_NO_CONTEXT ) {
        __android_log_print( ANDROID_LOG_ERROR, log_tag, "Cannot create GLES render context" );
        return false;
    }

    GLES_CALL( eglMakeCurrent( app.display,
                               app.surface,
                               app.surface,
                               app.context ),
               "Cannot set the current context" );
    GLES_CALL( eglSwapInterval( app.display, 1 ),
               "Cannot setup swap interval" );

    return true;
}

static bool gles_cleanup_egl( glesApp &app ) {
    GLES_CALL( eglMakeCurrent( app.display,
                               EGL_NO_SURFACE,
                               EGL_NO_SURFACE,
                               EGL_NO_CONTEXT ),
               "Cannot reset the window context" );

    if( app.context != EGL_NO_CONTEXT ) {
        GLES_CALL( eglDestroyContext( app.display,
                                      app.context ),
                   "Cannot destroy the render context" );
        app.context = EGL_NO_CONTEXT;
    }

    if( app.surface != EGL_NO_SURFACE ) {
        GLES_CALL( eglDestroySurface( app.display,
                                      app.surface ),
                   "Cannot destroy the GLES window surface" );
        app.context = EGL_NO_SURFACE;
    }

    if( app.display != EGL_NO_DISPLAY ) {
        GLES_CALL( eglTerminate( app.display ),
                   "Cannot terminate the connection to the display" );
        app.context = EGL_NO_DISPLAY;
    }

    return true;
}

/* initialize OpenGL
 */

static bool gles_init_opengl( glesApp &app ) {
    if( app.surface == EGL_NO_SURFACE )
        return false;
    if( app.context == EGL_NO_CONTEXT  )
        return false;

    glClearColor( 0.0f, 0.3f, 0.6f, 1.0f );

    return true;
}

static bool gles_cleanup_opengl( glesApp &app ) {
    return true;
}

/* Common initialization
 */

static bool init( glesApp &app ) {
    TUTORIAL_CALL( gles_init_egl( app ) );
    TUTORIAL_CALL( gles_init_opengl( app ) );
    return true;
}

static bool cleanup( glesApp &app ) {
    TUTORIAL_CALL( gles_cleanup_opengl( app ) );
    TUTORIAL_CALL( gles_cleanup_egl( app ) );
    return true;
}

static bool draw( glesApp &app ) {
    EGLint surface_width, surface_height;

    GLES_CALL( eglQuerySurface( app.display,
                                app.surface,
                                EGL_WIDTH,
                               &surface_width ),
       "Cannot get the width of the surface" );

    GLES_CALL( eglQuerySurface( app.display,
                                app.surface,
                                EGL_HEIGHT,
                               &surface_height ),
       "Cannot get the height of the surface" );

    glViewport( 0, 0, surface_width, surface_height );

    glClear( GL_COLOR_BUFFER_BIT );

    GLES_CALL( eglSwapBuffers( app.display,
                               app.surface ),
        "Cannot swap buffers of the surface" );

    return true;
}

/* callbacks
 */

static void android_looper_poll( android_app *native_ptr ) {
    int events;
    android_poll_source *source_ptr = nullptr;

    if( ALooper_pollOnce( 0,
                          nullptr,
                         &events,
                          reinterpret_cast< void** > ( &source_ptr ) ) ) {
        if( source_ptr )
            source_ptr->process( native_ptr,
                                 source_ptr );
    }
}

/* handle for the system commands
 */

void handle_cmd( android_app *native_ptr, int32_t command ) {
    glesApp *app = nullptr;

    if( native_ptr )
        app = reinterpret_cast< glesApp* > ( native_ptr->userData );

    switch ( command )
    {
    case APP_CMD_INIT_WINDOW:
        if( app ) {
            __android_log_print( ANDROID_LOG_ERROR, log_tag, "Application should not exist in this point" );
            return;
        }

        app = new glesApp;
        app->native_ptr = native_ptr;
        native_ptr->userData = reinterpret_cast< void* > ( app );

        if( !init( *app ) )  {
            cleanup( *app );
            return;
        }
        app->activity_init = true;

        break;
    case APP_CMD_TERM_WINDOW:
        if( !app ) {
            __android_log_print( ANDROID_LOG_ERROR, log_tag, "Application MUST exist in this point" );
            return;
        }

        app->activity_init = false;
        native_ptr->userData = nullptr;
        cleanup( *app );
        delete app;

        break;
    default:
        break;
    }
}

void android_main( struct android_app *native_ptr ) {
    /* register the even handler for system events
     */
    native_ptr->onAppCmd = handle_cmd;

    /* render loop
     */
    glesApp *app = nullptr;
    bool destroy_requested = false;
    do {
        app = reinterpret_cast< glesApp* > ( native_ptr->userData );
        if( app && app->activity_init ) {
            draw(*app);

            destroy_requested = app->native_ptr->destroyRequested;
        }

        android_looper_poll( native_ptr );

    } while( !destroy_requested );
}
