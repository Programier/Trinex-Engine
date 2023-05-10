#include <opengl_api.hpp>
#include <opengl_buffer.hpp>
#include <opengl_shader.hpp>
#include <opengl_types.hpp>

namespace Engine
{
    implement_opengl_instance_cpp(OpenGL_VertexBuffer);

    OpenGL_VertexBuffer& OpenGL_VertexBuffer::create_vertex_buffer(const byte* data, size_t size)
    {
        glGenBuffers(1, &_M_instance_id);
        glBindBuffer(GL_ARRAY_BUFFER, _M_instance_id);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return *this;
    }

    OpenGL_VertexBuffer& OpenGL_VertexBuffer::update_vertex_buffer(size_t offset, const byte* data, size_t size)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _M_instance_id);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, reinterpret_cast<const void*>(data));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return *this;
    }

    OpenGL_VertexBuffer& OpenGL_VertexBuffer::bind_vertex_buffer(size_t offset)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _M_instance_id);
        API->_M_current_shader->apply_vertex_attributes(offset);
        return *this;
    }

    static MappedMemory map_memory(GLuint internal_id, GLenum type)
    {
        GLint buffer_size = 0;
        glBindBuffer(type, internal_id);
        glGetBufferParameteriv(type, GL_BUFFER_SIZE, &buffer_size);
        void* data = glMapBufferRange(type, 0, buffer_size, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT);
        glBindBuffer(type, 0);
        return MappedMemory(reinterpret_cast<byte*>(data), buffer_size);
    }

    static void unmap_memory(GLuint internal_id, GLenum type)
    {
        glBindBuffer(type, internal_id);
        glUnmapBuffer(type);
        glBindBuffer(type, 0);
    }

    MappedMemory OpenGL::map_vertex_buffer(const Identifier& ID)
    {
        return map_memory(GET_TYPE(OpenGL_VertexBuffer, ID)->_M_instance_id, GL_ARRAY_BUFFER);
    }

    OpenGL& OpenGL::unmap_vertex_buffer(const Identifier& ID)
    {
        unmap_memory(GET_TYPE(OpenGL_VertexBuffer, ID)->_M_instance_id, GL_ARRAY_BUFFER);
        return *this;
    }

    OpenGL_VertexBuffer::~OpenGL_VertexBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }

    implement_opengl_instance_cpp(OpenGL_IndexBuffer);

    OpenGL_IndexBuffer& OpenGL_IndexBuffer::create_index_buffer(const byte* data, size_t size,
                                                                IndexBufferComponent component)
    {
        glGenBuffers(1, &_M_instance_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_instance_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        _M_component_type = get_type(component);
        return *this;
    }

    OpenGL_IndexBuffer& OpenGL_IndexBuffer::update_index_buffer(size_t offset, const byte* data, size_t size)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_instance_id);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, reinterpret_cast<const void*>(data));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        return *this;
    }

    OpenGL_IndexBuffer& OpenGL_IndexBuffer::bind_index_buffer(size_t offset)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_instance_id);
        API->_M_index_buffer_offset = offset;
        API->_M_index_buffer        = this;
        return *this;
    }

    MappedMemory OpenGL::map_index_buffer(const Identifier& ID)
    {
        return map_memory(GET_TYPE(OpenGL_IndexBuffer, ID)->_M_instance_id, GL_ELEMENT_ARRAY_BUFFER);
    }

    OpenGL& OpenGL::unmap_index_buffer(const Identifier& ID)
    {
        unmap_memory(GET_TYPE(OpenGL_IndexBuffer, ID)->_M_instance_id, GL_ELEMENT_ARRAY_BUFFER);
        return *this;
    }

    OpenGL_IndexBuffer::~OpenGL_IndexBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }


    implement_opengl_instance_cpp(OpenGL_UniformBuffer);

    OpenGL_UniformBuffer& OpenGL_UniformBuffer::create_uniform_buffer(const byte* data, size_t size)
    {
        glGenBuffers(1, &_M_instance_id);
        glBindBuffer(GL_UNIFORM_BUFFER, _M_instance_id);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_DYNAMIC_DRAW);
        return *this;
    }

    OpenGL_UniformBuffer& OpenGL_UniformBuffer::update_uniform_buffer(size_t offset, const byte* data, size_t size)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _M_instance_id);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, reinterpret_cast<const void*>(data));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        return *this;
    }

    OpenGL_UniformBuffer& OpenGL_UniformBuffer::bind_uniform_buffer(BindingIndex binding, size_t offset, size_t size)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _M_instance_id);

        auto it = API->_M_current_shader->_M_block_indices.find(binding);
        if (it != API->_M_current_shader->_M_block_indices.end())
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, _M_instance_id);
            glUniformBlockBinding(API->_M_current_shader->_M_instance_id, it->second, binding);
        }
        return *this;
    }

    OpenGL_UniformBuffer::~OpenGL_UniformBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }

}// namespace Engine
