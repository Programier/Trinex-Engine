#include <Core/etl/algorithm.hpp>
#include <Core/etl/storage.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <RHI/gc.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_query.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_sync.hpp>

namespace Trinex
{
	vk::PipelineRenderingCreateInfo VulkanContext::Framebuffer::pipeline_create_info() const
	{
		vk::PipelineRenderingCreateInfo info;
		info.setColorAttachmentCount(4);
		info.setPColorAttachmentFormats(formats);
		info.setDepthAttachmentFormat(formats[4]);
		info.setStencilAttachmentFormat(formats[5]);
		info.setViewMask(0);

		return info;
	}

	VulkanContext& VulkanContext::flush_state(u32 mask)
	{
		remove_dirty(mask);

		vertex_streams.flush();
		vertex_attributes.flush();
		return *this;
	}

	VulkanContext& VulkanContext::bind_framebuffer(const Framebuffer& fb)
	{
		if (m_framebuffer.size != fb.size)
		{
			m_dirty_flags |= RenderTarget;
			m_dirty_flags |= Viewport;
			m_dirty_flags |= Scissor;

			m_framebuffer = fb;
			return *this;
		}

		for (u16 i = 0; i < 6; ++i)
		{
			if (m_framebuffer.formats[i] != fb.formats[i])
			{
				m_dirty_flags |= RenderTarget;
				m_framebuffer = fb;
				return *this;
			}
		}
		return *this;
	}

	VulkanCommandHandle* VulkanContext::flush_graphics()
	{
		trinex_profile_cpu_n("VulkanContext::flush_graphics");
		trinex_assert_msg(m_pipeline, "Pipeline can't be nullptr");

		m_pipeline->flush(this);

		if (is_dirty(Viewport))
		{
			vk::Viewport vulkan_viewport;
			vulkan_viewport.setWidth(m_viewport.size.x * static_cast<float>(m_framebuffer.size.x));
			vulkan_viewport.setHeight(m_viewport.size.y * static_cast<float>(m_framebuffer.size.y));
			vulkan_viewport.setX(m_viewport.pos.x * static_cast<float>(m_framebuffer.size.x));
			vulkan_viewport.setY(m_viewport.pos.y * static_cast<float>(m_framebuffer.size.y));
			vulkan_viewport.setMinDepth(0.f);
			vulkan_viewport.setMaxDepth(1.f);
			m_cmd->setViewport(0, vulkan_viewport);

			remove_dirty(Viewport);
		}

		if (is_dirty(Scissor))
		{
			vk::Rect2D vulkan_scissor;
			vulkan_scissor.offset.setX(m_scissor.pos.x * static_cast<float>(m_framebuffer.size.x));
			vulkan_scissor.offset.setY(m_scissor.pos.y * static_cast<float>(m_framebuffer.size.y));
			vulkan_scissor.extent.setWidth(m_scissor.size.x * static_cast<float>(m_framebuffer.size.x));
			vulkan_scissor.extent.setHeight(m_scissor.size.y * static_cast<float>(m_framebuffer.size.y));
			m_cmd->setScissor(0, vulkan_scissor);

			remove_dirty(Scissor);
		}

		if (is_dirty(DepthStencilState))
		{
			auto& stencil = m_pipeline_state->depth_stencil.stencil;
			m_cmd->setStencilCompareMask(vk::StencilFaceFlagBits::eFrontAndBack, stencil.compare_mask);
			m_cmd->setStencilWriteMask(vk::StencilFaceFlagBits::eFrontAndBack, stencil.write_mask);
			m_cmd->setStencilReference(vk::StencilFaceFlagBits::eFrontAndBack, stencil.reference);
		}

		if (is_dirty(DepthBias))
		{
			m_cmd->setDepthBias(m_depth_bias.constant, m_depth_bias.clamp, m_depth_bias.slope);
			remove_dirty(DepthBias);
		}

		flush_state(GraphicsMask);
		return m_cmd;
	}

