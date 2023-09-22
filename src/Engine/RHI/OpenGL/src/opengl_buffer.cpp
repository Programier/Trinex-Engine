#include <opengl_api.hpp>
#include <opengl_buffer.hpp>
#include <opengl_shader.hpp>
#include <opengl_types.hpp>

namespace Engine
{

    implement_opengl_instance_cpp(OpenGL_Buffer);

    MappedMemory OpenGL_Buffer::map_memory()
    {
        if (!_M_mapped_data)
        {
            glBindBuffer(_M_type, _M_instance_id);
            glGetBufferParameteriv(_M_type, GL_BUFFER_SIZE, &_M_buffer_size);
            _M_mapped_data = reinterpret_cast<byte*>(
                    glMapBufferRange(_M_type, 0, _M_buffer_size, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT));
            glBindBuffer(_M_type, 0);
        }

        return MappedMemory(_M_mapped_data, _M_buffer_size);
    }

    OpenGL_Buffer& OpenGL_Buffer::unmap_memory()
    {
        glBindBuffer(_M_type, _M_instance_id);
        glUnmapBuffer(_M_type);
        _M_mapped_data = nullptr;
        _M_buffer_size = 0;
        glBindBuffer(_M_type, 0);
        return *this;
    }

    bool OpenGL_Buffer::is_mapped() const
    {
        return _M_mapped_data != nullptr;
    }

    OpenGL_Buffer& OpenGL_Buffer::create(const byte* data, size_t size)
    {
        glGenBuffers(1, &_M_instance_id);
        glBindBuffer(_M_type, _M_instance_id);
        glBufferData(_M_type, size, data, GL_STATIC_DRAW);
        glBindBuffer(_M_type, 0);
        return *this;
    }

    OpenGL_Buffer& OpenGL_Buffer::update(size_t offset, const byte* data, size_t size)
    {
        glBindBuffer(_M_type, _M_instance_id);
        glBufferSubData(_M_type, offset, size, reinterpret_cast<const void*>(data));
        glBindBuffer(_M_type, 0);
        return *this;
    }

    implement_opengl_instance_cpp(OpenGL_VertexBuffer);

    OpenGL_VertexBuffer::OpenGL_VertexBuffer()
    {
        _M_type = GL_ARRAY_BUFFER;
    }

    OpenGL_VertexBuffer& OpenGL_VertexBuffer::bind(size_t offset)
    {
        unmap_memory();
        glBindBuffer(GL_ARRAY_BUFFER, _M_instance_id);
        API->state.shader->apply_vertex_attributes(offset);
        return *this;
    }

    MappedMemory OpenGL::map_vertex_buffer(const Identifier& ID)
    {
        return GET_TYPE(OpenGL_VertexBuffer, ID)->map_memory();
    }

    OpenGL& OpenGL::unmap_vertex_buffer(const Identifier& ID)
    {
        GET_TYPE(OpenGL_VertexBuffer, ID)->unmap_memory();
        return *this;
    }

    OpenGL_VertexBuffer::~OpenGL_VertexBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }

    implement_opengl_instance_cpp(OpenGL_IndexBuffer);

    OpenGL_IndexBuffer::OpenGL_IndexBuffer()
    {
        _M_type = GL_ELEMENT_ARRAY_BUFFER;
    }

    OpenGL_IndexBuffer& OpenGL_IndexBuffer::component_type(IndexBufferComponent component)
    {
        _M_component_type = get_type(component);
        return *this;
    }

    OpenGL_IndexBuffer& OpenGL_IndexBuffer::bind(size_t offset)
    {
        unmap_memory();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _M_instance_id);
        API->_M_index_buffer_offset = offset;
        API->state.index_buffer     = this;
        return *this;
    }

    MappedMemory OpenGL::map_index_buffer(const Identifier& ID)
    {
        return GET_TYPE(OpenGL_IndexBuffer, ID)->map_memory();
    }

    OpenGL& OpenGL::unmap_index_buffer(const Identifier& ID)
    {
        GET_TYPE(OpenGL_IndexBuffer, ID)->unmap_memory();
        return *this;
    }

    OpenGL_IndexBuffer::~OpenGL_IndexBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }


    implement_opengl_instance_cpp(OpenGL_UniformBuffer);

    OpenGL_UniformBuffer::OpenGL_UniformBuffer()
    {
        _M_type = GL_UNIFORM_BUFFER;
    }

    OpenGL_UniformBuffer& OpenGL_UniformBuffer::bind(BindingIndex binding)
    {
        unmap_memory();
        glBindBuffer(GL_UNIFORM_BUFFER, _M_instance_id);

        auto it = API->state.shader->_M_block_indices.find(binding);
        if (it != API->state.shader->_M_block_indices.end())
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, binding, _M_instance_id);
            glUniformBlockBinding(API->state.shader->_M_instance_id, it->second, binding);
        }
        return *this;
    }

    OpenGL_UniformBuffer::~OpenGL_UniformBuffer()
    {
        glDeleteBuffers(1, &_M_instance_id);
    }


    implement_opengl_instance_cpp(OpenGL_UniformBufferMap);


    OpenGL_UniformBufferMap::OpenGL_UniformBufferMap(const byte* data, size_t size)
    {
        for (int i = 0; i < 2; i++)
        {
            _M_buffers[i] = new OpenGL_UniformBuffer();
            _M_buffers[i]->create(data, size);
        }
    }

    OpenGL_UniformBuffer* OpenGL_UniformBufferMap::current_buffer()
    {
        return _M_buffers[API->_M_current_buffer_index];
    }

    OpenGL_UniformBufferMap::~OpenGL_UniformBufferMap()
    {
        for (int i = 0; i < 2; i++)
        {
            delete _M_buffers[i];
        }
    }

}// namespace Engine
