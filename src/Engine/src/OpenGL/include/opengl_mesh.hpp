#pragma once
#include <opengl.hpp>

#define mesh(id) static_cast<OpenGL_Mesh*>(object_of(id)->_M_data)
#define make_mesh(variable, id) OpenGL_Mesh* variable = mesh(id)

struct OpenGL_Mesh {
    GLuint _M_VBO = 0, _M_VAO = 0;
    std::size_t _M_buffer_size;
};