	VulkanCommandHandle* VulkanContext::flush_compute()
	{
		auto cmd = handle();
		trinex_assert_msg(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(this);
		flush_state(ComputeMask);
		return cmd;
	}

	VulkanCommandHandle* VulkanContext::flush_raytrace()
	{
		auto cmd = handle();
		trinex_assert_msg(m_pipeline, "Pipeline can't be nullptr");
		m_pipeline->flush(this);
		flush_state(ComputeMask);
		return cmd;
	}

	VulkanContext& VulkanContext::reset()
	{
		m_dirty_flags = GraphicsMask | ComputeMask | GeneralMask;

		vertex_streams.make_dirty();
		vertex_attributes.make_dirty();
		m_framebuffer = Framebuffer();
		m_topology    = RHITopology::TriangleList;

		m_pipeline_state->depth_stencil = {};
		m_pipeline_state->blending      = {};
		m_pipeline_state->rasterizer    = {};

		m_depth_bias.constant = 0.f;
		m_depth_bias.clamp    = 0.f;
		m_depth_bias.slope    = 0.f;
		return *this;
	}

	VulkanContext& VulkanContext::copy_state(VulkanContext* src, usize dirty_mask)
	{
		m_dirty_flags    = src->m_dirty_flags & dirty_mask;
		m_framebuffer    = src->m_framebuffer;
		m_pipeline       = src->m_pipeline;
		m_pipeline_state = src->m_pipeline_state;

		vertex_streams    = src->vertex_streams;
		vertex_attributes = src->vertex_attributes;
		return *this;
	}

	vk::PipelineVertexInputStateCreateInfo VulkanContext::create_vertex_input(VulkanVertexAttribute* attributes, usize count)
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

			for (usize i = 0; i < count; ++i)
			{
				auto& src = attributes[i];

				auto va_state = vertex_attributes.resource(src.semantic);

				va_desc[i].format   = VulkanEnums::vertex_format_of(va_state.format);
				va_desc[i].offset   = va_state.offset;
				va_desc[i].location = src.binding;
				va_desc[i].binding  = va_state.stream;
			}

			etl::sort(va_desc, va_desc + count, [](const VADesc& a, const VADesc& b) -> bool { return a.binding < b.binding; });

			info.vertexBindingDescriptionCount = 1;

			for (usize i = 1; i < count; ++i)
			{
				if (va_desc[i].binding != va_desc[i - 1].binding)
					++info.vertexBindingDescriptionCount;
			}
		}

		// Initialize vertex streams
		{
			auto vb_desc                    = StackAllocator<VBDesc>::allocate(info.vertexBindingDescriptionCount);
			info.pVertexBindingDescriptions = vb_desc;

			usize va_index = 0;

			while (va_index < count)
			{
				u32 stream    = va_desc[va_index++].binding;
				auto vs_state = vertex_streams.resource(stream);

				vb_desc->binding   = stream;
				vb_desc->stride    = vs_state.stride;
				vb_desc->inputRate = vs_state.rate;
				++vb_desc;

				while (va_index < count && va_desc[va_index].binding == stream)
					++va_index;
			}
		}

