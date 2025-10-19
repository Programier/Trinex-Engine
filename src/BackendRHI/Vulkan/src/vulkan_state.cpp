#include <Core/etl/storage.hpp>
#include <Core/exception.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <algorithm>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
	VulkanStateManager::VulkanStateManager()
	{
		m_graphics_state.init();
	}

	VulkanStateManager::~VulkanStateManager() {}

	void VulkanStateManager::GraphicsState::init()
	{
		memset(this, 0, sizeof(*this));
		topology     = RHIPrimitiveTopology::TriangleList;
		polygon_mode = RHIPolygonMode::Fill;
		cull_mode    = RHICullMode::None;
		front_face   = RHIFrontFace::CounterClockWise;
		write_mask   = RHIColorComponent::RGBA;
	}

	VulkanStateManager& VulkanStateManager::flush_state(VulkanCommandHandle* handle)
	{
		m_dirty_flags = 0;
		uniform_buffers.flush();
		storage_buffers.flush();
		uniform_texel_buffers.flush();
		storage_texel_buffers.flush();
		samplers.flush();
		srv_images.flush();
		uav_images.flush();
		vertex_streams.flush();
		for (auto& va : vertex_attributes) va.flush();
		handle->flush_uniforms();
		return *this;
	}

	VulkanStateManager& VulkanStateManager::update_scalar(VulkanContext* ctx, const void* data, size_t size, size_t offset,
	                                                      byte buffer_index)
	{
		VulkanUniformBuffer* buffer = ctx->handle()->request_uniform_page(size + offset, buffer_index);
		buffer->update(data, size, offset);
		uniform_buffers.bind(UniformBuffer(buffer->buffer(), buffer->block_size(), buffer->block_offset()), buffer_index);
		return *this;
	}

	VulkanCommandHandle* VulkanStateManager::begin_render_pass(VulkanContext* ctx)
	{
		auto cmd      = ctx->handle();
		m_render_pass = m_render_target->m_render_pass;
		cmd->begin_render_pass(m_render_target);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::end_render_pass(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();

		if (ctx->handle()->is_inside_render_pass())
		{
			cmd->end_render_pass();
			m_render_pass = nullptr;
		}
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::flush_graphics(VulkanContext* ctx)
	{
		trinex_profile_cpu_n("VulkanStateManager::flush_graphics");
		auto cmd = ctx->handle();

		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		trinex_check(m_render_target, "Render target can't be nullptr");

		m_pipeline->flush(ctx);

		if (is_dirty(RenderTarget))
		{
			if (cmd->is_inside_render_pass())
				end_render_pass(ctx);
		}

		if (cmd->is_outside_render_pass())
			begin_render_pass(ctx);

		flush_state(cmd);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::flush_compute(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();
		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(ctx);
		flush_state(cmd);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::flush_raytrace(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();
		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(ctx);
		flush_state(cmd);
		return cmd;
	}

	VulkanStateManager& VulkanStateManager::reset()
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
		m_render_pass = nullptr;

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
			uint32_t format;
			uint16_t offset;
			uint16_t stride;
			uint16_t binding;
			byte stream;
			byte rate;
		};

		uint128_t hash = memory_hash(&m_graphics_state, sizeof(m_graphics_state), 0);

		// Submit vertex attributes
		{
			VACache va;

			for (size_t i = 0; i < count; ++i)
			{
				auto& src = attributes[i];

				auto va_state = vertex_attributes[src.semantic].resource(src.semantic_index);
				auto vs_state = vertex_streams.resource(va_state.stream);

				va.format  = static_cast<uint32_t>(src.format);
				va.offset  = va_state.offset;
				va.stride  = vs_state.stride;
				va.binding = src.binding;
				va.stream  = va_state.stream;
				va.rate    = static_cast<byte>(vs_state.rate);

				hash = memory_hash(&va, sizeof(va), hash);
			}
		}

		return hash;
	}

	Identifier VulkanStateManager::mesh_pipeline_id() const
	{
		return memory_hash(&m_graphics_state, sizeof(m_graphics_state));
	}

	VulkanContext& VulkanContext::primitive_topology(RHIPrimitiveTopology topology)
	{
		m_state_manager->bind(topology);
		return *this;
	}

	VulkanContext& VulkanContext::polygon_mode(RHIPolygonMode mode)
	{
		m_state_manager->bind(mode);
		return *this;
	}

	VulkanContext& VulkanContext::cull_mode(RHICullMode mode)
	{
		m_state_manager->bind(mode);
		return *this;
	}

	VulkanContext& VulkanContext::front_face(RHIFrontFace face)
	{
		m_state_manager->bind(face);
		return *this;
	}

	VulkanContext& VulkanContext::write_mask(RHIColorComponent mask)
	{
		m_state_manager->bind(mask);
		return *this;
	}

	VulkanContext& VulkanContext::update_scalar(const void* data, size_t size, size_t offset, BindingIndex buffer_index)
	{
		m_state_manager->update_scalar(this, data, size, offset, buffer_index);
		return *this;
	}

	VulkanContext& VulkanContext::bind_vertex_attribute(RHIVertexSemantic semantic, byte semantic_index, byte stream,
	                                                    uint16_t offset)
	{
		VulkanStateManager::VertexAttribute va;
		va.stream = stream;
		va.offset = offset;

		m_state_manager->vertex_attributes[semantic].bind(va, semantic_index);
		return *this;
	}
}// namespace Engine
