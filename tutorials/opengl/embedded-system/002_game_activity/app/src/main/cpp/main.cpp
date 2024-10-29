/*
    OpenGLES 3.2 tutorial
    
    Android Game Activity
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
extern "C" {
#include <game-activity/native_app_glue/android_native_app_glue.h>
}


/* Android log tag
 */
static const char log_tag[] = "GLES Tutorial";


/* application data
 */
struct glesApp {
    bool         activity_init { false }; /* flag that Android activity created */
    android_app *native_ptr    { nullptr }; /* pointer to the Android native activity data */
};


/* Common initialization
 */

static bool init( glesApp &app ) {
    return true;
}

static bool cleanup( glesApp &app ) {
    return true;
}

static bool draw( glesApp &app ) {
    /* nothing for now */
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
