#include <Core/memory.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_uniform_buffer.hpp>

namespace Engine
{
	static constexpr size_t default_uniform_buffer_size = 4096;

	class VulkanDynamicUniformBuffer
	{
	private:
		Vector<VulkanBuffer*> m_buffers;
		Vector<byte> m_shadow_data;

		size_t m_shadow_data_size = 0;
		size_t m_index            = 0;
		size_t m_used_data        = 0;

		void allocate(size_t size)
		{
			VulkanBuffer* buffer = new VulkanBuffer();

			if (size < default_uniform_buffer_size)
				size = default_uniform_buffer_size;

			buffer->create(size, nullptr, BufferCreateFlags::UniformBuffer | BufferCreateFlags::CPUWrite,
			               VMA_MEMORY_USAGE_CPU_TO_GPU);
			m_buffers.push_back(buffer);
		}

	public:
		VulkanDynamicUniformBuffer() { allocate(default_uniform_buffer_size); }

		void update(const void* data, size_t size, size_t offset)
		{
			m_shadow_data_size = glm::max(size + offset, m_shadow_data_size);

			if (m_shadow_data.size() < m_shadow_data_size)
			{
				m_shadow_data.resize(m_shadow_data_size);
			}

			std::memcpy(m_shadow_data.data() + offset, data, size);
		}

		void bind(BindingIndex index)
		{
			if (m_shadow_data_size == 0)
				return;

			while (m_buffers[m_index]->m_size < m_used_data + m_shadow_data_size)
			{
				++m_index;
				m_used_data = 0;

				if (m_buffers.size() <= m_index)
				{
					allocate(m_shadow_data_size);
				}
			}

			auto& current_buffer = m_buffers[m_index];
			current_buffer->copy(m_used_data, m_shadow_data.data(), m_shadow_data_size);

			API->bind_uniform_buffer(current_buffer, m_shadow_data_size, m_used_data, index);
			m_used_data =
			        align_memory(m_used_data + m_shadow_data_size, API->m_properties.limits.minUniformBufferOffsetAlignment);

			m_shadow_data_size = 0;
		}

		void reset()
		{
			m_shadow_data_size = 0;
			m_index            = 0;
			m_used_data        = 0;
		}

		~VulkanDynamicUniformBuffer()
		{
			for (VulkanBuffer* buffer : m_buffers)
			{
				buffer->release();
			}
		}
	};

	void VulkanUniformBufferManager::reset()
	{
		for (VulkanDynamicUniformBuffer* buffer : m_buffers)
		{
			if (buffer)
			{
				buffer->reset();
			}
		}
	}

	void VulkanUniformBufferManager::update(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		if (static_cast<size_t>(buffer_index) >= m_buffers.size())
		{
			m_buffers.resize(static_cast<size_t>(buffer_index) + 1, nullptr);
		}

		VulkanDynamicUniformBuffer*& buffer = m_buffers[buffer_index];

		if (!buffer)
		{
			buffer = new VulkanDynamicUniformBuffer();
		}

		buffer->update(data, size, offset);
	}

	void VulkanUniformBufferManager::bind()
	{
		BindingIndex index = 0;
		for (VulkanDynamicUniformBuffer* buffer : m_buffers)
		{
			if (buffer)
			{
				buffer->bind(index);
				++index;
			}
		}
	}

	VulkanUniformBufferManager::~VulkanUniformBufferManager()
	{
		for (VulkanDynamicUniformBuffer* buffer : m_buffers)
		{
			if (buffer)
			{
				delete buffer;
			}
		}
	}

	VulkanAPI& VulkanAPI::update_scalar_parameter(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		API->uniform_buffer_manager()->update(data, size, offset, buffer_index);
		return *this;
	}
}// namespace Engine
