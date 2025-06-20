#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/profiler.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_command_buffer.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
	vk::Image VulkanViewport::current_image()
	{
		return m_swapchain->backbuffer()->m_image;
	}

	vk::ImageLayout VulkanViewport::default_image_layout()
	{
		return vk::ImageLayout::ePresentSrcKHR;
	}

	VulkanSwapchainRenderTarget* VulkanViewport::render_target()
	{
		return m_swapchain->backbuffer();
	}

	VulkanViewport* VulkanViewport::init(WindowRenderViewport* viewport, bool vsync)
	{
		m_swapchain = new VulkanSwapchain(viewport->window(), vsync);
		return this;
	}

	void VulkanViewport::destroy_image_views()
	{
		for (auto& view : m_image_views)
		{
			API->m_device.destroyImageView(view);
		}

		m_image_views.clear();
	}

	void VulkanViewport::present()
	{
		trinex_profile_cpu_n("VulkanWindowViewport::end_render");
		auto cmd = API->current_command_buffer();
		render_target()->change_layout(vk::ImageLayout::ePresentSrcKHR);
		cmd->add_wait_semaphore(vk::PipelineStageFlagBits::eColorAttachmentOutput, m_swapchain->image_present_semaphore());
		API->m_cmd_manager->submit(m_swapchain->render_finished_semaphore());
		m_swapchain->try_present(&VulkanSwapchain::do_present, cmd, true);

		API->m_state_manager->submit();
	}

	void VulkanViewport::on_resize(const Size2D& new_size)
	{
		m_swapchain->m_need_recreate = true;
	}

	void VulkanViewport::vsync(bool flag)
	{
		m_swapchain->vsync(flag);
	}

	void VulkanViewport::bind()
	{
		return render_target()->bind();
	}

	void VulkanViewport::blit_target(RHI_RenderTargetView* surface, const RHIRect& src_rect, const RHIRect& dst_rect,
	                                 RHISamplerFilter filter)
	{
		auto cmd = API->end_render_pass();

		auto src = static_cast<VulkanTextureRTV*>(surface);
		src->change_layout(vk::ImageLayout::eTransferSrcOptimal);

		auto dst = render_target();
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		Vector2i src_end = src_rect.pos + src_rect.size;
		Vector2i dst_end = dst_rect.pos + dst_rect.size;

		vk::ImageBlit blit;
		blit.setSrcOffsets({vk::Offset3D(src_rect.pos.x, src_rect.pos.y, 0), vk::Offset3D(src_end.x, src_end.y, 1)});
		blit.setDstOffsets({vk::Offset3D(dst_rect.pos.x, dst_end.y, 0), vk::Offset3D(dst_end.x, dst_rect.pos.y, 1)});

		blit.setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		blit.setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
		cmd->blitImage(src->image(), src->layout(), dst->m_image, vk::ImageLayout::eTransferDstOptimal, blit,
		               VulkanEnums::filter_of(filter));
	}

	void VulkanViewport::clear_color(const LinearColor& color)
	{
		auto cmd = API->end_render_pass();

		auto dst = render_target();
		dst->change_layout(vk::ImageLayout::eTransferDstOptimal);

		cmd->clearColorImage(dst->m_image, vk::ImageLayout::eTransferDstOptimal,
		                     vk::ClearColorValue(color.r, color.g, color.b, color.a),
		                     vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	}

	VulkanViewport::~VulkanViewport()
	{
		API->wait_idle();
		delete m_swapchain;
	}

	VulkanSwapchain::Semaphore::Semaphore()
	{
		m_semaphore = API->m_device.createSemaphore(vk::SemaphoreCreateInfo());
	}

	VulkanSwapchain::Semaphore::Semaphore(Semaphore&& semaphore)
	{
		m_semaphore           = semaphore.m_semaphore;
		semaphore.m_semaphore = VK_NULL_HANDLE;
	}

	VulkanSwapchain::Semaphore::~Semaphore()
	{
		if (m_semaphore)
			API->m_device.destroySemaphore(m_semaphore);
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

		vkb::SwapchainBuilder builder(API->m_physical_device, API->m_device, m_surface, API->m_graphics_queue->index(),
		                              API->m_graphics_queue->index());

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
		m_image_present_semaphores.resize(images.size());
		m_render_finished_semaphores.resize(images.size());

		Size2D size = {swapchain->extent.width, swapchain->extent.height};

		for (int_t i = 0; auto& backbuffer : m_backbuffers)
		{
			backbuffer = new VulkanSwapchainRenderTarget(images[i], image_views[i], size, vk::Format(swapchain->image_format));
			++i;
		}

		m_swapchain = swapchain->swapchain;
		auto cmd    = API->current_command_buffer();
		cmd->end();
		cmd->submit();
		cmd->wait();
		cmd->begin();
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::release()
	{
		API->wait_idle();

		for (VulkanSwapchainRenderTarget* backbuffer : m_backbuffers)
		{
			delete backbuffer;
		}

		m_backbuffers.clear();

		if (m_swapchain)
			API->m_device.destroySwapchainKHR(m_swapchain);

		m_sync_index  = 0;
		m_image_index = -1;
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

	int_t VulkanSwapchain::acquire_image_index(VulkanCommandBuffer* cmd_buffer)
	{
		trinex_profile_cpu_n("VulkanSwapchain::acquire_image_index");

		const auto prev_sync_index = m_sync_index;
		m_sync_index               = (m_sync_index + 1) % m_image_present_semaphores.size();

		vk::ResultValue<uint32_t> result = vk::ResultValue<uint32_t>(vk::Result::eSuccess, 0);

		try
		{
			result = API->m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX,
			                                           m_image_present_semaphores[m_sync_index].semaphore());
		}
		catch (const vk::OutOfDateKHRError& e)
		{
			m_sync_index = prev_sync_index;
			return OutOfDate;
		}
		catch (const vk::SurfaceLostKHRError& e)
		{
			m_sync_index = prev_sync_index;
			return SurfaceLost;
		}

		if (result.result == vk::Result::eErrorOutOfDateKHR)
		{
			m_sync_index = prev_sync_index;
			return OutOfDate;
		}
		if (result.result == vk::Result::eErrorSurfaceLostKHR)
		{
			m_sync_index = prev_sync_index;
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

	int_t VulkanSwapchain::do_present(VulkanCommandBuffer* cmd_buffer)
	{
		if (m_image_index == -1)
			return Status::Success;

		auto image_index             = static_cast<uint32_t>(m_image_index);
		vk::Semaphore wait_semaphore = render_finished_semaphore();
		vk::PresentInfoKHR present_info(wait_semaphore, m_swapchain, image_index);
		vk::Result result;
		m_image_index = -1;

		try
		{
			trinex_profile_cpu_n("VulkanSwapchain::Present KHR");
			result = API->m_graphics_queue->queue().presentKHR(present_info);
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

	int_t VulkanSwapchain::try_present(int_t (VulkanSwapchain::*callback)(VulkanCommandBuffer*), VulkanCommandBuffer* cmd_buffer,
	                                   bool skip_on_out_of_date)
	{
		if (m_need_recreate)
		{
			recreate();
			return try_present(callback, cmd_buffer, skip_on_out_of_date);
		}

		int_t status = (this->*callback)(cmd_buffer);

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

			status = (this->*callback)(cmd_buffer);
		}

		return status;
	}

	vk::Semaphore VulkanSwapchain::render_finished_semaphore()
	{
		backbuffer();
		return m_render_finished_semaphores[m_image_index].semaphore();
	}

	vk::Semaphore VulkanSwapchain::image_present_semaphore()
	{
		backbuffer();
		return m_image_present_semaphores[m_sync_index].semaphore();
	}

	VulkanSwapchainRenderTarget* VulkanSwapchain::backbuffer()
	{
		if (m_image_index == -1)
		{
			trinex_profile_cpu_n("VulkanSwapchain::backbuffer");

			if (try_present(&VulkanSwapchain::acquire_image_index, nullptr, false) < 0)
			{
				throw EngineException("Failed to acquire image index");
			}
		}

		return m_backbuffers[m_image_index];
	}

	RHI_Viewport* VulkanAPI::create_viewport(WindowRenderViewport* viewport, bool vsync)
	{
		VulkanViewport* vulkan_viewport = new VulkanViewport();
		vulkan_viewport->init(viewport, vsync);
		return vulkan_viewport;
	}
}// namespace Engine
