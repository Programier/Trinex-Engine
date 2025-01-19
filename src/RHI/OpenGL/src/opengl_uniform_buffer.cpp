#include <Core/memory.hpp>
#include <cstring>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>

#define MAX_DEFAULT_UBO_SIZE 4096

namespace Engine
{
	struct OpenGL_UniformBufferBlock {
		GLuint m_id;
		size_t m_size;

		OpenGL_UniformBufferBlock(size_t size) : m_size(size)
		{
			glGenBuffers(1, &m_id);
			glBindBuffer(GL_UNIFORM_BUFFER, m_id);
			glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_DYNAMIC_COPY);
		}

		void bind(BindingIndex index, size_t offset, size_t size)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, index, m_id, offset, size);
		}

		void update(size_t offset, size_t size, const byte* data)
		{
			glBindBuffer(GL_UNIFORM_BUFFER, m_id);

			if (auto mapped = glMapBufferRange(GL_UNIFORM_BUFFER, offset, size, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT))
			{
				std::memcpy(reinterpret_cast<byte*>(mapped), data, size);
				glUnmapBuffer(GL_UNIFORM_BUFFER);
			}
		}

		~OpenGL_UniformBufferBlock()
		{
			glDeleteBuffers(1, &m_id);
		}
	};

	struct OpenGL_LocalUniformBuffer {
		Vector<byte> m_shadow_data;
		Vector<OpenGL_UniformBufferBlock*> m_buffers;
		GLsync m_fence            = 0;
		size_t m_shadow_data_size = 0;
		size_t m_used_data_size   = 0;
		size_t m_index            = 0;


		OpenGL_LocalUniformBuffer()
		{
			m_buffers.push_back(new OpenGL_UniformBufferBlock(MAX_DEFAULT_UBO_SIZE));
		}

		bool is_busy()
		{
			if (m_fence == 0)
				return false;

			GLint sync_status = 0;
			glGetSynciv(m_fence, GL_SYNC_STATUS, sizeof(sync_status), nullptr, &sync_status);

			if (sync_status == GL_SIGNALED)
			{
				glDeleteSync(m_fence);
				m_fence = 0;
				return false;
			}
			return true;
		}

		void bind(BindingIndex binding_index)
		{
			if (m_shadow_data_size == 0)
			{
				glBindBufferBase(GL_UNIFORM_BUFFER, binding_index, 0);
				return;
			}

			while (m_buffers[m_index]->m_size < m_used_data_size + m_shadow_data_size)
			{
				++m_index;
				m_used_data_size = 0;

				if (m_buffers.size() <= m_index)
				{
					m_buffers.push_back(
							new OpenGL_UniformBufferBlock(glm::max<size_t>(m_shadow_data_size, MAX_DEFAULT_UBO_SIZE)));
				}
			}

			OpenGL_UniformBufferBlock* buffer = m_buffers[m_index];
			buffer->update(m_used_data_size, m_shadow_data_size, m_shadow_data.data());
			buffer->bind(binding_index, m_used_data_size, m_shadow_data_size);

			m_used_data_size   = align_memory(m_used_data_size + m_shadow_data_size, OPENGL_API->m_uniform_alignment);
			m_shadow_data_size = 0;
		}

		void update(const void* data, size_t size, size_t offset)
		{
			m_shadow_data_size = glm::max(size + offset, m_shadow_data_size);
			if (m_shadow_data.size() < m_shadow_data_size)
				m_shadow_data.resize(m_shadow_data_size);

			std::memcpy(m_shadow_data.data() + offset, data, size);
		}

		void submit()
		{
			m_fence            = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			m_used_data_size   = 0;
			m_shadow_data_size = 0;
			m_index            = 0;
		}

		~OpenGL_LocalUniformBuffer()
		{
			for (auto& ell : m_buffers)
			{
				delete ell;
			}

			m_buffers.clear();

			if (m_fence != 0)
			{
				glDeleteSync(m_fence);
			}
		}
	};

	template<typename BufferType>
	BufferType* find_buffer_internal(Vector<BufferType*>& buffers, int_t& index)
	{
		if (index != -1)
			return buffers[index];

		for (auto buffer : buffers)
		{
			++index;

			if (!buffer->is_busy())
			{
				return buffer;
			}
		}

		++index;
		buffers.push_back(new BufferType());
		return buffers.back();
	}

	OpenGL_LocalUniformBuffer* OpenGL_LocalUniformBufferManager::buffer()
	{
		return find_buffer_internal(m_buffers, m_index);
	}

	void OpenGL_LocalUniformBufferManager::bind(BindingIndex index)
	{
		buffer()->bind(index);
	}

	void OpenGL_LocalUniformBufferManager::update(const void* data, size_t size, size_t offset)
	{
		buffer()->update(data, size, offset);
	}

	void OpenGL_LocalUniformBufferManager::submit()
	{
		if (m_index != -1)
		{
			buffer()->submit();
			m_index = -1;
		}
	}

	OpenGL_LocalUniformBufferManager::~OpenGL_LocalUniformBufferManager()
	{
		for (auto buffer : m_buffers)
		{
			delete buffer;
		}
	}

	OpenGL& OpenGL::update_scalar_parameter(const void* data, size_t size, size_t offset)
	{
		m_local_ubo->update(data, size, offset);
		return *this;
	}

	OpenGL& OpenGL::initialize_ubo()
	{
		m_local_ubo = new OpenGL_LocalUniformBufferManager();
		return *this;
	}
}// namespace Engine
