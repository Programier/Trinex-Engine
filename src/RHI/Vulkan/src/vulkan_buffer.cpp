#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#include <Core/exception.hpp>
#include <Core/memory.hpp>
#include <Graphics/shader_parameters.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
	VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, const byte* data, vk::BufferUsageFlags usage,
	                                   VmaMemoryUsage memory_usage)
	{
		m_size  = size;
		m_usage = vk::BufferUsageFlagBits::eTransferDst | usage;
		vk::BufferCreateInfo buffer_info({}, size, m_usage, vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = memory_usage;
		VkBuffer out_buffer                = VK_NULL_HANDLE;
		auto res = vmaCreateBuffer(API->m_allocator, &static_cast<VkBufferCreateInfo&>(buffer_info), &alloc_info, &out_buffer,
		                           &m_allocation, nullptr);
		m_buffer = out_buffer;
		trinex_check(res == VK_SUCCESS, "Failed to create buffer");

		if (data)
		{
			copy(0, data, size);
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::copy(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
	{
		if (m_allocation->IsMappingAllowed())
		{
			auto res = vmaCopyMemoryToAllocation(API->m_allocator, data, m_allocation, offset, size);
			trinex_check(res == VK_SUCCESS, "Failed to copy data");
		}
		else
		{
			auto buffer = API->m_stagging_manager->allocate(size, vk::BufferUsageFlagBits::eTransferSrc);
			buffer->copy(offset, data, size);

			auto cmd = API->begin_single_time_command_buffer();

			vk::BufferCopy region(0, offset, size);
			cmd.copyBuffer(buffer->m_buffer, m_buffer, region);
			API->end_single_time_command_buffer(cmd);
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::update(vk::DeviceSize offset, const byte* data, vk::DeviceSize size)
	{
		auto cmd = API->current_command_buffer();
		if (cmd->is_inside_render_pass())
			API->end_render_pass();

		{
			const vk::MemoryBarrier barrier(vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead);
			cmd->m_cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, barrier,
			                           {}, {});
		}
		
		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdUpdateBuffer.html
		if (size <= 65536 && size % 4 == 0 && offset % 4 == 0)
		{
			cmd->m_cmd.updateBuffer(m_buffer, offset, size, data);
		}
		else
		{
			auto staging = API->m_stagging_manager->allocate(size, vk::BufferUsageFlagBits::eTransferSrc);
			staging->copy(0, data, size);

			vk::BufferCopy region(0, offset, size);
			cmd->m_cmd.copyBuffer(staging->m_buffer, m_buffer, region);
			cmd->add_object(staging);
		}

		{
			const vk::MemoryBarrier barrier(vk::AccessFlagBits::eMemoryWrite,
			                                vk::AccessFlagBits::eMemoryRead | vk::AccessFlagBits::eMemoryRead);
			cmd->m_cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eAllCommands, {}, barrier,
			                           {}, {});
		}

		return *this;
	}

	byte* VulkanBuffer::map_memory()
	{
		union
		{
			void* mapped = nullptr;
			byte* data;
		};

		auto res = vmaMapMemory(API->m_allocator, m_allocation, &mapped);
		trinex_check(res == VK_SUCCESS, "Failed to map buffer");
		return data;
	}

	VulkanBuffer& VulkanBuffer::unmap_memory()
	{
		vmaUnmapMemory(API->m_allocator, m_allocation);
		return *this;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (m_allocation)
			vmaDestroyBuffer(API->m_allocator, m_buffer, m_allocation);
	}

	VulkanBuffer* VulkanStaggingBufferManager::allocate(vk::DeviceSize size, vk::BufferUsageFlags usage,
	                                                    VmaMemoryUsage memory_usage)
	{
		for (size_t i = 0, size = m_free.size(); i < size; i++)
		{
			auto buffer = m_free[i].m_buffer;
			if (buffer->references() == 1 && buffer->m_size >= size && (buffer->m_usage & usage) == usage)
			{
				m_free.erase(m_free.begin() + i);
				return buffer;
			}
		}

		VulkanBuffer* buffer = new VulkanBuffer();
		buffer->create(size, nullptr, usage, memory_usage);
		m_buffers.push_back(buffer);
		return buffer;
	}

	VulkanStaggingBufferManager& VulkanStaggingBufferManager::release(VulkanBuffer* buffer)
	{
		m_free.emplace_back(buffer, VK_STAGGING_RESOURCE_WAIT_FRAMES);
		return *this;
	}

	VulkanStaggingBufferManager& VulkanStaggingBufferManager::update()
	{
		for (size_t i = 0, size = m_free.size(); i < size;)
		{
			auto& entry = m_free[i];
			--entry.m_frame_number;

			if (entry.m_frame_number == 0)
			{
				entry.m_buffer->release();
				m_free.erase(m_free.begin() + i);
				--size;
			}
			else
			{
				++i;
			}
		}
		return *this;
	}

	VulkanStaggingBufferManager::~VulkanStaggingBufferManager()
	{
		for (auto buffer : m_buffers)
		{
			buffer->release();
		}

		m_buffers.clear();
	}

	VulkanVertexBuffer& VulkanVertexBuffer::create(const byte* data, size_t size)
	{
		m_buffer.create(size, data, vk::BufferUsageFlagBits::eVertexBuffer);
		return *this;
	}

	void VulkanVertexBuffer::bind(byte stream_index, size_t stride, size_t offset)
	{
		RHI_VertexBuffer*& current = API->m_state.m_current_vertex_buffer[stream_index];
		if (current != this)
		{
			auto cmd = API->current_command_buffer();
			cmd->m_cmd.bindVertexBuffers(stream_index, m_buffer.m_buffer, {offset});
			cmd->add_object(this);
			current = this;
		}
	}

	void VulkanVertexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		m_buffer.update(offset, data, size);
	}

	VulkanIndexBuffer& VulkanIndexBuffer::create(const byte* data, size_t size, IndexBufferFormat format)
	{
		m_type = format == IndexBufferFormat::UInt32 ? vk::IndexType::eUint32 : vk::IndexType::eUint16;
		m_buffer.create(size, data, vk::BufferUsageFlagBits::eIndexBuffer);
		return *this;
	}

	void VulkanIndexBuffer::bind(size_t offset)
	{
		RHI_IndexBuffer*& current = API->m_state.m_current_index_buffer;
		if (current != this)
		{
			auto cmd = API->current_command_buffer();
			cmd->m_cmd.bindIndexBuffer(m_buffer.m_buffer, offset, m_type);
			cmd->add_object(this);
			current = this;
		}
	}

	void VulkanIndexBuffer::update(size_t offset, size_t size, const byte* data)
	{
		m_buffer.update(offset, data, size);
	}

	VulkanSSBO& VulkanSSBO::create(const byte* data, size_t size)
	{
		m_buffer.create(size, data, vk::BufferUsageFlagBits::eStorageBuffer);
		return *this;
	}

	void VulkanSSBO::bind(BindLocation location)
	{
		if (API->m_state.m_pipeline)
		{
			API->m_state.m_pipeline->bind_ssbo(this, location);
		}
	}

	void VulkanSSBO::update(size_t offset, size_t size, const byte* data)
	{
		m_buffer.update(offset, data, size);
	}

	RHI_VertexBuffer* VulkanAPI::create_vertex_buffer(size_t size, const byte* data, RHIBufferType type)
	{
		return &(new VulkanVertexBuffer())->create(data, size);
	}

	RHI_IndexBuffer* VulkanAPI::create_index_buffer(size_t size, const byte* data, IndexBufferFormat format, RHIBufferType type)
	{
		return &(new VulkanIndexBuffer())->create(data, size, format);
	}

	RHI_SSBO* VulkanAPI::create_ssbo(size_t size, const byte* data, RHIBufferType type)
	{
		return &(new VulkanSSBO())->create(data, size);
	}
}// namespace Engine
