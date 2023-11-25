#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_VertexBuffer : public RHI_VertexBuffer {
        GLuint _M_id;


        OpenGL_VertexBuffer(size_t size, const byte* data);
        void bind(byte stream_index, size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_VertexBuffer();
    };

    struct OpenGL_IndexBuffer : public RHI_IndexBuffer {
        GLuint _M_id;
        GLuint _M_element_type;

        OpenGL_IndexBuffer(size_t, const byte* data, IndexBufferComponent);
        void bind(size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_IndexBuffer();
    };

    struct OpenGL_UniformBuffer : public RHI_UniformBuffer {
        GLuint _M_id;


        OpenGL_UniformBuffer(size_t size, const byte* data);
        void bind(BindLocation location) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_UniformBuffer();
    };
}// namespace Engine
