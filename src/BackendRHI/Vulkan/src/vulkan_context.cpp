#include <Core/etl/algorithm.hpp>
#include <Core/etl/storage.hpp>
#include <Core/math/math.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <vulkan_api.hpp>
#include <vulkan_buffer.hpp>
#include <vulkan_context.hpp>
#include <vulkan_descriptor.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_fence.hpp>
#include <vulkan_pipeline.hpp>
#include <vulkan_query.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_resource_view.hpp>

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

		uniform_buffers.flush();
		storage_buffers.flush();
		uniform_texel_buffers.flush();
		storage_texel_buffers.flush();
		samplers.flush();
		srv_images.flush();
		uav_images.flush();
		vertex_streams.flush();
		vertex_attributes.flush();
		m_cmd->flush_uniforms();
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
		uniform_buffers.make_dirty();
		storage_buffers.make_dirty();
		uniform_texel_buffers.make_dirty();
		storage_texel_buffers.make_dirty();
		samplers.make_dirty();
		srv_images.make_dirty();
		uav_images.make_dirty();
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

		uniform_buffers       = src->uniform_buffers;
		storage_buffers       = src->storage_buffers;
		uniform_texel_buffers = src->uniform_texel_buffers;
		storage_texel_buffers = src->storage_texel_buffers;
		samplers              = src->samplers;
		srv_images            = src->srv_images;
		uav_images            = src->uav_images;
		vertex_streams        = src->vertex_streams;
		vertex_attributes     = src->vertex_attributes;
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

				while (va_index < count && va_desc[va_index].binding == stream) ++va_index;
			}
		}

		return info;
	}

	u128 VulkanContext::graphics_pipeline_id(VulkanVertexAttribute* attributes, usize count) const
	{
		struct VACache {
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

	VulkanContext& VulkanContext::update_scalar(const void* data, usize size, usize offset, u8 buffer_index)
	{
		VulkanUniformBuffer* buffer = m_cmd->request_uniform_page(size + offset, buffer_index);
		buffer->update(data, size, offset);
		uniform_buffers.bind(UniformBuffer(buffer->buffer(), buffer->block_size(), buffer->block_offset()), buffer_index);
		return *this;
	}

	VulkanContext& VulkanContext::bind_vertex_attribute(RHIVertexSemantic semantic, RHIVertexFormat format, u8 stream, u16 offset)
	{
		VertexAttribute va;
		va.stream = stream;
		va.offset = offset;
		va.format = format;

		vertex_attributes.bind(va, semantic);
		return *this;
	}


	VulkanUniformBuffer* VulkanCommandHandle::UniformBuffer::request_uniform_page(usize size)
	{
		while (*m_uniform_buffer_current)
		{
			VulkanUniformBuffer* buffer = *m_uniform_buffer_current;
			if (buffer->contains(size))
				return buffer;
			m_uniform_buffer_current = &buffer->next;
		}


		VulkanUniformBuffer* buffer = trx_new VulkanUniformBuffer(size);
		(*m_uniform_buffer_current) = buffer;
		return buffer;
	}

	VulkanCommandHandle::UniformBuffer& VulkanCommandHandle::UniformBuffer::reset()
	{
		VulkanUniformBuffer* buffer = m_uniform_buffer_head;
		m_uniform_buffer_current    = &m_uniform_buffer_head;

		while (buffer)
		{
			buffer->reset();
			buffer = buffer->next;
		}

		return *this;
	}

	VulkanCommandHandle::UniformBuffer& VulkanCommandHandle::UniformBuffer::flush()
	{
		if (*m_uniform_buffer_current)
		{
			(*m_uniform_buffer_current)->flush();
		}

		return *this;
	}

	VulkanCommandHandle::VulkanCommandHandle(VulkanCommandBufferManager* manager, RHIContextFlags flags) : m_flags(flags)
	{
		m_fence   = trx_new VulkanFence(false);
		m_manager = manager;

		const bool secondary               = is_secondary();
		const vk::CommandBufferLevel level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;

		vk::CommandPool pool = manager->command_pool();
		vk::CommandBufferAllocateInfo alloc_info(pool, level, 1);

		static_cast<vk::CommandBuffer&>(*this) = vk::check_result(API->m_device.allocateCommandBuffers(alloc_info)).front();
	}

	VulkanCommandHandle& VulkanCommandHandle::refresh_fence_status()
	{
		if (m_state == State::Submitted)
		{
			if (m_fence->is_signaled())
			{
				m_state = State::Unused;
				vk::check_result(reset());
				m_fence->reset();

				for (auto& page : m_uniform_buffers)
				{
					page.reset();
				}
			}
		}

		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::begin(const vk::CommandBufferBeginInfo& info)
	{
		trinex_assert_msg(is_unused(), "Command Buffer must be unused");

		vk::check_result(vk::CommandBuffer::begin(info));
		m_state = State::Active;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::end()
	{
		trinex_assert_msg(is_active(), "Command Buffer must be active!");
		vk::check_result(vk::CommandBuffer::end());
		m_state = State::Pending;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::enqueue()
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::submit");
		trinex_assert_msg(is_pending(), "Command Buffer must be in ended state!");
		API->m_graphics_queue->submit({}, m_fence->fence());
		m_fence->make_pending();
		m_state = State::Submitted;
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::wait()
	{
		trinex_profile_cpu_n("VulkanCommandBuffer::wait");
		if (m_state == State::Submitted)
		{
			if (!m_fence->is_signaled())
				m_fence->wait();

			refresh_fence_status();
		}
		return *this;
	}

	VulkanCommandHandle& VulkanCommandHandle::flush_uniforms()
	{
		for (auto& page : m_uniform_buffers)
		{
			page.flush();
		}
		return *this;
	}

	void VulkanCommandHandle::destroy()
	{
		for (RHIObject* stagging : m_stagging)
		{
			stagging->destroy();
		}

		m_stagging.clear();
		m_manager->return_handle(this);
	}

	VulkanCommandHandle::~VulkanCommandHandle()
	{
		wait();

		auto pool = m_manager->command_pool();
		API->m_device.freeCommandBuffers(pool, *this);

		for (auto& page : m_uniform_buffers)
		{
			while (page.m_uniform_buffer_head)
			{
				auto current               = page.m_uniform_buffer_head;
				page.m_uniform_buffer_head = page.m_uniform_buffer_head->next;
				trx_delete_inline(current);
			}
		}
		trx_delete m_fence;
	}

	VulkanCommandBufferManager::VulkanCommandBufferManager()
	{
		m_pool = vk::check_result(API->m_device.createCommandPool(
		        vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, API->m_graphics_queue->index())));
	}

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		for (VulkanCommandHandle* handle : m_primary)
		{
			trx_delete handle;
		}

		for (VulkanCommandHandle* handle : m_secondary)
		{
			trx_delete handle;
		}

		API->m_device.destroyCommandPool(m_pool);
	}

	VulkanCommandBufferManager* VulkanCommandBufferManager::instance()
	{
		static thread_local VulkanCommandBufferManager* manager = trx_new VulkanCommandBufferManager();
		return manager;
	}

	VulkanCommandHandle* VulkanCommandBufferManager::request_handle(RHIContextFlags flags)
	{
		{
			ScopeLock lock(m_critical);

			Deque<VulkanCommandHandle*>* handles = flags & RHIContextFlags::Secondary ? &m_secondary : &m_primary;

			if (!handles->empty())
			{
				auto front = handles->front();

				if (front->refresh_fence_status().is_unused())
				{
					handles->pop_front();
					return front;
				}
			}
		}

		return trx_new VulkanCommandHandle(this, flags);
	}

	VulkanCommandBufferManager& VulkanCommandBufferManager::return_handle(VulkanCommandHandle* handle)
	{
		ScopeLock lock(m_critical);

		auto& queue = handle->is_primary() ? m_primary : m_secondary;
		queue.push_back(handle);

		handle->add_reference();
		handle->enqueue();
		return *this;
	}

	VulkanContext::VulkanContext(RHIContextFlags flags) : m_flags(flags)
	{
		reset();
	}

	VulkanContext::~VulkanContext()
	{
		if (auto handle = end())
		{
			handle->release();
		}
	}

	VulkanContext& VulkanContext::begin(const RHIContextInheritanceInfo* inheritance)
	{
		if (m_cmd == nullptr)
		{
			m_cmd = VulkanCommandBufferManager::instance()->request_handle(m_flags);

			if (is_secondary())
			{
				trinex_assert(inheritance);

				VulkanContext* primary = static_cast<VulkanContext*>(inheritance->primary);

				vk::CommandBufferBeginInfo begin_info;
				vk::CommandBufferInheritanceInfo inherit;
				vk::CommandBufferInheritanceRenderingInfoKHR inherit_rendering;
				vk::Format formats[4] = {vk::Format::eUndefined};

				begin_info.setPInheritanceInfo(&inherit);

				if (inheritance->flags & RHIContextInheritanceFlags::RenderPassContinue)
				{
					inherit.pNext = &inherit_rendering;
					inherit_rendering.setColorAttachmentCount(4);
					inherit_rendering.setPColorAttachmentFormats(formats);
					inherit_rendering.setRasterizationSamples(vk::SampleCountFlagBits::e1);

					for (u32 i = 0; i < 4; ++i)
					{
						formats[i] = VulkanEnums::format_of(inheritance->colors[i].as_color_format());
					}

					vk::Format depth = VulkanEnums::format_of(inheritance->depth.as_color_format());

					if (VulkanEnums::is_depth_format(depth))
					{
						inherit_rendering.setDepthAttachmentFormat(depth);
					}

					if (VulkanEnums::is_stencil_format(depth))
					{
						inherit_rendering.setStencilAttachmentFormat(depth);
					}

					begin_info.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue);
				}

				m_cmd->begin(begin_info);
				copy_state(primary);
			}
			else
			{
				m_cmd->begin();
				reset();
			}
		}
		return *this;
	}

	VulkanCommandHandle* VulkanContext::end()
	{
		if (m_cmd == nullptr)
			return nullptr;

		VulkanCommandHandle* cmd = m_cmd;
		m_cmd                    = nullptr;
		cmd->end();

		return cmd;
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

	VulkanContext& VulkanContext::execute(RHICommandHandle* handle)
	{
		m_cmd->executeCommands(*static_cast<VulkanCommandHandle*>(handle));

		handle->add_reference();
		m_cmd->add_stagging(handle);
		return *this;
	}

	VulkanContext& VulkanContext::viewport(const RHIViewport& viewport)
	{
		trinex_profile_cpu_n("VulkanContext::viewport");

		if (m_viewport != viewport)
		{
			m_viewport = viewport;
			m_dirty_flags |= Viewport;
		}

		return *this;
	}

	VulkanContext& VulkanContext::scissor(const RHIScissor& scissor)
	{
		trinex_profile_cpu_n("VulkanContext::scissor");

		if (m_scissor != scissor)
		{
			m_scissor = scissor;
			m_dirty_flags |= Scissor;
		}

		return *this;
	}

	VulkanContext& VulkanContext::draw(RHITopology topology, usize vertex_count, usize vertices_offset, usize instances)
	{
		trinex_profile_cpu_n("VulkanContext::draw");
		flush_graphics(topology)->draw(vertex_count, instances, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_indexed(RHITopology topology, usize indices_count, usize indices_offset,
	                                           usize vertices_offset, usize instances)
	{
		trinex_profile_cpu_n("VulkanContext::draw_indexed");
		flush_graphics(topology)->drawIndexed(indices_count, instances, indices_offset, vertices_offset, 0);
		return *this;
	}

	VulkanContext& VulkanContext::draw_mesh(u32 x, u32 y, u32 z)
	{
		trinex_profile_cpu_n("VulkanContext::draw_mesh");
		flush_graphics()->drawMeshTasksEXT(x, y, z);
		return *this;
	}

	VulkanContext& VulkanContext::dispatch(u32 group_x, u32 group_y, u32 group_z)
	{
		trinex_profile_cpu_n("VulkanContext::dispatch");
		flush_compute()->dispatch(group_x, group_y, group_z);
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

	void VulkanContext::destroy()
	{
		trx_delete this;
	}

	RHIContext* VulkanAPI::create_context(RHIContextFlags flags)
	{
		return trx_new VulkanContext(flags);
	}
}// namespace Trinex
