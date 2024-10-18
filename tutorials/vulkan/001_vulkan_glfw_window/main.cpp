/*
    Vulkan 1.3 tutorial
    
    GLFW window
 */

#include <cstdlib>
#include <iostream>

/* don't load Vulkan, will be done by volk
 */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>


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

static bool init_glfw( vkApp &app ) {
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

static bool cleanup_glfw( vkApp &app ) {
    if( !app.glfw.init )
        return true;

    /* cleanup GLFW resources
     */
    glfwTerminate();
    app.glfw.init = false;

    return true;
}

static bool init_window( vkApp &app ) {
    if( !app.glfw.init )
        return false;

    /* don't create any graphical context
     */
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

    app.glfw.window = glfwCreateWindow(
        window_width,
        window_height,
        "Vulkan 1.3 - Tutorial - Window", /* window title */
        nullptr,                          /* actual monitor - only for full screen applications */
        nullptr                           /* window to share context with */
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

static bool init( vkApp &app ) {
    TUTORIAL_CALL( init_glfw( app ) );
    TUTORIAL_CALL( init_window( app ) );

    return true;
}

static bool cleanup( vkApp &app ) {
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