		return info;
	}

	u128 VulkanContext::graphics_pipeline_id(VulkanVertexAttribute* attributes, usize count) const
	{
		struct VACache
		{
			u16 offset;
			u16 stride;
			u16 binding;
			u8 format;
			u8 stream;
			u8 rate;
			u8 padding = 0;
		};

		u128 hash = pipeline_state_id();

		// Hash render target formats
		{
			hash = memory_hash(m_framebuffer.formats, sizeof(m_framebuffer.formats), hash);
		}

		// Submit vertex attributes
		{
			VACache va;

			for (usize i = 0; i < count; ++i)
			{
				auto& src = attributes[i];

				auto va_state = vertex_attributes.resource(src.semantic);
				auto vs_state = vertex_streams.resource(va_state.stream);

				va.format  = va_state.format;
				va.offset  = va_state.offset;
				va.stride  = vs_state.stride;
				va.binding = src.binding;
				va.stream  = va_state.stream;
				va.rate    = static_cast<u8>(vs_state.rate);

				hash = memory_hash(&va, sizeof(va), hash);
			}
		}

		return hash;
	}

	VulkanContext& VulkanContext::depth_stencil_state(const RHIDepthStencilState& state)
	{
		if (m_pipeline_state->depth_stencil != state)
		{
			m_pipeline_state->depth_stencil = state;
			m_dirty_flags |= DepthStencilState;
		}
		return *this;
	}

	VulkanContext& VulkanContext::blending_state(const RHIBlendingState& state)
	{
		if (m_pipeline_state->blending != state)
		{
			m_pipeline_state->blending = state;
			m_dirty_flags |= BlendingState;
		}
		return *this;
	}

	VulkanContext& VulkanContext::rasterizer_state(const RHIRasterizerState& state)
	{
		if (m_pipeline_state->rasterizer != state)
		{
			m_pipeline_state->rasterizer = state;
			m_dirty_flags |= RasterizerState;
		}
		return *this;
	}

	VulkanContext& VulkanContext::bind_vertex_attribute(RHISemantic semantic, RHIVertexFormat format, u8 stream, u16 offset)
	{
		VertexAttribute va;
		va.stream = stream;
		va.offset = offset;
		va.format = format;

		vertex_attributes.bind(va, semantic);
		return *this;
	}

	VulkanCommandHandle::VulkanCommandHandle(VulkanQueue* queue)
	{
		constexpr auto flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient;
		m_pool = vk::verify(VulkanAPI::device()->createCommandPool(vk::CommandPoolCreateInfo(flags, queue->family_index())));

		vk::CommandBufferAllocateInfo alloc_info(m_pool, vk::CommandBufferLevel::ePrimary, 1);
		static_cast<vk::CommandBuffer&>(*this) = vk::verify(VulkanAPI::device()->allocateCommandBuffers(alloc_info)).front();
	}

	VulkanCommandHandle::~VulkanCommandHandle()
	{
		VulkanAPI::device()->freeCommandBuffers(m_pool, *this);
		VulkanAPI::device()->destroyCommandPool(m_pool);
	}

	void VulkanCommandHandle::on_reclaimable()
	{
		m_objects.clear();
	}

	RHI* VulkanCommandHandle::device() const
	{
		return VulkanAPI::instance();
	}

	VulkanContext::VulkanContext(VulkanQueue* queue) : m_queue(queue)
	{
		trinex_assert(m_queue);
		reset();
	}

	VulkanContext::~VulkanContext()
	{
		if (auto handle = end())
		{
			handle->release();
		}
	}

	RHI* VulkanContext::device() const
	{
		return VulkanAPI::instance();
	}

	VulkanContext& VulkanContext::begin(VulkanCommandHandle* handle)
	{
		m_cmd = handle;
		return reset();
	}

	VulkanContext& VulkanContext::begin()
	{
		if (m_cmd == nullptr)
		{
			RHIGarbageGroup* group = m_queue->command_group();

			if (auto gc = VulkanAPI::instance()->gc())
			{
				if (auto instance = gc->take(group))
				{
					return begin(instance->as<VulkanCommandHandle>());
				}
			}

			return begin(trx_new VulkanCommandHandle(m_queue));
		}

		return *this;
	}

	VulkanCommandHandle* VulkanContext::end()
	{
		if (m_cmd == nullptr)
			return nullptr;

		VulkanCommandHandle* cmd = m_cmd;
		vk::verify(cmd->end());

		m_cmd = nullptr;
		return cmd;
	}

	VulkanContext& VulkanContext::retain(RefCounted* object)
	{
		if (object && m_cmd)
		{
			m_cmd->retain(object);
		}

		return *this;
	}

	VulkanContext& VulkanContext::adopt(RefCounted* object)
	{
		if (object && m_cmd)
		{
			m_cmd->adopt(object);
		}

		return *this;
	}

	VulkanContext& VulkanContext::begin_rendering(const RHIRenderingInfo& info)
	{
		Framebuffer fb;

		auto push_extent = [&fb](const vk::Extent3D& extent) {
			Vector2u16 current = {extent.width, extent.height};
			fb.size            = (fb.size.x || fb.size.y) ? Math::min(fb.size, current) : current;
		};

		vk::RenderingAttachmentInfo attachments[6];
		vk::RenderingAttachmentInfo* depth   = nullptr;
		vk::RenderingAttachmentInfo* stencil = nullptr;

		for (u32 i = 0; i < 4; ++i)
		{
			const RHIColorAttachmentInfo& src = info.colors[i];

			if (src.view == nullptr)
				continue;

			vk::RenderingAttachmentInfo& dst = attachments[i];

			VulkanTextureRTV* rtv = static_cast<VulkanTextureRTV*>(src.view);

			push_extent(rtv->extent());
			fb.formats[i] = rtv->format();

			dst.setClearValue(vk::ClearColorValue(src.ucolor.x, src.ucolor.y, src.ucolor.z, src.ucolor.w));
			dst.setImageLayout(vk::ImageLayout::eColorAttachmentOptimal);
			dst.setImageView(rtv->view());
			dst.setStoreOp(VulkanEnums::store_of(src.store));
			dst.setLoadOp(VulkanEnums::load_of(src.load));
		}

		if (info.depth_stencil.view)
		{
			const RHIDepthStencilAttachmentInfo& src = info.depth_stencil;

			VulkanTextureDSV* dsv = static_cast<VulkanTextureDSV*>(info.depth_stencil.view);
			vk::Format format     = dsv->format();

			push_extent(dsv->extent());

			if (VulkanEnums::is_depth_format(format))
			{
				depth         = &attachments[4];
				fb.formats[4] = format;

				depth->setClearValue(vk::ClearDepthStencilValue(src.depth, src.stencil));
				depth->setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				depth->setImageView(dsv->view());
				depth->setStoreOp(VulkanEnums::store_of(src.depth_store));
				depth->setLoadOp(VulkanEnums::load_of(src.depth_load));
			}

			if (VulkanEnums::is_stencil_format(format))
			{
				stencil       = &attachments[5];
				fb.formats[5] = format;

				stencil->setClearValue(vk::ClearDepthStencilValue(src.depth, src.stencil));
				stencil->setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
				stencil->setImageView(dsv->view());
				stencil->setStoreOp(VulkanEnums::store_of(src.stencil_store));
				stencil->setLoadOp(VulkanEnums::load_of(src.stencil_load));
			}
		}

		vk::RenderingInfo rendering;
		rendering.setLayerCount(1);

		if (info.rect.size.x && info.rect.size.y)
		{
			vk::Rect2D rect = {vk::Offset2D(info.rect.pos.x, info.rect.pos.y), vk::Extent2D(info.rect.size.x, info.rect.size.y)};
			rendering.setRenderArea(rect);
		}
		else
		{
			rendering.setRenderArea(vk::Rect2D({0, 0}, {fb.size.x, fb.size.y}));
		}

		rendering.setColorAttachmentCount(4);
		rendering.setPColorAttachments(attachments);
		rendering.setPDepthAttachment(depth);
		rendering.setPStencilAttachment(stencil);
		rendering.setFlags(VulkanEnums::rendering_flags_of(info.flags));

		m_cmd->beginRenderingKHR(rendering);
		return bind_framebuffer(fb);
	}

	VulkanContext& VulkanContext::end_rendering()
	{
		m_cmd->endRenderingKHR();
		return *this;
	}

	VulkanContext& VulkanContext::viewport(const RHIRegion& viewport)
	{
		trinex_profile_cpu_n("VulkanContext::viewport");

		if (m_viewport != viewport)
		{
			m_viewport = viewport;
			m_dirty_flags |= Viewport;
		}

		return *this;
	}

	VulkanContext& VulkanContext::scissor(const RHIRegion& scissor)
	{
		trinex_profile_cpu_n("VulkanContext::scissor");

		if (m_scissor != scissor)
		{
			m_scissor = scissor;
			m_dirty_flags |= Scissor;
		}

		return *this;
	}

	VulkanContext& VulkanContext::draw(RHITopology topology, usize vertex_count, usize vertices_offset, usize instances,
	                                   usize first_instance)
	{
		trinex_profile_cpu_n("VulkanContext::draw");
		flush_graphics(topology)->draw(vertex_count, instances, vertices_offset, first_instance);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed(RHITopology topology, usize indices_count, usize indices_offset,
	                                           usize vertices_offset, usize instances, usize first_instance)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indexed");
		flush_graphics(topology)->drawIndexed(indices_count, instances, indices_offset, vertices_offset, first_instance);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indirect(RHITopology topology, const RHIBufferAddress& args, u32 count, u32 stride)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indirect");
		vk::Buffer buffer = static_cast<VulkanBuffer*>(args.buffer)->buffer();
		flush_graphics(topology)->drawIndirect(buffer, args.offset, count, stride);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indirect(RHITopology topology, const RHIBufferAddress& args, const RHIBufferAddress& count,
	                                            u32 max_count, u32 stride)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indirect");
		vk::Buffer buffer       = static_cast<VulkanBuffer*>(args.buffer)->buffer();
		vk::Buffer count_buffer = static_cast<VulkanBuffer*>(count.buffer)->buffer();
		flush_graphics(topology)->drawIndirectCountKHR(buffer, args.offset, count_buffer, count.offset, max_count, stride);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed_indirect(RHITopology topology, const RHIBufferAddress& args, uint32_t count,
	                                                    uint32_t stride)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indexed_indirect");
		vk::Buffer buffer = static_cast<VulkanBuffer*>(args.buffer)->buffer();
		flush_graphics(topology)->drawIndexedIndirect(buffer, args.offset, count, stride);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed_indirect(RHITopology topology, const RHIBufferAddress& args,
	                                                    const RHIBufferAddress& count, u32 max_count, uint32_t stride)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indexed_indirect");
		vk::Buffer buffer       = static_cast<VulkanBuffer*>(args.buffer)->buffer();
		vk::Buffer count_buffer = static_cast<VulkanBuffer*>(count.buffer)->buffer();
		flush_graphics(topology)->drawIndexedIndirectCountKHR(buffer, args.offset, count_buffer, count.offset, max_count, stride);
		return *this;
	}

	VulkanContext& VulkanContext::draw_mesh(u32 x, u32 y, u32 z)
	{
		trinex_profile_cpu_n("VulkanContext::draw_mesh");
		flush_graphics()->drawMeshTasksEXT(x, y, z);
		return *this;
	}

	VulkanContext& VulkanContext::dispatch(Vector3u groups, Vector3u base)
	{
		trinex_profile_cpu_n("VulkanContext::dispatch");
		flush_compute()->dispatchBase(base.x, base.y, base.z, groups.x, groups.y, groups.z);
		return *this;
	}

	VulkanContext& VulkanContext::dispatch_indirect(const RHIBufferAddress& args)
	{
		trinex_profile_cpu_n("VulkanContext::dispatch_indirect");
		vk::Buffer buffer = static_cast<VulkanBuffer*>(args.buffer)->buffer();
		flush_compute()->dispatchIndirect(buffer, args.offset);
		return *this;
	}

	VulkanContext& VulkanContext::push_debug_stage(const char* stage)
	{
		if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdBeginDebugUtilsLabelEXT)
		{
			vk::DebugUtilsLabelEXT label_info = {};
			label_info.pLabelName             = stage;
			label_info.color[0]               = 1.f;
			label_info.color[1]               = 1.f;
			label_info.color[2]               = 1.f;
			label_info.color[3]               = 1.f;

			m_cmd->beginDebugUtilsLabelEXT(&label_info);
		}
		return *this;
	}

	VulkanContext& VulkanContext::pop_debug_stage()
	{
		if (VULKAN_HPP_DEFAULT_DISPATCHER.vkCmdEndDebugUtilsLabelEXT)
		{
			m_cmd->endDebugUtilsLabelEXT();
		}
		return *this;
	}

	VulkanContext& VulkanContext::depth_bias(float constant, float clamp, float slope)
	{
		if (m_depth_bias.constant != constant)
		{
			m_depth_bias.constant = constant;
			m_dirty_flags |= DepthBias;
		}

		if (m_depth_bias.clamp != clamp)
		{
			m_depth_bias.constant = constant;
			m_dirty_flags |= DepthBias;
		}

		if (m_depth_bias.slope != slope)
		{
			m_depth_bias.slope = slope;
			m_dirty_flags |= DepthBias;
		}

		return *this;
	}

	void VulkanContext::destroy() noexcept
	{
		trx_delete this;
	}

	RHIContext* VulkanAPI::create_context(RHIQueue* queue)
	{
		return trx_new VulkanContext(static_cast<VulkanQueue*>(queue));
	}
} // namespace Trinex
