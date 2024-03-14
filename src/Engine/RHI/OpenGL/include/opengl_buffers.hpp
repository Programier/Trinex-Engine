#pragma once
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
    struct OpenGL_VertexBuffer : public RHI_VertexBuffer {
        GLuint m_id;


        OpenGL_VertexBuffer(size_t size, const byte* data);
        void bind(byte stream_index, size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_VertexBuffer();
    };

    struct OpenGL_IndexBuffer : public RHI_IndexBuffer {
        GLuint m_id;

        OpenGL_IndexBuffer(size_t, const byte* data);
        void bind(size_t offset) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_IndexBuffer();
    };

    struct OpenGL_UniformBuffer {
        GLuint m_id;
        size_t m_size;

        OpenGL_UniformBuffer(size_t size);
        void bind(BindLocation location);
        void bind(BindingIndex index);
        void update(size_t offset, size_t size, const byte* data);

        ~OpenGL_UniformBuffer();
    };

    struct OpenGL_LocalUniformBuffer {
        Vector<OpenGL_UniformBuffer*> m_buffers;
        Vector<byte> shadow_data;
        size_t shadow_data_size = 0;
        Index index             = 0;

        OpenGL_LocalUniformBuffer();
        void bind(BindingIndex index);
        void update(const void* data, size_t size, size_t offset);
        ~OpenGL_LocalUniformBuffer();
    };

    struct OpenGL_SSBO : public RHI_SSBO {
        GLuint m_id;

        OpenGL_SSBO(size_t size, const byte* data);

        void bind(BindLocation location) override;
        void update(size_t offset, size_t size, const byte* data) override;

        ~OpenGL_SSBO();
    };
}// namespace Engine
