#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Graphics/pipeline.hpp>
#include <d3d11_api.hpp>
#include <d3d11_buffer.hpp>
#include <d3d11_pipeline.hpp>
#include <d3d11_uniform_buffer.hpp>

namespace Engine
{
	class D3D11_LocalUniformBuffer
	{
		D3D11_UniformBuffer m_uniform_buffer;
		Vector<byte> m_shadow_data;
		size_t m_buffer_size = 0;
		size_t m_shadow_size = 0;

	public:
		void create()
		{
			if (m_shadow_size > m_buffer_size)
			{
				release();
				m_buffer_size = glm::max(m_shadow_size, static_cast<size_t>(1024));
				m_uniform_buffer.init(m_buffer_size, nullptr, RHIBufferType::Dynamic);
			}
		}

		void update(const void* data, size_t size, size_t offset)
		{
			size_t max_size = offset + size;

			if (m_shadow_data.size() < max_size)
			{
				m_shadow_data.resize(max_size);
			}

			m_shadow_size = glm::max(m_shadow_size, max_size);
			std::memcpy(m_shadow_data.data() + offset, data, size);
		}

		void bind(BindingIndex index)
		{
			if (m_shadow_size > 0)
			{
				if (m_buffer_size < m_shadow_size)
				{
					create();
				}

				m_uniform_buffer.update(0, m_shadow_size, m_shadow_data.data());
				m_uniform_buffer.bind(index);

				m_shadow_size = 0;
			}
		}

		void release()
		{
			d3d11_release(m_uniform_buffer.m_buffer);
			m_buffer_size = 0;
		}
	};

	D3D11_UniformBufferManager& D3D11_UniformBufferManager::update(const void* data, size_t size, size_t offset,
	                                                               BindingIndex buffer_index)
	{
		if (m_buffers.size() <= static_cast<size_t>(buffer_index))
			m_buffers.resize(static_cast<size_t>(buffer_index) + 1, nullptr);

		D3D11_LocalUniformBuffer*& buffer = m_buffers[buffer_index];

		if (buffer == nullptr)
			buffer = new D3D11_LocalUniformBuffer();
		buffer->update(data, size, offset);
		return *this;
	}

	D3D11_UniformBufferManager& D3D11_UniformBufferManager::bind()
	{
		BindingIndex index = 0;

		for (auto* buffer : m_buffers)
		{
			if (buffer)
				buffer->bind(index);
			++index;
		}

		return *this;
	}

	D3D11_UniformBufferManager::~D3D11_UniformBufferManager()
	{
		for (auto* buffer : m_buffers)
		{
			if (buffer)
				delete buffer;
		}
	}

	D3D11& D3D11::update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		m_unifor_buffer_manager->update(data, size, offset, buffer_index);
		return *this;
	}
}// namespace Engine
