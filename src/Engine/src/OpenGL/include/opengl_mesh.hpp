#pragma once
#include <opengl.hpp>
#include <opengl_object.hpp>

#define mesh(id) object_of<OpenGL_Mesh>(id)
#define make_mesh(variable, id) OpenGL_Mesh* variable = mesh(id)

class OpenGL_Mesh : public OpenGL_Object{
public:
    GLuint _M_VBO = 0, _M_VAO = 0;
    std::size_t _M_buffer_size;

    void destroy() override;
};


