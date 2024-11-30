#include <Core/exception.hpp>
#include <Core/profiler.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_queue.hpp>
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

	vk::Semaphore* VulkanViewport::SyncObject::image_present()
	{
		return nullptr;
	}

	vk::Semaphore* VulkanViewport::SyncObject::render_finished()
	{
		return nullptr;
	}

	VulkanViewport::SyncObject::~SyncObject()
	{}

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
	{}

	void VulkanViewport::end_render()
	{
		API->m_cmd_manager->submit_active_cmd_buffer(current_sync_object()->render_finished());
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
		auto cmd            = API->current_command_buffer();
		bool in_render_pass = cmd->is_inside_render_pass();

		if (in_render_pass)
			API->end_render_pass();

		auto src = surface->rhi_object<VulkanSurface>();

		auto src_layout = src->layout();
		src->change_layout(vk::ImageLayout::eTransferSrcOptimal, cmd->m_cmd);

		auto dst    = current_image();
		auto layout = default_image_layout();
		transition_image_layout(dst, layout, vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		auto src_end = src_rect.position + src_rect.size;
		auto dst_end = dst_rect.position + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.position.x, src_rect.position.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.position.x, dst_end.y, 0), vk::Offset3D(dst_end.x, dst_rect.position.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(src->aspect(), 0, 0, src->layer_count()));
		blit.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		cmd->m_cmd.blitImage(src->image(), src->layout(), dst, vk::ImageLayout::eTransferDstOptimal, blit, filter_of(filter));

		transition_image_layout(dst, vk::ImageLayout::eTransferDstOptimal, default_image_layout(), cmd->m_cmd);
		src->change_layout(src_layout, cmd->m_cmd);

		if (in_render_pass)
		{
			API->begin_render_pass();
		}
	}

	void VulkanViewport::clear_color(const Color& color)
	{
		auto cmd            = API->current_command_buffer();
		bool in_render_pass = cmd->is_inside_render_pass();

		if (in_render_pass)
			API->end_render_pass();

		auto dst    = current_image();
		auto layout = default_image_layout();

		transition_image_layout(dst, layout, vk::ImageLayout::eTransferDstOptimal, cmd->m_cmd);

		cmd->m_cmd.clearColorImage(dst, vk::ImageLayout::eTransferDstOptimal,
		                           vk::ClearColorValue(color.r, color.g, color.b, color.a),
		                           vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

		transition_image_layout(dst, vk::ImageLayout::eTransferDstOptimal, layout, cmd->m_cmd);

		if (in_render_pass)
			API->begin_render_pass();
	}

	// Surface Viewport
	VulkanSurfaceViewport::VulkanSurfaceViewport()
	{
		m_sync_object   = new SyncObject();
		m_render_target = nullptr;
	}

	VulkanSurfaceViewport::~VulkanSurfaceViewport()
	{
		API->wait_idle();
		delete m_sync_object;
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
	}

	void VulkanSurfaceViewport::end_render()
	{
		VulkanViewport::end_render();
		VulkanViewport::after_end_render();
	}

	// Window Viewport

	VulkanBackBuffer& VulkanBackBuffer::setup(vk::Image backbuffer, vk::ImageView view, Size2D size, vk::Format format)
	{
		m_image_present_semaphore   = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
		m_render_finished_semaphore = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());

		m_render_target = new VulkanSwapchainRenderTarget(backbuffer, view, size, format);
		return *this;
	}

	VulkanBackBuffer& VulkanBackBuffer::wait_for_command_buffer()
	{
		if (m_command_buffer)
		{
			m_command_buffer->wait();
			m_command_buffer = nullptr;
		}
		return *this;
	}

	VulkanBackBuffer& VulkanBackBuffer::release()
	{
		API->m_device.destroySemaphore(m_image_present_semaphore);
		API->m_device.destroySemaphore(m_render_finished_semaphore);

		delete m_render_target;
		m_render_target = nullptr;
		return *this;
	}

	VulkanSwapchain::VulkanSwapchain(Window* window, bool vsync) : m_surface(API->create_surface(window))
	{
		this->vsync(vsync, true);
		create();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		release();
		vk::Instance(API->m_instance.instance).destroySurfaceKHR(m_surface);
	}

	VulkanSwapchain& VulkanSwapchain::vsync(bool flag, bool is_init)
	{
		if (is_init)
		{
			m_present_mode = API->present_mode_of(flag, m_surface);
		}
		else
		{
			auto mode = API->present_mode_of(flag, m_surface);
			if (mode != m_present_mode)
			{
				m_present_mode  = mode;
				m_need_recreate = true;
			}
		}
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::create(vk::SwapchainKHR* old)
	{
		info_log("Vulkan API", "Creating new swapchain");
		m_need_recreate = false;

		vkb::SwapchainBuilder builder(API->m_physical_device, API->m_device, m_surface, API->m_graphics_queue->m_index,
		                              API->m_present_queue->m_index);

		builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(m_present_mode));
		auto capabilities = API->m_physical_device.getSurfaceCapabilitiesKHR(m_surface);

		builder.add_image_usage_flags(
		        static_cast<VkImageUsageFlags>(vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst));

		if (capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
			builder.set_pre_transform_flags(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
		else
			builder.set_pre_transform_flags(static_cast<VkSurfaceTransformFlagBitsKHR>(capabilities.currentTransform));


		VkSurfaceFormatKHR f;
		f.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		f.format     = VK_FORMAT_B8G8R8A8_UNORM;
		builder.set_desired_format(f);

		if (old)
		{
			builder.set_old_swapchain(*old);
		}

		auto swapchain = builder.build();

		if (!swapchain)
		{
			throw EngineException(swapchain.error().message());
		}

		auto images_result = swapchain->get_images();
		if (!images_result.has_value())
			throw EngineException(images_result.error().message());

		auto image_views_result = swapchain->get_image_views();
		if (!image_views_result.has_value())
			throw EngineException(image_views_result.error().message());

		auto& images      = images_result.value();
		auto& image_views = image_views_result.value();

		m_backbuffers.resize(images.size());
		Size2D size = {swapchain->extent.width, swapchain->extent.height};

		for (int_t i = 0; auto& backbuffer : m_backbuffers)
		{
			backbuffer.setup(images[i], image_views[i], size, vk::Format(swapchain->image_format));
			++i;
		}

		auto cmd = API->begin_single_time_command_buffer();

		for (VkImage image : images)
		{
			transition_image_layout(vk::Image(image), vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR, cmd);
		}

		API->end_single_time_command_buffer(cmd);
		m_swapchain = swapchain->swapchain;
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::release()
	{
		API->wait_idle();

		for (auto& backbuffer : m_backbuffers)
		{
			backbuffer.wait_for_command_buffer();
		}

		for (auto& backbuffer : m_backbuffers)
		{
			backbuffer.release();
		}

		m_backbuffers.clear();

		if (m_swapchain)
			API->m_device.destroySwapchainKHR(m_swapchain);

		m_buffer_index = 0;
		m_image_index  = -1;
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::recreate()
	{
		vk::SwapchainKHR swapchain = m_swapchain;
		m_swapchain                = VK_NULL_HANDLE;

		release();
		create(&swapchain);

		API->m_device.destroySwapchainKHR(swapchain);

		return *this;
	}

	int_t VulkanSwapchain::acquire_image_index()
	{
		trinex_profile_cpu_n("VulkanSwapchain::acquire_image_index");

		const auto prev_buffer_index = m_buffer_index;
		m_buffer_index               = (m_buffer_index + 1) % m_backbuffers.size();

		auto& backbuffer = m_backbuffers[m_buffer_index].wait_for_command_buffer();

		vk::ResultValue<uint32_t> result = vk::ResultValue<uint32_t>(vk::Result::eSuccess, 0);

		try
		{
			result = API->m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, backbuffer.m_image_present_semaphore);
		}
		catch (const vk::OutOfDateKHRError& e)
		{
			m_buffer_index = prev_buffer_index;
			return OutOfDate;
		}
		catch (const vk::SurfaceLostKHRError& e)
		{
			m_buffer_index = prev_buffer_index;
			return SurfaceLost;
		}

		if (result.result == vk::Result::eErrorOutOfDateKHR)
		{
			m_buffer_index = prev_buffer_index;
			return OutOfDate;
		}
		if (result.result == vk::Result::eErrorSurfaceLostKHR)
		{
			m_buffer_index = prev_buffer_index;
			return SurfaceLost;
		}

		if (result.result == vk::Result::eErrorValidationFailedEXT)
		{
			error_log("Vulkan", "vkAcquireNextImageKHR failed with validation error");
		}
		else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
		{
			throw EngineException("Failed to acquire image index");
		}

		m_image_index = result.value;
		return m_image_index;
	}

	int_t VulkanSwapchain::do_present()
	{
		if (m_image_index == -1)
			return Status::Success;

		auto image_index = static_cast<uint32_t>(m_image_index);
		vk::PresentInfoKHR present_info(*render_finished_semaphore(), m_swapchain, image_index);
		vk::Result result;
		m_image_index = -1;

		m_backbuffers[m_buffer_index].m_command_buffer = API->current_command_buffer();

		try
		{
			trinex_profile_cpu_n("VulkanSwapchain::Present KHR");
			result = API->m_present_queue->m_queue.presentKHR(present_info);
		}
		catch (const vk::OutOfDateKHRError& e)
		{
			return OutOfDate;
		}
		catch (const vk::SurfaceLostKHRError& e)
		{
			return SurfaceLost;
		}

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			return OutOfDate;
		}
		if (result == vk::Result::eErrorSurfaceLostKHR)
		{
			return SurfaceLost;
		}

		return Status::Success;
	}

	int_t VulkanSwapchain::try_present(int_t (VulkanSwapchain::*callback)(), bool skip_on_out_of_date)
	{
		if (m_need_recreate)
		{
			recreate();
			return try_present(callback, skip_on_out_of_date);
		}

		int_t status = (this->*callback)();

		while (status < 0)
		{
			if (status == Status::OutOfDate)
			{
				if (skip_on_out_of_date)
					return status;
			}
			else if (status == Status::SurfaceLost)
			{
				warn_log("Vulkan", "Swapchain surface lost");
			}
			else
			{
				throw EngineException("Failed to present swapchain");
			}

			ThisThread::sleep_for(0.1);
			recreate();

			status = (this->*callback)();
		}

		return status;
	}

	vk::Semaphore* VulkanSwapchain::render_finished_semaphore()
	{
		backbuffer();
		return &m_backbuffers[m_buffer_index].m_render_finished_semaphore;
	}

	vk::Semaphore* VulkanSwapchain::image_present_semaphore()
	{
		backbuffer();
		return &m_backbuffers[m_buffer_index].m_image_present_semaphore;
	}

	VulkanBackBuffer* VulkanSwapchain::backbuffer()
	{
		if (m_image_index == -1)
		{
			trinex_profile_cpu_n("VulkanSwapchain::backbuffer");

			if (try_present(&VulkanSwapchain::acquire_image_index, false) < 0)
			{
				throw EngineException("Failed to acquire image index");
			}
		}

		return &m_backbuffers[m_image_index];
	}

	vk::Image VulkanWindowViewport::current_image()
	{
		return m_swapchain->backbuffer()->m_render_target->m_image;
	}

	vk::ImageLayout VulkanWindowViewport::default_image_layout()
	{
		return vk::ImageLayout::ePresentSrcKHR;
	}

	VulkanRenderTargetBase* VulkanWindowViewport::render_target()
	{
		return m_swapchain->backbuffer()->m_render_target;
	}

	bool VulkanWindowViewport::is_window_viewport()
	{
		return true;
	}

	VulkanViewport* VulkanWindowViewport::init(WindowRenderViewport* viewport, bool vsync)
	{
		m_swapchain = new VulkanSwapchain(viewport->window(), vsync);
		return this;
	}

	void VulkanWindowViewport::on_resize(const Size2D& new_size)
	{
		m_swapchain->m_need_recreate = true;
	}

	void VulkanWindowViewport::on_orientation_changed(Orientation orientation)
	{
		m_swapchain->m_need_recreate = true;
	}

	void VulkanWindowViewport::vsync(bool flag)
	{
		m_swapchain->vsync(flag);
	}

	VulkanWindowViewport::~VulkanWindowViewport()
	{
		API->wait_idle();
		delete m_swapchain;
	}

	void VulkanWindowViewport::begin_render()
	{
		before_begin_render();
	}

	void VulkanWindowViewport::end_render()
	{
		trinex_profile_cpu_n("VulkanWindowViewport::end_render");
		auto cmd = API->current_command_buffer();
		cmd->add_wait_semaphore(vk::PipelineStageFlagBits::eColorAttachmentOutput, *m_swapchain->image_present_semaphore());
		API->m_cmd_manager->submit_active_cmd_buffer(m_swapchain->render_finished_semaphore());
		m_swapchain->try_present(&VulkanSwapchain::do_present, true);

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
