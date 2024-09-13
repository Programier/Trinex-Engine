#include <Core/exception.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
	static void transition_image_layout(vk::Image image, vk::ImageLayout current, vk::ImageLayout new_layout,
	                                    vk::CommandBuffer& cmd)
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout           = current;
		barrier.newLayout           = new_layout;
		barrier.image               = image;
		barrier.subresourceRange    = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		Barrier::transition_image_layout(cmd, barrier);
	}

	static vk::Filter filter_of(SamplerFilter filter)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
			case SamplerFilter::Trilinear:
				return vk::Filter::eLinear;

			default:
				return vk::Filter::eNearest;
		}
	}

	void VulkanViewport::destroy_image_views()
	{
		for (auto& view : m_image_views)
		{
			API->m_device.destroyImageView(view);
		}

		m_image_views.clear();
	}

	void VulkanViewport::before_begin_render()
	{
		API->m_state.reset();
		API->m_state.m_current_viewport = this;
	}

	void VulkanViewport::after_end_render()
	{
		API->m_state.m_current_viewport = nullptr;
	}

	void VulkanViewport::begin_render()
	{
		SyncObject& sync = *current_sync_object();

		if (auto fence = sync.fence())
		{
			while (vk::Result::eTimeout == API->m_device.waitForFences(fence, VK_TRUE, UINT64_MAX))
			{
			}

			API->m_device.resetFences(fence);
		}

		API->current_command_buffer()->reset();
		API->current_command_buffer_handle().begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
	}

	void VulkanViewport::end_render()
	{
		if (API->m_state.m_render_target)
		{
			API->m_state.m_render_target->unbind();
		}

		API->current_command_buffer_handle().end();

		SyncObject& sync = *current_sync_object();

		static const vk::PipelineStageFlags wait_flags(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		vk::SubmitInfo submit_info({}, {}, API->current_command_buffer_handle(), {});

		auto image_present   = sync.image_present();
		auto render_finished = sync.render_finished();

		if (image_present)
		{
			submit_info.setWaitSemaphores(image_present);
			submit_info.setWaitDstStageMask(wait_flags);
		}

		if (render_finished)
			submit_info.setSignalSemaphores(render_finished);

		API->m_graphics_queue.submit(submit_info, sync.fence());
	}

	void VulkanViewport::on_resize(const Size2D& new_size)
	{}

	void VulkanViewport::on_orientation_changed(Orientation orientation)
	{}

	void VulkanViewport::vsync(bool flag)
	{}

	void VulkanViewport::bind()
	{
		return render_target()->bind();
	}

	void VulkanViewport::blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter)
	{
		auto current = API->m_state.m_render_target;

		if (current)
		{
			current->unbind();
		}

		auto& cmd = API->current_command_buffer_handle();
		auto src  = surface->rhi_object<VulkanSurface>();

		auto src_layout = src->layout();
		src->change_layout(vk::ImageLayout::eTransferSrcOptimal, cmd);

		auto dst    = current_image();
		auto layout = default_image_layout();
		transition_image_layout(dst, layout, vk::ImageLayout::eTransferDstOptimal, cmd);

		auto src_end = src_rect.position + src_rect.size;
		auto dst_end = dst_rect.position + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.position.x, src_rect.position.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.position.x, dst_end.y, 0), vk::Offset3D(dst_end.x, dst_rect.position.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(src->aspect(), 0, 0, src->layer_count()));
		blit.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		cmd.blitImage(src->image(), src->layout(), dst, vk::ImageLayout::eTransferDstOptimal, blit, filter_of(filter));

		transition_image_layout(dst, vk::ImageLayout::eTransferDstOptimal, default_image_layout(), cmd);
		src->change_layout(src_layout, cmd);

		if (current)
		{
			current->bind();
		}
	}

	void VulkanViewport::clear_color(const Color& color)
	{
		auto current = API->m_state.m_render_target;

		if (current)
		{
			current->unbind();
		}

		auto& cmd   = API->current_command_buffer_handle();
		auto dst    = current_image();
		auto layout = default_image_layout();

		transition_image_layout(dst, layout, vk::ImageLayout::eTransferDstOptimal, cmd);

		cmd.clearColorImage(dst, vk::ImageLayout::eTransferDstOptimal, vk::ClearColorValue(color.r, color.g, color.b, color.a),
		                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		transition_image_layout(dst, vk::ImageLayout::eTransferDstOptimal, layout, cmd);

		if (current)
		{
			current->bind();
		}
	}

	VulkanCommandBuffer* VulkanAPI::current_command_buffer()
	{
		return m_state.m_current_viewport->current_command_buffer();
	}

	vk::CommandBuffer& VulkanAPI::current_command_buffer_handle()
	{
		return m_state.m_current_viewport->current_command_buffer()->m_cmd;
	}

	// Surface Viewport
	VulkanSurfaceViewport::SyncObject::SyncObject()
	{
		m_fence = API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	VulkanSurfaceViewport::SyncObject::~SyncObject()
	{
		DESTROY_CALL(destroyFence, m_fence);
	}

	VulkanSurfaceViewport::VulkanSurfaceViewport()
	{
		m_command_buffer = new VulkanCommandBuffer();
		m_sync_object    = new SyncObject();
		m_render_target  = nullptr;
	}

	VulkanSurfaceViewport::~VulkanSurfaceViewport()
	{
		API->wait_idle();
		delete m_command_buffer;
		delete m_sync_object;
	}

	VulkanCommandBuffer* VulkanSurfaceViewport::current_command_buffer()
	{
		return m_command_buffer;
	}

	VulkanSurfaceViewport::SyncObject* VulkanSurfaceViewport::current_sync_object()
	{
		return m_sync_object;
	}

	vk::Image VulkanSurfaceViewport::current_image()
	{
		return m_surface[0]->rhi_object<VulkanSurface>()->image();
	}

	vk::ImageLayout VulkanSurfaceViewport::default_image_layout()
	{
		return vk::ImageLayout::eShaderReadOnlyOptimal;
	}

	VulkanRenderTargetBase* VulkanSurfaceViewport::render_target()
	{
		return m_render_target;
	}

	bool VulkanSurfaceViewport::is_window_viewport()
	{
		return false;
	}

	VulkanViewport* VulkanSurfaceViewport::init(SurfaceRenderViewport* viewport)
	{
		m_surface[0] = viewport->render_surface();

		if (m_surface[0])
			m_render_target = VulkanRenderTarget::find_or_create(m_surface, nullptr);
		return this;
	}

	void VulkanSurfaceViewport::begin_render()
	{
		VulkanViewport::before_begin_render();
		VulkanViewport::begin_render();

		if (m_render_target)
		{
			Size2D rt_size = m_render_target->state()->m_size;
			ViewPort viewport;
			viewport.pos       = {0.f, 0.f};
			viewport.size      = rt_size;
			viewport.min_depth = 0.f;
			viewport.max_depth = 1.f;
			API->viewport(viewport);

			Scissor scissor;
			scissor.pos  = {0.f, 0.f};
			scissor.size = rt_size;
			API->scissor(scissor);
		}
	}

	void VulkanSurfaceViewport::end_render()
	{
		VulkanViewport::end_render();
		VulkanViewport::after_end_render();
	}

	// Window Viewport

	VulkanWindowViewport::SyncObject::SyncObject()
	{
		m_image_present   = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
		m_render_finished = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
		m_fence           = API->m_device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
	}

	VulkanWindowViewport::SyncObject::~SyncObject()
	{
		DESTROY_CALL(destroySemaphore, m_image_present);
		DESTROY_CALL(destroyFence, m_fence);
		DESTROY_CALL(destroySemaphore, m_render_finished);
	}

	VulkanCommandBuffer* VulkanWindowViewport::current_command_buffer()
	{
		return &m_command_buffers[API->m_current_buffer];
	}

	VulkanWindowViewport::SyncObject* VulkanWindowViewport::current_sync_object()
	{
		return &m_sync_objects[API->m_current_buffer];
	}

	vk::Image VulkanWindowViewport::current_image()
	{
		return m_images[m_buffer_index];
	}

	vk::ImageLayout VulkanWindowViewport::default_image_layout()
	{
		return vk::ImageLayout::ePresentSrcKHR;
	}

	VulkanRenderTargetBase* VulkanWindowViewport::render_target()
	{
		return m_render_target->frame();
	}

	bool VulkanWindowViewport::is_window_viewport()
	{
		return true;
	}

	VulkanViewport* VulkanWindowViewport::init(WindowRenderViewport* viewport, bool vsync)
	{
		m_viewport = viewport;
		m_vsync    = vsync;
		m_surface  = API->m_window == viewport->window() ? API->m_surface : API->create_surface(viewport->window());

		m_command_buffers.resize(API->m_framebuffers_count);
		m_sync_objects.resize(API->m_framebuffers_count);

		create_swapchain();
		create_render_target();
		return this;
	}

	void VulkanWindowViewport::on_resize(const Size2D& new_size)
	{
		m_need_recreate_swap_chain = true;
		reinterpret_cast<VulkanWindowRenderTarget*>(m_render_target)->frame()->size(new_size.x, new_size.y);
	}

	void VulkanWindowViewport::on_orientation_changed(Orientation orientation)
	{
		m_need_recreate_swap_chain = true;
	}

	void VulkanWindowViewport::vsync(bool flag)
	{
		m_need_recreate_swap_chain = true;
	}

	void VulkanWindowViewport::create_swapchain()
	{
		// Creating swapchain
		vulkan_info_log("Vulkan API", "Creating new swapchain");
		vkb::SwapchainBuilder swapchain_builder(API->m_bootstrap_device, m_surface);

		if (m_swapchain)
		{
			swapchain_builder.set_old_swapchain(m_swapchain->swapchain);
		}

		swapchain_builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(API->present_mode_of(m_vsync, m_surface)));
		auto capabilities = API->m_physical_device.getSurfaceCapabilitiesKHR(m_surface);

		size_t images_count = API->m_framebuffers_count;
		swapchain_builder.set_desired_min_image_count(images_count).set_required_min_image_count(images_count);

		swapchain_builder.add_image_usage_flags(
		        static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst));

		if (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			swapchain_builder.set_pre_transform_flags(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
		else
			swapchain_builder.set_pre_transform_flags(static_cast<VkSurfaceTransformFlagBitsKHR>(capabilities.currentTransform));


		VkSurfaceFormatKHR f;
		f.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		f.format     = VK_FORMAT_B8G8R8A8_UNORM;
		swapchain_builder.set_desired_format(f);

		auto swap_ret = swapchain_builder.build();

		if (!swap_ret)
		{
			throw std::runtime_error(swap_ret.error().message());
		}

		if (m_swapchain)
		{
			destroy_swapchain(false);
		}

		if (!m_swapchain)
			m_swapchain = new vkb::Swapchain();

		(*m_swapchain) = swap_ret.value();

		auto images_result = m_swapchain->get_images();
		if (!images_result.has_value())
			throw EngineException(images_result.error().message());

		m_images = Vector<VkImage>(images_result.value().begin(), images_result.value().end());

		auto image_views_result = m_swapchain->get_image_views();
		if (!image_views_result.has_value())
			throw EngineException(image_views_result.error().message());
		m_image_views = std::move(image_views_result.value());

		auto cmd = API->begin_single_time_command_buffer();

		for (VkImage image : m_images)
		{
			transition_image_layout(vk::Image(image), vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, cmd);
		}

		API->end_single_time_command_buffer(cmd);
	}

	void VulkanWindowViewport::recreate_swapchain()
	{
		if (m_need_recreate_swap_chain)
		{
			m_need_recreate_swap_chain              = false;
			VulkanWindowRenderTarget* render_target = reinterpret_cast<VulkanWindowRenderTarget*>(m_render_target);
			API->wait_idle();

			render_target->destroy();
			create_swapchain();
			create_render_target();

			m_sync_objects.clear();
			m_sync_objects.resize(API->m_framebuffers_count);
		}
	}

	void VulkanWindowViewport::create_render_target()
	{
		if (!m_render_target)
			m_render_target = new VulkanWindowRenderTarget();

		m_render_target->resize_count(m_swapchain->image_count);
		m_render_target->init(this);
	}

	void VulkanWindowViewport::destroy_swapchain(bool fully)
	{
		vkb::destroy_swapchain(*m_swapchain);
		destroy_image_views();

		if (fully)
		{
			delete m_swapchain;
			m_swapchain = nullptr;
		}
	}

	vk::ResultValue<uint32_t> VulkanWindowViewport::swapchain_image_index()
	{
		SyncObject& sync = m_sync_objects[API->m_current_buffer];
		try
		{
			return API->m_device.acquireNextImageKHR(m_swapchain->swapchain, UINT64_MAX, sync.m_image_present, nullptr);
		}
		catch (const std::exception& e)
		{
			return vk::ResultValue<uint32_t>(vk::Result::eErrorOutOfDateKHR, -1);
		}
	}

	VulkanWindowViewport::~VulkanWindowViewport()
	{
		API->wait_idle();

		delete m_render_target;
		destroy_swapchain(true);
		vk::Instance(API->m_instance.instance).destroySurfaceKHR(m_surface);
	}


	void VulkanWindowViewport::begin_render()
	{
		before_begin_render();
		recreate_swapchain();

		VulkanViewport::begin_render();

		auto current_buffer_index = swapchain_image_index();

		if (current_buffer_index.result == vk::Result::eErrorOutOfDateKHR)
		{
			m_need_recreate_swap_chain = true;
			recreate_swapchain();
			return begin_render();
		}

		if (current_buffer_index.result != vk::Result::eSuccess && current_buffer_index.result != vk::Result::eSuboptimalKHR)
		{
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		m_buffer_index = current_buffer_index.value;

		Size2D rt_size = render_target()->state()->m_size;
		ViewPort viewport;
		viewport.pos       = {0.f, 0.f};
		viewport.size      = rt_size;
		viewport.min_depth = 0.f;
		viewport.max_depth = 1.f;
		API->viewport(viewport);

		Scissor scissor;
		scissor.pos  = {0.f, 0.f};
		scissor.size = rt_size;
		API->scissor(scissor);
	}


	void VulkanWindowViewport::end_render()
	{
		VulkanViewport::end_render();

		SyncObject& sync = m_sync_objects[API->m_current_buffer];

		vk::SwapchainKHR swapchain = m_swapchain->swapchain;
		vk::PresentInfoKHR present_info(sync.m_render_finished, swapchain, m_buffer_index);
		vk::Result result;

		try
		{
			result = API->m_present_queue.presentKHR(present_info);
		}
		catch (const std::exception& e)
		{
			result = vk::Result::eErrorOutOfDateKHR;
		}

		switch (result)
		{
			case vk::Result::eSuccess:
				break;

			case vk::Result::eSuboptimalKHR:
#if SKIP_SUBOPTIMAL_KHR_ERROR
				break;
#endif
			case vk::Result::eErrorOutOfDateKHR:
				m_need_recreate_swap_chain = true;
				break;

			default:
				assert(false);
		}


		recreate_swapchain();
		after_end_render();
	}

	// Creating Viewports

	RHI_Viewport* VulkanAPI::create_viewport(SurfaceRenderViewport* viewport)
	{
		VulkanSurfaceViewport* vulkan_viewport = new VulkanSurfaceViewport();
		vulkan_viewport->init(viewport);
		return vulkan_viewport;
	}

	RHI_Viewport* VulkanAPI::create_viewport(WindowRenderViewport* viewport, bool vsync)
	{
		VulkanWindowViewport* vulkan_viewport = new VulkanWindowViewport();
		vulkan_viewport->init(viewport, vsync);
		return vulkan_viewport;
	}
}// namespace Engine
