#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <cstring>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_shader.hpp>

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
}// namespace Engine
