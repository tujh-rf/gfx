/*
    Demonstration how to initialize OpenGL 2.1
 */

#include <cstdlib>
#include <iostream>

/* definition says GLFW do not load OpenGL functions - will be done by Glad */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

/* global constants */
static const int window_width  = 800;
static const int window_height = 600;

/* error handling */
static void error_callback( int error, const char* description ) {
    /* print error description */
    std::cout << "GLFW Error: "
              << description
              << std::endl;
}

/* create the render target window */
static GLFWwindow* create_window() {
    /* request OpenGL 2.1 functionality */
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );

    /* create render target window */
    GLFWwindow *window = glfwCreateWindow(
        window_width,
        window_height,
        "OpenGL 2.1 - Initialization",  /* window title */
        nullptr,                        /* actual screen - only for full screen applications */
        nullptr                         /* window to share content with */
    );

    if( window ) {
        /* connect OpenGL content to the window */
        glfwMakeContextCurrent( window );
        glfwSwapInterval( 1 );

        /* load OpenGL functions with Glad library */
        gladLoadGL();
    }

    return window;
}

/* key pressing
    ESC - close application
 */
static void key_callback( GLFWwindow* window, int key, int scancode, int action, int mods ) {
    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
        glfwSetWindowShouldClose( window, GLFW_TRUE );
}

/* initialize OpenGL */
static void opengl_init() {
    /* use blue color to cleanup scene background */
    glClearColor( 0.0f, 0.3f, 0.6f, 1.0f );
}

/* draw scene */
static void render() {
    /* clean window */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

/* application entry point */
int main() {
    /* setup error callback first to print all possible errors */
    glfwSetErrorCallback( error_callback );

    if( glfwInit() == GLFW_FALSE ) {
        std::cout << "Cannot initialize GLFW, continue is not possible"
                  << std::endl;
        exit( EXIT_FAILURE );
    }

    /* create render target window */
    GLFWwindow *window = create_window();
    if( !window ){
        std::cout << "Cannot create the window and assign OpenGL content to it, continue is not possible"
                  << std::endl;
        glfwTerminate();
        exit( EXIT_FAILURE );
    }

    /* setup keyboard callback */
    glfwSetKeyCallback( window, key_callback );

    /* initialize OpenGL */
    opengl_init();

    /* render loop */
    while( !glfwWindowShouldClose( window ) ) {
        int buff_width, buff_height;
        glfwGetFramebufferSize( window, &buff_width, &buff_height );

        /* synchronize viewport with window size */
        glViewport( 0, 0, buff_width, buff_height );

        /* draw the scene */
        render();

        /* update window with current scene */
        glfwSwapBuffers( window );

        /* proceed keyboard and mouse */
        glfwPollEvents();
    }

    /* cleanup */
    glfwDestroyWindow( window );
    glfwTerminate();

    return EXIT_SUCCESS;
}
