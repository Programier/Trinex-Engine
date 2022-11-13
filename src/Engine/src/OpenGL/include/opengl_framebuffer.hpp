#pragma once
#include <opengl.hpp>
#include <opengl_object.hpp>

#define frame_buffer(id) object_of<OpenGL_FrameBuffer>(id)
#define make_frame_buffer(variable, id) OpenGL_FrameBuffer* variable = frame_buffer(id)

class OpenGL_FrameBuffer : public OpenGL_Object {
public:
    GLuint _M_ID;
    GLuint _M_type;
    void destroy() override;
    declare_hpp_destructor(OpenGL_FrameBuffer);
};
