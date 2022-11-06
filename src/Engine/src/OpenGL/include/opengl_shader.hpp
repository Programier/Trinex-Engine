#pragma once

#include <opengl_object.hpp>


#define VERTEXT_ID 0
#define FRAGMENT_ID 1
#define COMPUTE_ID 2
#define GEOMENTRY_ID 3

#define make_shader(shader, ID) OpenGL_Shader* shader = object_of<OpenGL_Shader>(ID)

class OpenGL_Shader : public OpenGL_Object
{
public:
    bool _M_inided = false;

    GLuint _M_shaders_types_id[4] = {0, 0, 0, 0};
    GLuint _M_shader_id = 0;

    void destroy() override;
};
