#define VMA_IMPLEMENTATION
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0

#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/shader_parameters.hpp>
#include <vk_mem_alloc.h>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_resource_view.hpp>

namespace Trinex
{
	VulkanBuffer& VulkanBuffer::create(vk::DeviceSize size, RHIBufferFlags flags, VmaMemoryUsage memory_usage)
	{
		m_size  = size;
		m_flags = flags;

		vk::BufferCreateInfo buffer_info({}, size, vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo alloc_info = {};
		alloc_info.usage                   = memory_usage;

		if (flags & RHIBufferFlags::VertexBuffer)
			buffer_info.usage |= vk::BufferUsageFlagBits::eVertexBuffer;

		if (flags & RHIBufferFlags::IndexBuffer)
			buffer_info.usage |= vk::BufferUsageFlagBits::eIndexBuffer;

		if (flags & RHIBufferFlags::UniformBuffer)
			buffer_info.usage |= vk::BufferUsageFlagBits::eUniformBuffer;

		if (flags & RHIBufferFlags::TransferSrc)
			buffer_info.usage |= vk::BufferUsageFlagBits::eTransferSrc;

		if (flags & RHIBufferFlags::TransferDst)
			buffer_info.usage |= vk::BufferUsageFlagBits::eTransferDst;

		if (flags & RHIBufferFlags::AccelerationStorage)
			buffer_info.usage |= vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR;

		if (flags & RHIBufferFlags::AccelerationInput)
			buffer_info.usage |= vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR;

		if (flags & RHIBufferFlags::ShaderBindingTable)
			buffer_info.usage |= vk::BufferUsageFlagBits::eShaderBindingTableKHR;

		if (flags & RHIBufferFlags::ShaderResource)
		{
			if (flags & (RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::StructuredBuffer))
				buffer_info.usage |= vk::BufferUsageFlagBits::eStorageBuffer;
			else
				buffer_info.usage |= vk::BufferUsageFlagBits::eUniformTexelBuffer;
		}

		if (flags & RHIBufferFlags::UnorderedAccess)
		{
			if (flags & (RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::StructuredBuffer))
				buffer_info.usage |= vk::BufferUsageFlagBits::eStorageBuffer;
			else
				buffer_info.usage |= vk::BufferUsageFlagBits::eStorageTexelBuffer;
		}

		if (flags & RHIBufferFlags::DeviceAddress)
		{
			buffer_info.usage |= vk::BufferUsageFlagBits::eShaderDeviceAddressKHR;
			alloc_info.flags |= VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
		}

		if ((flags & RHIBufferFlags::CPURead) || (flags & RHIBufferFlags::CPUWrite))
		{
			alloc_info.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
		}

		VkBuffer out_buffer = VK_NULL_HANDLE;
		auto res            = vmaCreateBuffer(VulkanAPI::instance()->m_allocator, &static_cast<VkBufferCreateInfo&>(buffer_info),
		                                      &alloc_info, &out_buffer, &m_allocation, nullptr);
		m_buffer            = out_buffer;
		trinex_assert_msg(res == VK_SUCCESS, "Failed to create buffer");

		if (flags & RHIBufferFlags::DeviceAddress)
		{
			m_address = VulkanAPI::instance()->m_device.getBufferAddressKHR(vk::BufferDeviceAddressInfo(m_buffer));
		}

		if (flags & RHIBufferFlags::ShaderResource)
		{
			if (flags & (RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::StructuredBuffer))
			{
				m_srv = trx_new VulkanStorageBufferSRV(this);
			}
			else
			{
				m_srv = trx_new VulkanUniformTexelBufferSRV(this);
			}
		}

		if (flags & RHIBufferFlags::UnorderedAccess)
		{
			if (flags & (RHIBufferFlags::ByteAddressBuffer | RHIBufferFlags::StructuredBuffer))
			{
				m_uav = trx_new VulkanBufferUAV(this);
			}
			else
			{
				m_uav = trx_new VulkanBufferUAV(this);
			}
		}

		return *this;
	}

	VulkanBuffer& VulkanBuffer::copy(VulkanContext* ctx, vk::DeviceSize offset, const u8* data, vk::DeviceSize size)
	{
		if (u8* mapped = VulkanBuffer::map())
		{
			std::memcpy(mapped + offset, data, size);
			VulkanBuffer::unmap();
		}
		else
		{
			const bool is_transient = ctx == nullptr;

			if (is_transient)
			{
				ctx = static_cast<VulkanContext*>(RHIContextPool::global_instance()->begin_context());
			}

			auto buffer = VulkanAPI::instance()->stagging_manager()->allocate(size, RHIBufferFlags::TransferSrc);
			buffer->copy(ctx, offset, data, size);

			barrier(ctx, RHIAccess::TransferDst);
			auto cmd = ctx->handle();

			vk::BufferCopy region(0, offset, size);
			cmd->copyBuffer(buffer->buffer(), m_buffer, region);
			cmd->add_stagging(buffer);

			if (is_transient)
			{
				RHIContextPool::global_instance()->end_context(ctx);
			}
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::update(VulkanContext* ctx, usize offset, usize size, const u8* data)
	{
		auto cmd = ctx->handle();
		barrier(ctx, RHIAccess::TransferDst);

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/vkCmdUpdateBuffer.html
		if (size <= 65536 && size % 4 == 0 && offset % 4 == 0)
		{
			cmd->updateBuffer(m_buffer, offset, size, data);
		}
		else
		{
			auto staging = VulkanAPI::instance()->stagging_manager()->allocate(size, RHIBufferFlags::TransferSrc);
			staging->copy(ctx, 0, data, size);

			vk::BufferCopy region(0, offset, size);
			cmd->copyBuffer(staging->m_buffer, m_buffer, region);
			cmd->add_stagging(staging);
		}
		return *this;
	}

	VulkanBuffer& VulkanBuffer::barrier(VulkanContext* ctx, RHIAccess access)
	{
		if ((m_access & access) == access && !(access & RHIAccess::WritableMask))
			return *this;

		if (m_access == RHIAccess::Undefined)
		{
			m_access = access;
			return *this;
		}

		const vk::PipelineStageFlags src_stage = VulkanEnums::pipeline_stage_of(m_access);
		const vk::PipelineStageFlags dst_stage = VulkanEnums::pipeline_stage_of(access);
		const vk::AccessFlags src_access       = VulkanEnums::access_of(m_access);
		const vk::AccessFlags dst_access       = VulkanEnums::access_of(access);

		vk::BufferMemoryBarrier barrier(src_access, dst_access, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_buffer, 0,
		                                VK_WHOLE_SIZE);

		ctx->handle()->pipelineBarrier(src_stage, dst_stage, {}, {}, barrier, {});
		m_access = access;
		return *this;
	}

	RHIShaderResourceView* VulkanBuffer::as_srv()
	{
		return m_srv;
	}

	RHIUnorderedAccessView* VulkanBuffer::as_uav()
	{
		return m_uav;
	}

	RHIDeviceAddress VulkanBuffer::address()
	{
		return m_address;
	}

	u8* VulkanBuffer::map(RHIMappingAccess access)
	{
		if (m_allocation->IsMappingAllowed())
		{
			void* mapped = nullptr;
			auto res     = vmaMapMemory(VulkanAPI::instance()->m_allocator, m_allocation, &mapped);
			trinex_assert_msg(res == VK_SUCCESS, "Failed to map buffer");
			return reinterpret_cast<u8*>(mapped);
		}
		return nullptr;
	}

	void VulkanBuffer::unmap()
	{
		if (m_allocation->IsMappingAllowed())
		{
			vmaUnmapMemory(VulkanAPI::instance()->m_allocator, m_allocation);
		}
	}

	usize VulkanBuffer::size() const
	{
		return m_size;
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (m_allocation)
			vmaDestroyBuffer(VulkanAPI::instance()->m_allocator, m_buffer, m_allocation);

		if (m_srv)
			trx_delete m_srv;

		if (m_uav)
			trx_delete m_uav;
	}

	VulkanUniformBuffer::VulkanUniformBuffer(usize min_size)
	{
		static constexpr usize uniform_buffer_page_size = 1024 * 32;// 32 KB

		constexpr auto flags =
		        RHIBufferFlags::UniformBuffer | RHIBufferFlags::CPURead | RHIBufferFlags::CPUWrite;
		create(Math::max(uniform_buffer_page_size, min_size), flags);
		m_memory = m_block_start = m_block_end = map();
	}

	VulkanUniformBuffer::~VulkanUniformBuffer()
	{
		unmap();
	}

	VulkanUniformBuffer& VulkanUniformBuffer::flush()
	{
		m_block_start = m_block_end =
		        align_up_ptr(m_block_end, VulkanAPI::instance()->m_properties.limits.minUniformBufferOffsetAlignment);
		return *this;
	}

	VulkanUniformBuffer& VulkanUniformBuffer::update(const void* data, usize size, usize offset)
	{
		std::memcpy(m_block_start + offset, data, size);
		m_block_end = std::max<u8*>(m_block_end, m_block_start + offset + size);
		m_block_end = align_up_ptr(m_block_end, VulkanAPI::instance()->m_properties.limits.minUniformBufferOffsetAlignment);
		return *this;
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
			trx_delete this;
	}

	VulkanStaggingBuffer* VulkanStaggingBufferManager::allocate(vk::DeviceSize buffer_size, RHIBufferFlags flags)
	{
		flags |= RHIBufferFlags::CPUWrite;

		for (usize i = 0, size = m_free.size(); i < size; i++)
		{
			auto buffer = m_free[i].m_buffer;
			if (buffer->size() >= buffer_size && (buffer->flags() & flags) == flags)
			{
				m_free.erase(m_free.begin() + i);
				return buffer;
			}
		}

		VulkanStaggingBuffer* buffer = trx_new VulkanStaggingBuffer(this);
		buffer->create(buffer_size, flags, VMA_MEMORY_USAGE_CPU_ONLY);
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
		for (usize i = 0, size = m_free.size(); i < size;)
		{
			auto& entry = m_free[i];
			--entry.m_frame_number;

			if (entry.m_frame_number == 0)
			{
				m_buffers.erase(entry.m_buffer);
				trx_delete entry.m_buffer;
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
				trx_delete buffer;
			}
			else
			{
				buffer->m_manager = nullptr;
			}
		}

		m_buffers.clear();
	}

	RHIBuffer* VulkanAPI::create_buffer(usize size, RHIBufferFlags flags)
	{
		return &(trx_new VulkanBuffer())->create(size, flags);
	}

	VulkanContext& VulkanContext::bind_vertex_buffer(RHIBuffer* buffer, usize byte_offset, u16 stride, u8 stream,
	                                                 RHIVertexInputRate rate)
	{
		m_cmd->bindVertexBuffers(stream, static_cast<VulkanBuffer*>(buffer)->buffer(), byte_offset);

		VulkanContext::VertexStream vertex_stream;
		vertex_stream.stride = stride;
		vertex_stream.rate   = VulkanEnums::input_rate_of(rate);
		vertex_streams.bind(vertex_stream, stream);
		return *this;
	}

	VulkanContext& VulkanContext::bind_index_buffer(RHIBuffer* buffer, RHIIndexFormat format, usize byte_offset)
	{
		VulkanBuffer* vulkan_buffer = static_cast<VulkanBuffer*>(buffer);
		m_cmd->bindIndexBuffer(vulkan_buffer->buffer(), byte_offset,
		                       format == RHIIndexFormat::UInt16 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);
		return *this;
	}

	VulkanContext& VulkanContext::bind_uniform_buffer(RHIBuffer* buffer, u8 slot)
	{
		VulkanBuffer* vulkan_buffer = static_cast<VulkanBuffer*>(buffer);
		return bind_uniform_buffer(vulkan_buffer, vulkan_buffer->size(), 0, slot);
	}

	VulkanContext& VulkanContext::bind_uniform_buffer(VulkanBuffer* buffer, u32 size, u32 offset, u8 slot)
	{
		uniform_buffers.bind(VulkanContext::UniformBuffer(buffer->buffer(), size, offset), slot);
		return *this;
	}

	VulkanContext& VulkanContext::barrier(RHIBuffer* buffer, RHIAccess dst_access)
	{
		static_cast<VulkanBuffer*>(buffer)->barrier(this, dst_access);
		return *this;
	}

	VulkanContext& VulkanContext::update(RHIBuffer* dst, const void* src, const RHIBufferCopy& region)
	{
		const u8* data = static_cast<const u8*>(src) + region.src_offset;
		static_cast<VulkanBuffer*>(dst)->update(this, region.dst_offset, region.size, data);
		return *this;
	}

	VulkanContext& VulkanContext::copy(RHIBuffer* dst, RHIBuffer* src, const RHIBufferCopy& region)
	{
		vk::Buffer src_buffer = static_cast<VulkanBuffer*>(src)->buffer();
		vk::Buffer dst_buffer = static_cast<VulkanBuffer*>(dst)->buffer();
		m_cmd->copyBuffer(src_buffer, dst_buffer, vk::BufferCopy(region.src_offset, region.dst_offset, region.size));
		return *this;
	}
}// namespace Trinex
