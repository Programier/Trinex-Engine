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
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_state.hpp>
#include <vulkan_types.hpp>

namespace Engine
{
	VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, const byte* data, BufferCreateFlags flags,
	                                   VmaMemoryUsage memory_usage)
	{
		m_size  = size;
		m_flags = flags;

		vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eTransferDst;

		if (flags & BufferCreateFlags::VertexBuffer)
			usage |= vk::BufferUsageFlagBits::eVertexBuffer;

		if (flags & BufferCreateFlags::IndexBuffer)
			usage |= vk::BufferUsageFlagBits::eIndexBuffer;

		if (flags & BufferCreateFlags::UniformBuffer)
			usage |= vk::BufferUsageFlagBits::eUniformBuffer;


		if (flags & BufferCreateFlags::ShaderResource)
		{
			if (flags & (BufferCreateFlags::ByteAddressBuffer | BufferCreateFlags::StructuredBuffer))
			{
				usage |= vk::BufferUsageFlagBits::eStorageBuffer;
				m_srv = new TypedVulkanBufferSRV<vk::DescriptorType::eStorageBuffer>(this);
			}
			else
			{
				usage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
				m_srv = new TypedVulkanBufferSRV<vk::DescriptorType::eUniformTexelBuffer>(this);
			}
		}

		if (flags & BufferCreateFlags::UnorderedAccess)
		{
			if (flags & (BufferCreateFlags::ByteAddressBuffer | BufferCreateFlags::StructuredBuffer))
			{
				usage |= vk::BufferUsageFlagBits::eStorageBuffer;
				m_uav = new TypedVulkanBufferUAV<vk::DescriptorType::eStorageBuffer>(this);
			}
		}

		if (flags & BufferCreateFlags::TransferSrc)
			usage |= vk::BufferUsageFlagBits::eTransferSrc;

		if (flags & BufferCreateFlags::TransferDst)
			usage |= vk::BufferUsageFlagBits::eTransferDst;


		vk::BufferCreateInfo buffer_info({}, size, usage, vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = memory_usage;

		if ((flags & BufferCreateFlags::CPURead) || (flags & BufferCreateFlags::CPUWrite))
		{
			alloc_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		VkBuffer out_buffer = VK_NULL_HANDLE;
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
		if (map() != nullptr)
		{
			std::memcpy(m_mapped + offset, data, size);
		}
		else
		{
			auto buffer = API->m_stagging_manager->allocate(size, BufferCreateFlags::TransferSrc);
			buffer->copy(offset, data, size);

			transition(RHIAccess::CopyDst);
			auto cmd = API->end_render_pass();

			vk::BufferCopy region(0, offset, size);
			cmd->copyBuffer(buffer->buffer(), m_buffer, region);
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::update(size_t offset, size_t size, const byte* data)
	{
		auto cmd = API->end_render_pass();
		transition(RHIAccess::CopyDst);

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdUpdateBuffer.html
		if (size <= 65536 && size % 4 == 0 && offset % 4 == 0)
		{
			cmd->updateBuffer(m_buffer, offset, size, data);
		}
		else
		{
			auto staging = API->m_stagging_manager->allocate(size, BufferCreateFlags::TransferSrc);
			staging->copy(0, data, size);

			vk::BufferCopy region(0, offset, size);
			cmd->copyBuffer(staging->m_buffer, m_buffer, region);
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::transition(RHIAccess access)
	{
		if (m_access == access)
			return *this;

		if (m_access == RHIAccess::Undefined)
		{
			m_access = access;
			return *this;
		}

		API->end_render_pass();

		const vk::PipelineStageFlags src_stage = VulkanEnums::pipeline_stage_of(m_access);
		const vk::PipelineStageFlags dst_stage = VulkanEnums::pipeline_stage_of(access);
		const vk::AccessFlags src_access       = VulkanEnums::access_of(m_access);
		const vk::AccessFlags dst_access       = VulkanEnums::access_of(access);

		vk::BufferMemoryBarrier barrier(src_access, dst_access, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_buffer, 0,
		                                VK_WHOLE_SIZE);

		API->current_command_buffer()->pipelineBarrier(src_stage, dst_stage, {}, {}, barrier, {});
		m_access = access;
		return *this;
	}

	byte* VulkanBuffer::map()
	{
		if (m_mapped == nullptr && m_allocation->IsMappingAllowed())
		{
			auto res = vmaMapMemory(API->m_allocator, m_allocation, reinterpret_cast<void**>(&m_mapped));
			trinex_check(res == VK_SUCCESS, "Failed to map buffer");
		}
		return m_mapped;
	}

	void VulkanBuffer::unmap()
	{
		if (m_mapped)
		{
			vmaUnmapMemory(API->m_allocator, m_allocation);
			m_mapped = nullptr;
		}
	}

	VulkanBuffer::~VulkanBuffer()
	{
		unmap();

		if (m_allocation)
			vmaDestroyBuffer(API->m_allocator, m_buffer, m_allocation);

		if (m_srv)
			delete m_srv;

		if (m_uav)
			delete m_uav;
	}


	VulkanStaggingBuffer::VulkanStaggingBuffer(VulkanStaggingBufferManager* manager) : m_manager(manager)
	{
		m_references = 0;
	}

	void VulkanStaggingBuffer::destroy()
	{
		if (m_manager)
			m_manager->release(const_cast<VulkanStaggingBuffer*>(this));
		else
			delete this;
	}

	VulkanStaggingBuffer* VulkanStaggingBufferManager::allocate(vk::DeviceSize buffer_size, BufferCreateFlags flags)
	{
		flags |= BufferCreateFlags::CPUWrite;

		for (size_t i = 0, size = m_free.size(); i < size; i++)
		{
			auto buffer = m_free[i].m_buffer;
			if (buffer->references() == 0 && buffer->size() >= buffer_size && (buffer->flags() & flags) == flags)
			{
				m_free.erase(m_free.begin() + i);
				return buffer;
			}
		}

		VulkanStaggingBuffer* buffer = new VulkanStaggingBuffer(this);
		buffer->create(buffer_size, nullptr, flags, VMA_MEMORY_USAGE_CPU_ONLY);
		m_buffers.insert(buffer);
		return buffer;
	}

	VulkanStaggingBufferManager& VulkanStaggingBufferManager::release(VulkanStaggingBuffer* buffer)
	{
		if (buffer->m_manager == this)
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
				m_buffers.erase(entry.m_buffer);
				delete entry.m_buffer;
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
			if (buffer->references() == 0)
			{
				delete buffer;
			}
			else
			{
				buffer->m_manager = nullptr;
			}
		}

		m_buffers.clear();
	}

	RHI_Buffer* VulkanAPI::create_buffer(size_t size, const byte* data, BufferCreateFlags flags)
	{
		return &(new VulkanBuffer())->create(size, data, flags);
	}

	VulkanAPI& VulkanAPI::bind_vertex_buffer(RHI_Buffer* buffer, size_t byte_offset, uint16_t stride, byte stream)
	{
		auto* cmd = current_command_buffer();
		cmd->bindVertexBuffers(stream, static_cast<VulkanBuffer*>(buffer)->buffer(), byte_offset);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_index_buffer(RHI_Buffer* buffer, RHIIndexFormat format)
	{
		auto* cmd                   = current_command_buffer();
		VulkanBuffer* vulkan_buffer = static_cast<VulkanBuffer*>(buffer);
		cmd->bindIndexBuffer(vulkan_buffer->buffer(), 0,
		                     format == RHIIndexFormat::UInt16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_uniform_buffer(RHI_Buffer* buffer, byte slot)
	{
		VulkanBuffer* vulkan_buffer = static_cast<VulkanBuffer*>(buffer);
		return bind_uniform_buffer(vulkan_buffer, vulkan_buffer->size(), 0, slot);
	}

	VulkanAPI& VulkanAPI::bind_uniform_buffer(VulkanBuffer* buffer, size_t size, size_t offset, byte slot)
	{
		if (m_state.m_pipeline)
		{
			vk::DescriptorBufferInfo info;
			info.buffer = buffer->buffer();
			info.offset = offset;
			info.range  = size;
			m_state.m_pipeline->bind_uniform_buffer(info, slot, vk::DescriptorType::eUniformBuffer);
		}

		return *this;
	}

	VulkanAPI& VulkanAPI::barrier(RHI_Buffer* buffer, RHIAccess dst_access)
	{
		static_cast<VulkanBuffer*>(buffer)->transition(dst_access);
		return *this;
	}

	VulkanAPI& VulkanAPI::update_buffer(RHI_Buffer* buffer, size_t offset, size_t size, const byte* data)
	{
		static_cast<VulkanBuffer*>(buffer)->update(offset, size, data);
		return *this;
	}

	VulkanAPI& VulkanAPI::copy_buffer_to_buffer(RHI_Buffer* src, RHI_Buffer* dst, size_t size, size_t src_offset,
	                                            size_t dst_offset)
	{
		VulkanBuffer* src_buffer = static_cast<VulkanBuffer*>(src);
		VulkanBuffer* dst_buffer = static_cast<VulkanBuffer*>(dst);

		auto* cmd = current_command_buffer();

		vk::BufferCopy region(src_offset, dst_offset, size);
		cmd->copyBuffer(src_buffer->buffer(), dst_buffer->buffer(), region);
		return *this;
	}
}// namespace Engine
