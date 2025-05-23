#include <Graphics/pipeline.hpp>
#include <Graphics/shader.hpp>
#include <Graphics/shader_parameters.hpp>
#include <cstring>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>
#include <opengl_shader.hpp>

namespace Engine
{
	OpenGL_Buffer::OpenGL_Buffer(size_t size, const byte* data, BufferCreateFlags flags)
	{
		if (flags & (BufferCreateFlags::StructuredBuffer | BufferCreateFlags::ByteAddressBuffer))
		{
			m_target = GL_SHADER_STORAGE_BUFFER;
		}
		else if (flags & BufferCreateFlags::IndexBuffer)
		{
			m_target = GL_ELEMENT_ARRAY_BUFFER;
		}
		else
		{
			m_target = GL_ARRAY_BUFFER;
		}

		glGenBuffers(1, &m_id);
		glBindBuffer(m_target, m_id);
		glBufferData(m_target, size, data, (flags & BufferCreateFlags::Dynamic) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);


		if (flags & BufferCreateFlags::ShaderResource)
		{
			if (flags & (BufferCreateFlags::ByteAddressBuffer | BufferCreateFlags::StructuredBuffer))
				m_srv = new OpenGL_BufferSRV<GL_SHADER_STORAGE_BUFFER>(m_id);
			else
				m_srv = new OpenGL_BufferSRV<GL_TEXTURE_BUFFER>(m_id);
		}

		if (flags & BufferCreateFlags::UnorderedAccess)
		{
			if (flags & (BufferCreateFlags::ByteAddressBuffer | BufferCreateFlags::StructuredBuffer))
			{
				m_uav = new OpenGL_BufferUAV<GL_SHADER_STORAGE_BUFFER>(m_id);
			}
		}
	}

	OpenGL_Buffer::~OpenGL_Buffer()
	{
		glDeleteBuffers(1, &m_id);

		if (m_srv)
			delete m_srv;
		if (m_uav)
			delete m_uav;
	}

	byte* OpenGL_Buffer::map()
	{
		if (m_mapped == nullptr)
		{
			glBindBuffer(m_target, m_id);
			m_mapped = reinterpret_cast<byte*>(glMapBuffer(m_target, GL_READ_WRITE));
		}

		return m_mapped;
	}

	void OpenGL_Buffer::unmap()
	{
		if (m_mapped)
		{
			glBindBuffer(m_target, m_id);
			glUnmapBuffer(m_target);
			m_mapped = nullptr;
		}
	}

	OpenGL_Buffer& OpenGL_Buffer::update(size_t offset, size_t size, const byte* data)
	{
		glBindBuffer(m_target, m_id);
		glBufferSubData(m_target, offset, size, data);
		return *this;
	}

	RHI_Buffer* OpenGL::create_buffer(size_t size, const byte* data, BufferCreateFlags flags)
	{
		return new OpenGL_Buffer(size, data, flags);
	}

	OpenGL& OpenGL::bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream)
	{
		OpenGL_Buffer* gl_buffer = static_cast<OpenGL_Buffer*>(buffer);
		glBindVertexBuffer(stream, gl_buffer->m_id, byte_offset, stride);
		return *this;
	}

	OpenGL& OpenGL::bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format)
	{
		OpenGL_Buffer* gl_buffer = static_cast<OpenGL_Buffer*>(buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_buffer->m_id);
		m_state.index_type = format == RHIIndexFormat::UInt16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		return *this;
	}

	OpenGL& OpenGL::bind_uniform_buffer(RHI_Buffer* buffer, byte slot)
	{
		OpenGL_Buffer* gl_buffer = static_cast<OpenGL_Buffer*>(buffer);
		glBindBufferBase(GL_UNIFORM_BUFFER, slot, gl_buffer->m_id);
		return *this;
	}

	OpenGL& OpenGL::update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data)
	{
		static_cast<OpenGL_Buffer*>(buffer)->update(offset, size, data);
		return *this;
	}

	OpenGL& OpenGL::copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset, size_t dst_offset)
	{
		GLuint src_buffer = static_cast<OpenGL_Buffer*>(src)->m_id;
		GLuint dst_buffer = static_cast<OpenGL_Buffer*>(dst)->m_id;

		glBindBuffer(GL_COPY_READ_BUFFER, src_buffer);
		glBindBuffer(GL_COPY_WRITE_BUFFER, dst_buffer);

		glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, static_cast<GLintptr>(src_offset),
		                    static_cast<GLintptr>(dst_offset), static_cast<GLsizeiptr>(size));
		return *this;
	}

}// namespace Engine
