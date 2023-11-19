/*
    Demonstration how to initialize OpenGL 4.1
 */

#include <cstdlib>
#include <iostream>
#include <stdexcept>

/* definition says GLFW do not load OpenGL functions - will be done by Glad */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

/* application data */
struct oglApp {
    bool        glfw_init   { false };
    GLFWwindow *window      { nullptr };
};

/* global constants */
static const int window_width  = 800;
static const int window_height = 600;

/* global variables */
oglApp g_app;

/* error handling */
static void error_callback( int error, const char* description ) {
    /* print error description */
    std::cout << "GLFW Error: "
              << description
              << std::endl;
}

/* key pressing
    ESC - close application
 */
static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ) {
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GLFW_TRUE );
}

/* initialize GLFW */
static void init_glfw() {
    /* setup error callback first to print all possible errors */
    glfwSetErrorCallback( error_callback );

    if( glfwInit() == GLFW_FALSE )
        throw std::runtime_error( "Cannot initialize GLFW" );

    g_app.glfw_init = true;
}

/* initialize render target window */
static void init_window() {
    if( !g_app.glfw_init )
        throw std::runtime_error( "GLFW is not initialized" );

    /* request OpenGL 4.1 functionality */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );

    /* to make MacOS happy with core profile */
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

    /* create render target window */
    g_app.window = glfwCreateWindow(
        window_width,
        window_height,
        "OpenGL 4.1 - Full screen application", /* window title */
        glfwGetPrimaryMonitor(),                /* actual monitor to switch on full screen */
        nullptr                                 /* window to share content with */
    );

    if( !g_app.window )
        throw std::runtime_error( "Cannot create the window and assign OpenGL content to it" );

    /* connect OpenGL content to the window */
    glfwMakeContextCurrent( g_app.window );
    glfwSwapInterval( 1 );

    /* setup keyboard callback */
    glfwSetKeyCallback( g_app.window, key_callback );
}

/* initialize OpenGL */
static void init_opengl() {
    if( !g_app.window )
        throw std::runtime_error( "GLFW window is not created" );

    if( !gladLoadGL() )
        throw std::runtime_error( "Cannot initialize OpenGL loader - Glad" );

    /* use blue color to cleanup the background of the scene */
    glClearColor( 0.0f, 0.3f, 0.6f, 1.0f );
}

/* cleanup OpenGL */
static void cleanup_opengl() {
    /* nothing */
}

/* clean GLFW */
static void cleanup_glfw() {
    if( g_app.window ) {
        glfwDestroyWindow( g_app.window );
        g_app.window = nullptr;
    }

    if( g_app.glfw_init ) {
        glfwTerminate();
        g_app.glfw_init = false;
    }
}

/* draw the scene */
static void draw() {
    int buff_width, buff_height;
    glfwGetFramebufferSize( g_app.window, &buff_width, &buff_height );

    /* synchronize viewport with window size */
    glViewport( 0, 0, buff_width, buff_height );

    /* clean window */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* update window with current scene */
    glfwSwapBuffers( g_app.window );
}

/* application entry point */
int main() {
    try {
        init_glfw();
        init_window();
        init_opengl();
    }
    catch( const std::exception &ex ) {
        std::cerr << "Error:"
                  << std::endl
                  << ex.what()
                  << std::endl
                  << "continue is not possible"
                  << std::endl;

        cleanup_opengl();
        cleanup_glfw();

        return EXIT_FAILURE;
    }

    /* render loop */
    while( !glfwWindowShouldClose( g_app.window ) ) {
        draw();

        /* proceed keyboard and mouse */
        glfwPollEvents();
    }

    cleanup_opengl();
    cleanup_glfw();

    return EXIT_SUCCESS;
}
