#pragma once

#include <Core/buffer_types.hpp>
#include <opengl_object.hpp>

namespace Engine
{
    struct OpenGL_VertexBuffer : OpenGL_Object {
        implement_opengl_instance_hpp();
        OpenGL_VertexBuffer& create_vertex_buffer(const byte*, size_t);
        OpenGL_VertexBuffer& update_vertex_buffer(size_t offset, const byte*, size_t);
        OpenGL_VertexBuffer& bind_vertex_buffer(size_t offset);
        ~OpenGL_VertexBuffer();
    };

    struct OpenGL_IndexBuffer : OpenGL_Object {
        GLenum _M_component_type;
        implement_opengl_instance_hpp();
        OpenGL_IndexBuffer& create_index_buffer(const byte*, size_t, IndexBufferComponent);
        OpenGL_IndexBuffer& update_index_buffer(size_t offset, const byte*, size_t);
        OpenGL_IndexBuffer& bind_index_buffer(size_t offset);
        ~OpenGL_IndexBuffer();
    };

    struct OpenGL_UniformBuffer : OpenGL_Object {
        implement_opengl_instance_hpp();
        OpenGL_UniformBuffer& create_uniform_buffer(const byte*, size_t);
        OpenGL_UniformBuffer& update_uniform_buffer(size_t offset, const byte*, size_t);
        OpenGL_UniformBuffer& bind_uniform_buffer(BindingIndex binding);
        ~OpenGL_UniformBuffer();
    };
}// namespace Engine
