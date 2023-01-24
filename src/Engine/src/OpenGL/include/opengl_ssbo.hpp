#pragma once
#include <opengl_object.hpp>

#define make_ssbo(ssbo, ID) OpenGL_SSBO* ssbo = object_of<OpenGL_SSBO>(ID)

class OpenGL_SSBO : public OpenGL_Object
{
public:
    GLuint _M_ID = 0;

    void destroy() override;
    declare_hpp_destructor(OpenGL_SSBO);
};
