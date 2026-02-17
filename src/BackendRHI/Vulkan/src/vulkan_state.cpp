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
		reset();
	}

	VulkanStateManager::~VulkanStateManager() {}

	void VulkanStateManager::GraphicsState::init()
	{
		(*this) = {};

		topology     = RHIPrimitiveTopology::TriangleList;
		polygon_mode = RHIPolygonMode::Fill;
		cull_mode    = RHICullMode::None;
		front_face   = RHIFrontFace::CounterClockWise;
		write_mask   = RHIColorComponent::RGBA;
		rate         = RHIShadingRate::e1x1;
		viewport     = RHIViewport();
		scissor      = RHIScissor();
	}

	VulkanStateManager& VulkanStateManager::flush_state(VulkanCommandHandle* handle, uint32_t mask)
	{
		remove_dirty(mask);

		uniform_buffers.flush();
		storage_buffers.flush();
		uniform_texel_buffers.flush();
		storage_texel_buffers.flush();
		samplers.flush();
		srv_images.flush();
		uav_images.flush();
		vertex_streams.flush();
		vertex_attributes.flush();
		handle->flush_uniforms();
		return *this;
	}

	VulkanStateManager& VulkanStateManager::update_viewport_and_scissor(VulkanRenderTarget* target)
	{
		if (m_render_target == nullptr || (m_render_target && (m_render_target->size() != target->size())))
		{
			m_dirty_flags |= Viewport;
			m_dirty_flags |= Scissor;
		}
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
		auto cmd = ctx->handle();

		if (ctx->is_secondary())
			return cmd;

		m_render_pass = m_render_target->m_render_pass;
		cmd->begin_render_pass(m_render_target);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::end_render_pass(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();

		if (ctx->is_secondary())
			return cmd;

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

		if (is_dirty(ShadingRate))
		{
			if (API->is_extension_enabled(VulkanAPI::find_extension_index(VK_KHR_FRAGMENT_SHADING_RATE_EXTENSION_NAME)))
			{
				vk::Extent2D extent;
				extent.width  = 1;
				extent.height = 1;

				vk::FragmentShadingRateCombinerOpKHR ops[2] = {
				        vk::FragmentShadingRateCombinerOpKHR::eKeep,
				        vk::FragmentShadingRateCombinerOpKHR::eKeep,
				};

				cmd->setFragmentShadingRateKHR(extent, ops, API->pfn);
			}
		}

		if (is_dirty(Viewport))
		{
			vk::Viewport vulkan_viewport;
			vulkan_viewport.setWidth(m_graphics_state.viewport.size.x * static_cast<float>(m_render_target->width()));
			vulkan_viewport.setHeight(m_graphics_state.viewport.size.y * static_cast<float>(m_render_target->height()));
			vulkan_viewport.setX(m_graphics_state.viewport.pos.x * static_cast<float>(m_render_target->width()));
			vulkan_viewport.setY(m_graphics_state.viewport.pos.y * static_cast<float>(m_render_target->height()));
			vulkan_viewport.setMinDepth(m_graphics_state.viewport.min_depth);
			vulkan_viewport.setMaxDepth(m_graphics_state.viewport.max_depth);
			cmd->setViewport(0, vulkan_viewport);

			remove_dirty(Viewport);
		}

		if (is_dirty(Scissor))
		{
			vk::Rect2D vulkan_scissor;
			vulkan_scissor.offset.setX(m_graphics_state.scissor.pos.x * static_cast<float>(m_render_target->width()));
			vulkan_scissor.offset.setY(m_graphics_state.scissor.pos.y * static_cast<float>(m_render_target->height()));
			vulkan_scissor.extent.setWidth(m_graphics_state.scissor.size.x * static_cast<float>(m_render_target->width()));
			vulkan_scissor.extent.setHeight(m_graphics_state.scissor.size.y * static_cast<float>(m_render_target->height()));
			cmd->setScissor(0, vulkan_scissor);

			remove_dirty(Scissor);
		}

		if (cmd->is_outside_render_pass())
			begin_render_pass(ctx);

		flush_state(cmd, GraphicsMask);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::flush_compute(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();

		if (cmd->is_inside_render_pass())
			end_render_pass(ctx);

		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(ctx);
		flush_state(cmd, ComputeMask);
		return cmd;
	}

	VulkanCommandHandle* VulkanStateManager::flush_raytrace(VulkanContext* ctx)
	{
		auto cmd = ctx->handle();
		trinex_check(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(ctx);
		flush_state(cmd, ComputeMask);
		return cmd;
	}

	VulkanStateManager& VulkanStateManager::reset()
	{
		m_dirty_flags = GraphicsMask | ComputeMask | GeneralMask;
		uniform_buffers.make_dirty();
		storage_buffers.make_dirty();
		uniform_texel_buffers.make_dirty();
		storage_texel_buffers.make_dirty();
		samplers.make_dirty();
		srv_images.make_dirty();
		uav_images.make_dirty();
		vertex_streams.make_dirty();
		vertex_attributes.make_dirty();
		m_render_pass = nullptr;

		m_graphics_state.init();
		return *this;
	}

	VulkanStateManager& VulkanStateManager::copy(VulkanStateManager* src, size_t dirty_mask)
	{
		m_dirty_flags         = src->m_dirty_flags & dirty_mask;
		uniform_buffers       = src->uniform_buffers;
		storage_buffers       = src->storage_buffers;
		uniform_texel_buffers = src->uniform_texel_buffers;
		storage_texel_buffers = src->storage_texel_buffers;
		samplers              = src->samplers;
		srv_images            = src->srv_images;
		uav_images            = src->uav_images;
		vertex_streams        = src->vertex_streams;
		vertex_attributes     = src->vertex_attributes;
		m_render_pass         = src->m_render_pass;
		m_render_target       = src->m_render_target;
		m_pipeline            = src->m_pipeline;
		m_graphics_state      = src->m_graphics_state;

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

				auto va_state = vertex_attributes.resource(src.semantic);

				va_desc[i].format   = VulkanEnums::vertex_format_of(va_state.format);
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

	uint128_t VulkanStateManager::graphics_pipeline_id(VulkanVertexAttribute* attributes, size_t count) const
	{
		struct VACache {
			uint16_t offset;
			uint16_t stride;
			uint16_t binding;
			byte format;
			byte stream;
			byte rate;
			byte padding = 0;
		};

		uint128_t hash = memory_hash(&m_graphics_state, sizeof(m_graphics_state), 0);

		// Submit vertex attributes
		{
			VACache va;

			for (size_t i = 0; i < count; ++i)
			{
				auto& src = attributes[i];

				auto va_state = vertex_attributes.resource(src.semantic);
				auto vs_state = vertex_streams.resource(va_state.stream);

				va.format  = va_state.format;
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

	uint128_t VulkanStateManager::mesh_pipeline_id() const
	{
		return memory_hash(&m_graphics_state, sizeof(m_graphics_state));
	}

	VulkanContext& VulkanContext::depth_state(const RHIDepthState& state)
	{
		m_state_manager->bind(state);
		return *this;
	}

	VulkanContext& VulkanContext::stencil_state(const RHIStencilState& state)
	{
		m_state_manager->bind(state);
		return *this;
	}

	VulkanContext& VulkanContext::blending_state(const RHIBlendingState& state)
	{
		m_state_manager->bind(state);
		return *this;
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

	VulkanContext& VulkanContext::bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, byte stream,
	                                                    uint16_t offset)
	{
		VulkanStateManager::VertexAttribute va;
		va.stream = stream;
		va.offset = offset;
		va.format = format;

		m_state_manager->vertex_attributes.bind(va, semantic);
		return *this;
	}
}// namespace Engine
