#include <Core/memory.hpp>
#include <cstring>
#include <opengl_api.hpp>
#include <opengl_buffers.hpp>

#define MAX_DEFAULT_UBO_SIZE 4096

namespace Engine
{
	struct OpenGL_UniformBuffer {
		GLuint m_id;
		size_t m_size;

		OpenGL_UniformBuffer(size_t size) : m_size(size)
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

		~OpenGL_UniformBuffer()
		{
			glDeleteBuffers(1, &m_id);
		}
	};

	struct OpenGL_UniformBufferArray {
		Vector<OpenGL_UniformBuffer*> m_buffers;
		GLsync m_fence = 0;

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

		void submit()
		{
			m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		}

		~OpenGL_UniformBufferArray()
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

	struct OpenGL_GlobalUniformBuffer : OpenGL_UniformBufferArray {
		Vector<size_t> m_stack;
		int_t m_index = -1;

		OpenGL_UniformBuffer* buffer()
		{
			return m_buffers[m_stack.back()];
		}

		void bind(BindingIndex index)
		{
			buffer()->bind(index, 0, sizeof(GlobalShaderParameters));
		}

		void push(const GlobalShaderParameters& params)
		{
			++m_index;
			m_stack.push_back(m_index);

			if (static_cast<int_t>(m_buffers.size()) <= m_index)
			{
				m_buffers.push_back(new OpenGL_UniformBuffer(sizeof(GlobalShaderParameters)));
			}

			buffer()->update(0, sizeof(params), reinterpret_cast<const byte*>(&params));
		}

		void pop()
		{
			if (!m_stack.empty())
				m_stack.pop_back();
		}

		void submit()
		{
			OpenGL_UniformBufferArray::submit();
			m_index = -1;
		}
	};

	struct OpenGL_LocalUniformBuffer : OpenGL_UniformBufferArray {
		Vector<byte> m_shadow_data;
		size_t m_shadow_data_size = 0;
		size_t m_used_data_size   = 0;
		size_t m_index            = 0;


		OpenGL_LocalUniformBuffer()
		{
			m_buffers.push_back(new OpenGL_UniformBuffer(MAX_DEFAULT_UBO_SIZE));
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
					m_buffers.push_back(new OpenGL_UniformBuffer(glm::max<size_t>(m_shadow_data_size, MAX_DEFAULT_UBO_SIZE)));
				}
			}

			OpenGL_UniformBuffer* buffer = m_buffers[m_index];
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
			OpenGL_UniformBufferArray::submit();
			m_used_data_size   = 0;
			m_shadow_data_size = 0;
			m_index            = 0;
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

	OpenGL_GlobalUniformBuffer* OpenGL_GlobalUniformBufferManager::buffer()
	{
		return find_buffer_internal(m_buffers, m_index);
	}

	void OpenGL_GlobalUniformBufferManager::bind(BindingIndex index)
	{
		buffer()->bind(index);
	}

	void OpenGL_GlobalUniformBufferManager::push(const GlobalShaderParameters& params)
	{
		buffer()->push(params);
	}

	void OpenGL_GlobalUniformBufferManager::pop()
	{
		buffer()->pop();
	}

	void OpenGL_GlobalUniformBufferManager::submit()
	{
		if (m_index != -1)
		{
			buffer()->submit();
			m_index = -1;
		}
	}

	OpenGL_GlobalUniformBufferManager::~OpenGL_GlobalUniformBufferManager()
	{
		for (auto buffer : m_buffers)
		{
			delete buffer;
		}
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

	OpenGL& OpenGL::push_global_params(const GlobalShaderParameters& params)
	{
		m_global_ubo->push(params);
		return *this;
	}

	OpenGL& OpenGL::pop_global_params()
	{
		m_global_ubo->pop();
		return *this;
	}

	OpenGL& OpenGL::update_local_parameter(const void* data, size_t size, size_t offset)
	{
		m_local_ubo->update(data, size, offset);
		return *this;
	}

	OpenGL& OpenGL::initialize_ubo()
	{
		m_global_ubo = new OpenGL_GlobalUniformBufferManager();
		m_local_ubo  = new OpenGL_LocalUniformBufferManager();
		return *this;
	}
}// namespace Engine
