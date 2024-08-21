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

	OpenGL_VertexBuffer::OpenGL_VertexBuffer(size_t size, const byte* data, RHIBufferType type)
	{
		GLenum gl_type = type == RHIBufferType::Static ? GL_STATIC_DRAW : GL_STREAM_DRAW;
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferData(GL_ARRAY_BUFFER, size, data, gl_type);
	}

	void OpenGL_VertexBuffer::bind(byte stream_index, size_t stride, size_t offset)
	{
		glBindVertexBuffer(stream_index, m_id, offset, stride);
	}

	void OpenGL_VertexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_id);
		glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
	}

	OpenGL_VertexBuffer::~OpenGL_VertexBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	RHI_VertexBuffer* OpenGL::create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)
	{
		return new OpenGL_VertexBuffer(size, data, type);
	}


	OpenGL_IndexBuffer::OpenGL_IndexBuffer(size_t size, const byte* data, IndexBufferFormat format, RHIBufferType type)
	{
		m_format = format == IndexBufferFormat::UInt32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;

		glGenBuffers(1, &m_id);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		GLenum gl_type = type == RHIBufferType::Static ? GL_STATIC_DRAW : GL_STREAM_DRAW;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, gl_type);

		if (OPENGL_API->m_state.index_buffer)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OPENGL_API->m_state.index_buffer->m_id);
		}
	}

	void OpenGL_IndexBuffer::bind(size_t offset)
	{
		if (OPENGL_API->m_state.index_buffer != this)
		{
			OPENGL_API->m_state.index_buffer = this;
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		}
	}

	void OpenGL_IndexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, reinterpret_cast<const void*>(data));

		if (OPENGL_API->m_state.index_buffer != nullptr && OPENGL_API->m_state.index_buffer != this)
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OPENGL_API->m_state.index_buffer->m_id);
		else
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	OpenGL_IndexBuffer::~OpenGL_IndexBuffer()
	{
		if (m_id)
		{
			glDeleteBuffers(1, &m_id);
		}
	}

	RHI_IndexBuffer* OpenGL::create_index_buffer(size_t size, const byte* data, IndexBufferFormat format, RHIBufferType type)
	{
		return new OpenGL_IndexBuffer(size, data, format, type);
	}


	OpenGL_UniformBuffer::OpenGL_UniformBuffer(size_t size) : m_size(size)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_COPY);
	}

	void OpenGL_UniformBuffer::bind(BindLocation location)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, location.binding, m_id);
	}

	void OpenGL_UniformBuffer::bind(BindingIndex index)
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, index, m_id);
	}

	void OpenGL_UniformBuffer::update(size_t offset, size_t size, const byte* data)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, m_id);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
	}

	OpenGL_UniformBuffer::~OpenGL_UniformBuffer()
	{
		glDeleteBuffers(1, &m_id);
	}

	OpenGL_LocalUniformBuffer::OpenGL_LocalUniformBuffer()
	{
		m_buffers.push_back(new OpenGL_UniformBuffer(MAX_LOCAL_UBO_SIZE));
	}

	void OpenGL_LocalUniformBuffer::bind(BindingIndex binding_index)
	{
		if (shadow_data_size == 0)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, index, 0);
			return;
		}

		while (m_buffers[index]->m_size < shadow_data_size)
		{
			++index;

			if (m_buffers.size() <= index)
			{
				m_buffers.push_back(new OpenGL_UniformBuffer(shadow_data_size));
			}
		}

		OpenGL_UniformBuffer* buffer = m_buffers[index];
		buffer->update(0, shadow_data_size, shadow_data.data());
		buffer->bind(binding_index);

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
		for (auto& ell : m_buffers)
		{
			delete ell;
		}

		m_buffers.clear();
	}

	OpenGL_SSBO::OpenGL_SSBO(size_t size, const byte* data)
	{
		glGenBuffers(1, &m_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
	}

	void OpenGL_SSBO::bind(BindLocation location)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, location.binding, m_id);
	}

	void OpenGL_SSBO::update(size_t offset, size_t size, const byte* data)
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_id);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}

	OpenGL_SSBO::~OpenGL_SSBO()
	{
		glDeleteBuffers(1, &m_id);
	}

	RHI_SSBO* OpenGL::create_ssbo(size_t size, const byte* data, RHIBufferType type)
	{
		return new OpenGL_SSBO(size, data);
	}

	OpenGL& OpenGL::initialize_ubo()
	{
		m_global_ubo = new OpenGL_UniformBuffer(sizeof(GlobalShaderParameters));
		m_local_ubo  = new OpenGL_LocalUniformBuffer();
		return *this;
	}

	OpenGL& OpenGL::push_global_params(const GlobalShaderParameters& params)
	{
		m_global_parameters_stack.push_back(params);
		m_global_ubo->update(0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
		return *this;
	}

	OpenGL& OpenGL::pop_global_params()
	{
		if (!m_global_parameters_stack.empty())
		{
			m_global_parameters_stack.pop_back();
			if (!m_global_parameters_stack.empty())
			{
				GlobalShaderParameters& params = m_global_parameters_stack.back();
				m_global_ubo->update(0, sizeof(GlobalShaderParameters), reinterpret_cast<const byte*>(&params));
			}
		}
		return *this;
	}

	OpenGL& OpenGL::update_local_parameter(const void* data, size_t size, size_t offset)
	{
		m_local_ubo->update(data, size, offset);
		return *this;
	}

}// namespace Engine
