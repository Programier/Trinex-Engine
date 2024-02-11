#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <cstring>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_shader.hpp>


#define MAX_LOCAL_UBO_SIZE 4096

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


    OpenGL_UniformBuffer::OpenGL_UniformBuffer(size_t size) : _M_size(size)
    {
        glGenBuffers(1, &_M_id);
        glBindBuffer(GL_UNIFORM_BUFFER, _M_id);
        glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_COPY);
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

    OpenGL_LocalUniformBuffer::OpenGL_LocalUniformBuffer()
    {
        _M_buffers.push_back(new OpenGL_UniformBuffer(MAX_LOCAL_UBO_SIZE));
    }

    void OpenGL_LocalUniformBuffer::bind()
    {
        if (shadow_data_size == 0)
        {
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, 0);
            return;
        }

        while(_M_buffers[index]->_M_size < shadow_data_size)
        {
            ++index;

            if(_M_buffers.size() <= index)
            {
                _M_buffers.push_back(new OpenGL_UniformBuffer(shadow_data_size));
            }
        }

        OpenGL_UniformBuffer* buffer = _M_buffers[index];
        buffer->update(0, shadow_data_size, shadow_data.data());
        buffer->bind({1, 0});

        shadow_data_size = 0;
        index            = 0;
    }

    void OpenGL_LocalUniformBuffer::update(const void* data, size_t size, size_t offset)
    {
        shadow_data_size = glm::max(size + offset, shadow_data_size);
        if (shadow_data.size() < shadow_data_size)
            shadow_data.resize(shadow_data_size);

        std::memcpy(shadow_data.data() + offset, data, size);
    }

    OpenGL_LocalUniformBuffer::~OpenGL_LocalUniformBuffer()
    {
        for (auto& ell : _M_buffers)
        {
            delete ell;
        }

        _M_buffers.clear();
    }


    OpenGL_SSBO::OpenGL_SSBO(size_t size, const byte* data)
    {
        glGenBuffers(1, &_M_id);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _M_id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }

    void OpenGL_SSBO::bind(BindLocation location)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location.binding, _M_id);
    }

    void OpenGL_SSBO::update(size_t offset, size_t size, const byte* data)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _M_id);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    }

    OpenGL_SSBO::~OpenGL_SSBO()
    {
        glDeleteBuffers(1, &_M_id);
    }

    RHI_SSBO* OpenGL::create_ssbo(size_t size, const byte* data)
    {
        return new OpenGL_SSBO(size, data);
    }

    OpenGL& OpenGL::initialize_ubo()
    {
        _M_global_ubo = new OpenGL_UniformBuffer(sizeof(GlobalShaderParameters));
        _M_local_ubo  = new OpenGL_LocalUniformBuffer();
        return *this;
    }

    OpenGL& OpenGL::push_global_params(const GlobalShaderParameters& params)
    {
        _M_global_parameters_stack.push_back(params);
        _M_global_ubo->update(0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
        return *this;
    }

    OpenGL& OpenGL::pop_global_params()
    {
        if (!_M_global_parameters_stack.empty())
        {
            _M_global_parameters_stack.pop_back();
            if (!_M_global_parameters_stack.empty())
            {
                GlobalShaderParameters& params = _M_global_parameters_stack.back();
                _M_global_ubo->update(0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
            }
        }
        return *this;
    }

    OpenGL& OpenGL::update_local_parameter(const void* data, size_t size, size_t offset)
    {
        _M_local_ubo->update(data, size, offset);
        return *this;
    }

}// namespace Engine
