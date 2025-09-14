#include <Core/etl/data_chain.hpp>
#include <Core/etl/storage.hpp>
#include <Core/exception.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <algorithm>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	static constexpr size_t default_uniform_buffer_size = 1024 * 32;// 32 KB

	class VulkanUniformBuffer : public VulkanBuffer
	{
		VulkanFenceRef m_fence;
		byte* m_memory;
		byte* m_block_start;
		byte* m_block_end;

	public:
		VulkanUniformBuffer* next = nullptr;

		VulkanUniformBuffer()
		{
			constexpr auto flags =
			        RHIBufferCreateFlags::UniformBuffer | RHIBufferCreateFlags::CPURead | RHIBufferCreateFlags::CPUWrite;
			create(default_uniform_buffer_size, nullptr, flags);
			m_memory = m_block_start = m_block_end = map();
		}

		inline VulkanUniformBuffer& reset()
		{
			m_block_start = m_block_end = m_memory;
			return *this;
		}

		inline VulkanUniformBuffer& flush()
		{
			m_block_start = m_block_end = align_up(m_block_end, API->m_properties.limits.minUniformBufferOffsetAlignment);
			return *this;
		}

		inline VulkanUniformBuffer& submit()
		{
			m_fence.signal(API->current_command_buffer());
			return *this;
		}

		inline bool update(const void* data, size_t size, size_t offset)
		{
			if (m_block_start + offset + size > m_memory + default_uniform_buffer_size)
			{
				return false;
			}

			std::memcpy(m_block_start + offset, data, size);
			m_block_end = std::max<byte*>(m_block_end, m_block_start + offset + size);
			return true;
		}

		inline bool is_free() { return m_fence.is_signaled() || !m_fence.is_waiting(); }
		inline size_t block_size() const { return m_block_end - m_block_start; }
		inline size_t block_offset() const { return m_block_start - m_memory; }
		inline byte* mapped_memory() const { return m_memory; }

		~VulkanUniformBuffer() { unmap(); }
	};


	struct VulkanChainNode : public Storage<32, 8> {

		inline bool operator<(const VulkanChainNode& node) const
		{
			const uint64_t* a = reinterpret_cast<const uint64_t*>(data);
			const uint64_t* b = reinterpret_cast<const uint64_t*>(node.data);

			for (size_t i = 0; i < 4; ++i)
			{
				if (a[i] != b[i])
					return a[i] < b[i];
			}

			return false;
		}

		inline void memset(uint64_t value)
		{
			uint64_t* ptr = reinterpret_cast<uint64_t*>(data);
			ptr[0]        = value;
			ptr[1]        = value;
			ptr[2]        = value;
			ptr[3]        = value;
		}
	};

	struct VulkanRasterizationInfo {
		class VulkanRenderPass* pass;
		vk::PrimitiveTopology primitive_topology;
		vk::PolygonMode polygon_mode;
		vk::CullModeFlags cull_mode;
		vk::FrontFace front_face;
		vk::ColorComponentFlags write_mask;
	};

	static DataChain<VulkanChainNode> s_pipeline_state_chain;

	VulkanStateManager::VulkanStateManager()
	{
		m_uniform_buffer_push_ptr = &m_uniform_buffer_pool;
	}

	VulkanStateManager::~VulkanStateManager()
	{
		for (VulkanUniformBuffer* buffer : m_global_uniform_buffers)
		{
			Engine::release(buffer);
		}

		while (m_uniform_buffer_pool)
		{
			VulkanUniformBuffer* buffer = m_uniform_buffer_pool;
			m_uniform_buffer_pool       = m_uniform_buffer_pool->next;
			Engine::release(buffer);
		}
	}

	VulkanUniformBuffer* VulkanStateManager::request_uniform_buffer()
	{
		if (m_uniform_buffer_pool && m_uniform_buffer_pool->is_free())
		{
			VulkanUniformBuffer* buffer = m_uniform_buffer_pool;
			m_uniform_buffer_pool       = m_uniform_buffer_pool->next;

			if (m_uniform_buffer_pool == nullptr)
				m_uniform_buffer_push_ptr = &m_uniform_buffer_pool;

			buffer->next = nullptr;
			return &buffer->reset();
		}

		return allocate<VulkanUniformBuffer>();
	}

	VulkanStateManager& VulkanStateManager::return_uniform_buffer(VulkanUniformBuffer* buffer)
	{
		(*m_uniform_buffer_push_ptr) = buffer;
		m_uniform_buffer_push_ptr    = &buffer->next;
		buffer->submit();
		return *this;
	}

	VulkanStateManager& VulkanStateManager::flush_state()
	{
		uniform_buffers.flush();
		storage_buffers.flush();
		uniform_texel_buffers.flush();
		storage_texel_buffers.flush();
		samplers.flush();
		srv_images.flush();
		uav_images.flush();
		vertex_streams.flush();
		for (auto& va : vertex_attributes) va.flush();

		size_t count = m_global_uniform_buffers.size();

		for (size_t i = 0; i < count; ++i)
		{
			m_global_uniform_buffers[i]->flush();
		}
		return *this;
	}

	VulkanStateManager& VulkanStateManager::update_scalar(const void* data, size_t size, size_t offset, byte buffer_index)
	{
		{
			size_t buffers_count = m_global_uniform_buffers.size();
			if (buffer_index >= buffers_count)
			{
				m_global_uniform_buffers.resize(buffer_index + 1);

				for (size_t i = buffers_count; i <= buffer_index; ++i)
				{
					m_global_uniform_buffers[i] = request_uniform_buffer();
				}
			}
		}

		VulkanUniformBuffer* buffer = m_global_uniform_buffers[buffer_index];

		if (!buffer->update(data, size, offset))
		{
			VulkanUniformBuffer* new_buffer = request_uniform_buffer();
			if (auto size = buffer->block_size())
			{
				new_buffer->update(buffer->mapped_memory() + buffer->block_offset(), size, 0);
			}

			return_uniform_buffer(buffer);
			buffer = new_buffer;
			buffer->update(data, size, offset);
		}

		m_global_uniform_buffers[buffer_index] = buffer;
		uniform_buffers.bind(Buffer(buffer->buffer(), buffer->block_size(), buffer->block_offset()), buffer_index);
		return *this;
	}

	VulkanCommandBuffer* VulkanStateManager::begin_render_pass()
	{
		auto cmd = API->current_command_buffer();
		m_render_target->lock_surfaces();
		m_render_pass = m_render_target->m_render_pass;
		cmd->begin_render_pass(m_render_target);
		return cmd;
	}

	VulkanCommandBuffer* VulkanStateManager::end_render_pass()
	{
		auto cmd = API->current_command_buffer();

		if (m_render_pass)
		{
			cmd->end_render_pass();
			m_render_pass = nullptr;
		}
		return cmd;
	}

	VulkanCommandBuffer* VulkanStateManager::flush_graphics()
	{
		trinex_profile_cpu_n("VulkanStateManager::flush_graphics");
		auto cmd = API->current_command_buffer();

		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		trinex_check(m_render_target, "Render target can't be nullptr");

		m_pipeline->flush(this);

		if (is_dirty(RenderTarget))
		{
			if (cmd->is_inside_render_pass())
				end_render_pass();
		}

		if (cmd->is_outside_render_pass())
			begin_render_pass();

		flush_state();
		return cmd;
	}

	VulkanCommandBuffer* VulkanStateManager::flush_compute()
	{
		auto cmd = API->current_command_buffer();
		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(this);
		flush_state();
		return cmd;
	}

	VulkanStateManager& VulkanStateManager::submit()
	{
		m_dirty_flags = GraphicsMask | ComputeMask;
		uniform_buffers.make_dirty();
		storage_buffers.make_dirty();
		uniform_texel_buffers.make_dirty();
		storage_texel_buffers.make_dirty();
		samplers.make_dirty();
		srv_images.make_dirty();
		uav_images.make_dirty();
		vertex_streams.make_dirty();
		for (auto& va : vertex_attributes) va.make_dirty();

		return *this;
	}

	vk::PipelineVertexInputStateCreateInfo VulkanStateManager::create_vertex_input(VulkanVertexAttribute* attributes,
	                                                                               size_t count)
	{
		using VADesc = vk::VertexInputAttributeDescription;
		using VBDesc = vk::VertexInputBindingDescription;

		if (count == 0)
			return vk::PipelineVertexInputStateCreateInfo();

		vk::PipelineVertexInputStateCreateInfo info;

		info.vertexAttributeDescriptionCount = count;
		info.vertexBindingDescriptionCount   = 0;

		auto va_desc = StackAllocator<VADesc>::allocate(count);

		// Initialize vertex attributes
		{
			info.pVertexAttributeDescriptions = va_desc;

			for (size_t i = 0; i < count; ++i)
			{
				auto& src = attributes[i];

				auto va_state = vertex_attributes[src.semantic].resource(src.semantic_index);

				va_desc[i].format   = src.format;
				va_desc[i].offset   = va_state.offset;
				va_desc[i].location = src.binding;
				va_desc[i].binding  = va_state.stream;
			}

			std::sort(va_desc, va_desc + count, [](const VADesc& a, const VADesc& b) -> bool { return a.binding < b.binding; });

			info.vertexBindingDescriptionCount = 1;

			for (size_t i = 1; i < count; ++i)
			{
				if (va_desc[i].binding != va_desc[i - 1].binding)
					++info.vertexBindingDescriptionCount;
			}
		}

		// Initialize vertex streams
		{
			auto vb_desc                    = StackAllocator<VBDesc>::allocate(info.vertexBindingDescriptionCount);
			info.pVertexBindingDescriptions = vb_desc;

			size_t va_index = 0;

			while (va_index < count)
			{
				uint32_t stream = va_desc[va_index++].binding;
				auto vs_state   = vertex_streams.resource(stream);

				vb_desc->binding   = stream;
				vb_desc->stride    = vs_state.stride;
				vb_desc->inputRate = vs_state.rate;
				++vb_desc;

				while (va_index < count && va_desc[va_index].binding == stream) ++va_index;
			}
		}

		return info;
	}

	Identifier VulkanStateManager::graphics_pipeline_id(VulkanVertexAttribute* attributes, size_t count) const
	{
		struct VACache {
			vk::Format format;
			uint16_t offset;
			byte binding;
			byte stream;
		};

		struct VSCache {
			uint16_t stride;
			byte binding;
			byte rate;
		};

		static_assert(sizeof(VACache) == 8);
		static_assert(sizeof(VulkanRasterizationInfo) == sizeof(VulkanChainNode));

		VulkanChainNode node;
		VulkanRasterizationInfo& info = node.as<VulkanRasterizationInfo, 0>();

		info.pass               = render_target()->render_pass();
		info.primitive_topology = primitive_topology();
		info.polygon_mode       = polygon_mode();
		info.cull_mode          = cull_mode();
		info.front_face         = front_face();
		info.write_mask         = write_mask();

		auto link = s_pipeline_state_chain.link(node);

		VulkanVertexAttribute* const end_va = attributes + count;
		VACache* const end_va_cache         = &node.as<VACache, 0>() + 4;
		VSCache* const end_vs_cache         = &node.as<VSCache, 0>() + 8;

		// Submit vertex attributes
		{
			VulkanVertexAttribute* va = attributes;

			while (va != end_va)
			{
				node.memset(0);
				VACache* cache = &node.as<VACache, 0>();

				while (cache != end_va_cache && va != end_va)
				{
					auto state = vertex_attributes[va->semantic].resource(va->semantic_index);

					cache->stream  = state.stream;
					cache->offset  = state.offset;
					cache->binding = va->binding;
					cache->format  = va->format;

					++cache;
					++va;
				}

				link = link.next(node);
			}
		}

		// Submit vertex streams
		{
			VulkanVertexAttribute* va = attributes;

			while (va != end_va)
			{
				node.memset(0);
				VSCache* cache = &node.as<VSCache, 0>();

				while (cache != end_vs_cache && va != end_va)
				{
					auto va_state = vertex_attributes[va->semantic].resource(va->semantic_index);
					auto vs_state = vertex_streams.resource(va_state.stream);

					cache->binding = va_state.stream;
					cache->stride  = vs_state.stride;
					cache->rate    = static_cast<byte>(vs_state.rate);

					++cache;
					++va;
				}

				link = link.next(node);
			}
		}

		return link.id();
	}

	Identifier VulkanStateManager::mesh_pipeline_id() const
	{
		VulkanChainNode node;

		static_assert(sizeof(VulkanRasterizationInfo) == sizeof(VulkanChainNode));
		VulkanRasterizationInfo& info = node.as<VulkanRasterizationInfo, 0>();

		info.pass               = render_target()->render_pass();
		info.primitive_topology = primitive_topology();
		info.polygon_mode       = polygon_mode();
		info.cull_mode          = cull_mode();
		info.front_face         = front_face();
		info.write_mask         = write_mask();

		return s_pipeline_state_chain.link(node).id();
	}

	VulkanAPI& VulkanAPI::primitive_topology(RHIPrimitiveTopology topology)
	{
		m_state_manager->bind(VulkanEnums::primitive_topology_of(topology));
		return *this;
	}

	VulkanAPI& VulkanAPI::polygon_mode(RHIPolygonMode mode)
	{
		m_state_manager->bind(VulkanEnums::polygon_mode_of(mode));
		return *this;
	}

	VulkanAPI& VulkanAPI::cull_mode(RHICullMode mode)
	{
		m_state_manager->bind(VulkanEnums::cull_mode_of(mode));
		return *this;
	}

	VulkanAPI& VulkanAPI::front_face(RHIFrontFace face)
	{
		m_state_manager->bind(VulkanEnums::face_of(face));
		return *this;
	}

	VulkanAPI& VulkanAPI::write_mask(RHIColorComponent mask)
	{
		vk::ColorComponentFlags color_mask;

		if (mask & RHIColorComponent::R)
		{
			color_mask |= vk::ColorComponentFlagBits::eR;
		}

		if (mask & RHIColorComponent::G)
		{
			color_mask |= vk::ColorComponentFlagBits::eG;
		}

		if (mask & RHIColorComponent::B)
		{
			color_mask |= vk::ColorComponentFlagBits::eB;
		}

		if (mask & RHIColorComponent::A)
		{
			color_mask |= vk::ColorComponentFlagBits::eA;
		}

		m_state_manager->bind(color_mask);
		return *this;
	}

	VulkanAPI& VulkanAPI::update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		m_state_manager->update_scalar(data, size, offset, buffer_index);
		return *this;
	}

	VulkanAPI& VulkanAPI::bind_vertex_attribute(RHIVertexSemantic semantic, byte semantic_index, byte stream, uint16_t offset)
	{
		VulkanStateManager::VertexAttribute va;
		va.stream = stream;
		va.offset = offset;

		m_state_manager->vertex_attributes[semantic].bind(va, semantic_index);
		return *this;
	}
}// namespace Engine
