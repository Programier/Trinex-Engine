#pragma once
#include <opengl.hpp>
#include <opengl_object.hpp>

#define frame_buffer(id) static_cast<OpenGL_FrameBuffer*>(object_of(id)->_M_data)
#define make_frame_buffer(variable, id) OpenGL_FrameBuffer* variable = frame_buffer(id)

struct OpenGL_FrameBuffer {
    GLuint _M_ID;
    GLuint _M_type;
};
