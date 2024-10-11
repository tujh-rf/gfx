/*
    OpenGL 2.1 tutorial
    
    GLFW window
 */

#include <cstdlib>
#include <iostream>

/* don't load OpenGL, will be done by Glad
 */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


/* global constants
 */
static const int window_width  = 800;
static const int window_height = 600;

/* application data
 */
struct oglApp {
    bool         glfw_init { false };
    GLFWwindow  *window    { nullptr };
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

static bool init_glfw( oglApp *app ) {
    if( !app )
        return false;

    /* setup error callback first to catch all errors
     */
    (void)glfwSetErrorCallback( glfw_error_callback );

    /* init the library
     */
    if( glfwInit() == GLFW_FALSE )
        return false;

    app->glfw_init = true;

    return true;
}

static bool cleanup_glfw( oglApp *app ) {
    if( !app )
        return false;

    if( !app->glfw_init )
        return true;

    /* cleanup GLFW resources
     */
    glfwTerminate();
    app->glfw_init = false;

    return true;
}

static bool init_window( oglApp *app ) {
    if( !app )
        return false;
    if( !app->glfw_init )
        return false;

    /* request OpenGL 2.1
     */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );

    app->window = glfwCreateWindow(
        window_width,
        window_height,
        "OpenGL 2.1 - Tutorial - Window", /* window title */
        nullptr,                          /* actual monitor - only for full screen applications */
        nullptr                           /* window to share context with */
    );
    if( !app->window )
        return false;

    /* setup user pointer
     */
    glfwSetWindowUserPointer(
        app->window,
        app 
    );

    /* connect OpenGL context with window
     */
    glfwMakeContextCurrent( app->window );
    glfwSwapInterval( 1 );  /* update window every time */

    /* acquire the keyboard
     */
    glfwSetKeyCallback(
        app->window,
        key_callback
    );

    return true;
}

static bool cleanup_window( oglApp *app ) {
    if( !app )
        return false;

    if( !app->window )
        return true;

    /* destroy GLFW window
     */
    glfwDestroyWindow( app->window );
    app->window = nullptr;

    return true;
}

static bool init( oglApp *app ) {
    if( !init_glfw( app ) )
        return false;
    if( !init_window( app ) )
        return false;

    return true;
}

static bool cleanup( oglApp *app ) {
    if( !cleanup_window( app ) )
        return false;
    if( !cleanup_glfw( app ) )
        return false;

    return true;
}

static void draw( oglApp *app ) {
    int frame_width, frame_height;

    /* read the actual frame buffer size of the window
     */
    glfwGetFramebufferSize(
        app->window,
       &frame_width,
       &frame_height
    );

    /* update window
     */
    glfwSwapBuffers( app->window );
}

int main() {
    oglApp app;

    if( !init( &app ) ) {
        std::cerr
            << "Cannot initialize the application"
                << std::endl;
        return EXIT_FAILURE;
    }

    /* render loop
     */
    while( glfwWindowShouldClose( app.window ) == GLFW_FALSE ) {
        /* draw the context of the window
         */
        draw( &app );

        /* proceed keyboard and mouse
         */
        glfwPollEvents();
    }

    if( !cleanup( &app ) ) {
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
