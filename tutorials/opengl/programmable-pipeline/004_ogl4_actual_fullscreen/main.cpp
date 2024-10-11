/*
    OpenGL 4.6 tutorial
    
    Fullscreen application with actual resolution of the screen
 */

#include <cstdlib>
#include <iostream>

/* don't load OpenGL, will be done by Glad
 */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


/* application data
 */
struct oglApp {
    bool         glfw_init { false };
    GLFWwindow  *window    { nullptr };
    bool         gl_loaded { false };
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

    /* request OpenGL 4.6
     */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );

    /* read actual screen resolution
     */
    GLFWmonitor *primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *primary_monitor_mode = glfwGetVideoMode( primary_monitor );
    int window_width = primary_monitor_mode->width;
    int window_height = primary_monitor_mode->height;

    app->window = glfwCreateWindow(
        window_width,
        window_height,
        "OpenGL 4.6 - Tutorial - Fullscreen",
        primary_monitor,
        nullptr
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

static bool init_opengl( oglApp *app ) {
    if( !app )
        return false;
    if( !app->window )
        return false;

    /* run Glad and load actual version of OpenGL
     */
    if( !gladLoadGL() )
        return false;

    app->gl_loaded = true;

    /* setup window clean color - light blue
     */
    glClearColor( 0.0f, 0.3f, 0.6f, 1.0f );

    return true;
}

static bool cleanup_opengl( oglApp *app ) {
    if( !app )
        return false;
    if( !app->gl_loaded )
        return false;

    app->gl_loaded = false;

    /* nothing is here
     */
    return true;
}

static bool init( oglApp *app ) {
    if( !init_glfw( app ) )
        return false;
    if( !init_window( app ) )
        return false;
    if( !init_opengl( app ) )
        return false;

    return true;
}

static bool cleanup( oglApp *app ) {
    if( !cleanup_opengl( app ) )
        return false;
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

        /* Synchronize viewport with window size
         */
        glViewport(
            0, 0,
            frame_width, frame_height
        );

        /* clean window background
         */
        glClear(
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT
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
