#pragma once

#include <Core/buffer_types.hpp>
#include <opengl_api.hpp>
#include <opengl_object.hpp>

namespace Engine
{

    struct OpenGL_Buffer : OpenGL_Object {
        implement_opengl_instance_hpp();

        byte* _M_mapped_data = nullptr;
        GLint _M_buffer_size = 0;
        GLuint _M_type;

        MappedMemory map_memory();
        OpenGL_Buffer& unmap_memory();

        OpenGL_Buffer& create(const byte*, size_t);
        OpenGL_Buffer& update(size_t offset, const byte*, size_t size);

        bool is_mapped() const;
    };

    struct OpenGL_VertexBuffer : OpenGL_Buffer {
        implement_opengl_instance_hpp();

        OpenGL_VertexBuffer();

        OpenGL_VertexBuffer& bind(size_t offset);
        ~OpenGL_VertexBuffer();
    };

    struct OpenGL_IndexBuffer : OpenGL_Buffer {
        GLenum _M_component_type;
        implement_opengl_instance_hpp();

        OpenGL_IndexBuffer();
        OpenGL_IndexBuffer& component_type(IndexBufferComponent component);
        OpenGL_IndexBuffer& bind(size_t offset);
        ~OpenGL_IndexBuffer();
    };

    struct OpenGL_UniformBuffer : OpenGL_Buffer {
        implement_opengl_instance_hpp();

        OpenGL_UniformBuffer();
        OpenGL_UniformBuffer& bind(BindingIndex binding);
        ~OpenGL_UniformBuffer();
    };


    struct OpenGL_UniformBufferMap : public OpenGL_Object {

        OpenGL_UniformBuffer* _M_buffers[2];

        implement_opengl_instance_hpp();

        OpenGL_UniformBufferMap(const byte* data, size_t size);

        OpenGL_UniformBuffer* current_buffer();
        OpenGL_UniformBuffer* next_buffer();

        ~OpenGL_UniformBufferMap();
    };
}// namespace Engine
