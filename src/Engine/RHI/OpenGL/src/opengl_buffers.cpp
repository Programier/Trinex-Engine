#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_shader.hpp>

namespace Engine
{

    OpenGL_VertexBuffer::OpenGL_VertexBuffer(size_t size, const byte* data)
    {
        glGenBuffers(1, &_M_id);
        glBindBuffer(GL_ARRAY_BUFFER, _M_id);
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
    }

    void OpenGL_VertexBuffer::bind(byte stream_index, size_t offset)
    {
        OpenGL_Pipeline* pipeline = OPENGL_API->_M_current_pipeline;

        if (pipeline && static_cast<size_t>(stream_index) < pipeline->_M_vertex_input.size())
        {
            glBindBuffer(GL_ARRAY_BUFFER, _M_id);
            auto& info = pipeline->_M_vertex_input[stream_index];
            glVertexAttribPointer(stream_index, info.count, info.type, info.normalize, info.size,
                                  reinterpret_cast<void*>(offset));
            glEnableVertexAttribArray(stream_index);
        }
    }

    void OpenGL_VertexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _M_id);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
    }

    OpenGL_VertexBuffer::~OpenGL_VertexBuffer()
    {
        glDeleteBuffers(1, &_M_id);
    }

    RHI_VertexBuffer* OpenGL::create_vertex_buffer(size_t size, const byte* data)
    {
        return new OpenGL_VertexBuffer(size, data);
    }


    static GLuint convert_index_buffer_component(IndexBufferComponent component)
    {
        switch (component)
        {
            case IndexBufferComponent::UnsignedByte:
                return GL_UNSIGNED_BYTE;
            case IndexBufferComponent::UnsignedShort:
                return GL_UNSIGNED_SHORT;
            case IndexBufferComponent::UnsignedInt:
                return GL_UNSIGNED_INT;
            default:
                throw EngineException("Unsupported index component type");
        }
    }

    OpenGL_IndexBuffer::OpenGL_IndexBuffer(size_t size, const byte* data, IndexBufferComponent component)
    {
        glGenBuffers(1, &_M_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        if (OPENGL_API->_M_current_index_buffer)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OPENGL_API->_M_current_index_buffer->_M_id);
        }

        _M_element_type = convert_index_buffer_component(component);
    }

    void OpenGL_IndexBuffer::bind(size_t offset)
    {
        if (OPENGL_API->_M_current_index_buffer != this)
        {
            OPENGL_API->_M_current_index_buffer = this;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_id);
        }
    }

    void OpenGL_IndexBuffer::update(size_t offset, size_t size, const byte* data)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_id);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, reinterpret_cast<const void*>(data));

        if (OPENGL_API->_M_current_index_buffer != nullptr && OPENGL_API->_M_current_index_buffer != this)
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OPENGL_API->_M_current_index_buffer->_M_id);
        else
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    OpenGL_IndexBuffer::~OpenGL_IndexBuffer()
    {
        if (_M_id)
        {
            glDeleteBuffers(1, &_M_id);
        }
    }

    RHI_IndexBuffer* OpenGL::create_index_buffer(size_t size, const byte* data, IndexBufferComponent component)
    {
        return new OpenGL_IndexBuffer(size, data, component);
    }


    OpenGL_UniformBuffer::OpenGL_UniformBuffer(size_t size, const byte* data)
    {
        glGenBuffers(1, &_M_id);
        glBindBuffer(GL_UNIFORM_BUFFER, _M_id);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
    }

    void OpenGL_UniformBuffer::bind(BindLocation location)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, location.binding, _M_id);
    }


    void OpenGL_UniformBuffer::update(size_t offset, size_t size, const byte* data)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, _M_id);
        glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
    }

    OpenGL_UniformBuffer::~OpenGL_UniformBuffer()
    {
        glDeleteBuffers(1, &_M_id);
    }


    RHI_UniformBuffer* OpenGL::create_uniform_buffer(size_t size, const byte* data)
    {
        return new OpenGL_UniformBuffer(size, data);
    }
}// namespace Engine
