# OpenGL with Programmable Pipeline

A Shader is a user-defined program designed to run on some stage of a graphics processor. Shaders provide the code for certain programmable stages of the rendering pipeline. They can also be used in a slightly more limited form for general, on-GPU computation.

The OpenGL rendering pipeline defines the following shader stages, with their enumerator name:

- Vertex Shaders: GL_VERTEX_SHADER
- Tessellation Control and Evaluation Shaders: GL_TESS_CONTROL_SHADER and GL_TESS_EVALUATION_SHADER. (requires GL 4.0 or ARB_tessellation_shader)
- Geometry Shaders: GL_GEOMETRY_SHADER
- Fragment Shaders: GL_FRAGMENT_SHADER
- Compute Shaders: GL_COMPUTE_SHADER. (requires GL 4.3 or ARB_compute_shader)

[Shader](https://www.khronos.org/opengl/wiki/Shader)

[GLFW Window](001_ogl4_glfw_window/README.md)
[OpenGL 4.6 Initialization](002_ogl4_opengl_initialization/README.md)
---
