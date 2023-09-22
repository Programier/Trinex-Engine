#pragma once

namespace Engine
{
    struct OpenGL_Shader;
    struct OpenGL_Texture;
    struct OpenGL_Sampler;
    struct OpenGL_IndexBuffer;
    struct OpenGL_FrameBufferSet;

    struct OpenGL_State {
        OpenGL_Shader* shader = nullptr;;
        OpenGL_IndexBuffer* index_buffer   = nullptr;
        OpenGL_FrameBufferSet* framebuffer = nullptr;

        OpenGL_State& flush();
        OpenGL_State& prepare_draw();
    };



}// namespace Engine
