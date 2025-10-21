#include <Core/exception.hpp>
#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/thread.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/config.hpp>
#include <Window/window.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_context.hpp>
#include <vulkan_enums.hpp>
#include <vulkan_queue.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
	class VulkanSwapchainTexture : public VulkanTypedTexture<vk::ImageViewType::e2D, RHITextureType::Texture2D>
	{
	public:
		VulkanSwapchainTexture(vk::Image image, vk::Format format, Vector2u size)
		{
			m_flags        = RHITextureCreateFlags::RenderTarget;
			m_image        = image;
			m_access       = RHIAccess::Undefined;
			m_format       = format;
			m_extent       = vk::Extent3D(size.x, size.y, 1);
			m_mips_count   = 1;
			m_layers_count = 1;
		}

		~VulkanSwapchainTexture() { m_image = vk::Image(); }
	};

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
		m_present_mode = API->present_mode_of(vsync, m_surface);
		create_swapchain();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		release_swapchain();
		vk::Instance(API->m_instance.instance).destroySurfaceKHR(m_surface);
	}

	VulkanSwapchain& VulkanSwapchain::create_swapchain(vk::SwapchainKHR* old)
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

		auto& images = images_result.value();

		m_backbuffers.resize(images.size());
		m_image_present_semaphores.resize(images.size() + 1);
		m_render_finished_semaphores.resize(images.size());

		m_size = {swapchain->extent.width, swapchain->extent.height};

		for (int_t i = 0; auto& backbuffer : m_backbuffers)
		{
			backbuffer = trx_new VulkanSwapchainTexture(images[i], vk::Format(swapchain->image_format), m_size);
			++i;
		}

		m_swapchain = swapchain->swapchain;

		// Creating render buffer
		{
			constexpr RHITextureCreateFlags flags = RHITextureCreateFlags::ShaderResource | RHITextureCreateFlags::RenderTarget;
			vk::Format format                     = vk::Format(swapchain->image_format);
			m_render_buffer = trx_new VulkanTypedTexture<vk::ImageViewType::e2D, RHITextureType::Texture2D>();
			m_render_buffer->create(format, Vector3u(m_size, 1u), 1, 1, flags);
		}
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::release_swapchain()
	{
		API->idle();

		for (VulkanTexture* backbuffer : m_backbuffers)
		{
			trx_delete backbuffer;
		}

		trx_delete m_render_buffer;

		m_backbuffers.clear();
		m_render_buffer = nullptr;

		if (m_swapchain)
			API->m_device.destroySwapchainKHR(m_swapchain);

		m_sync_index  = 0;
		m_image_index = -1;
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::recreate_swapchain()
	{
		vk::SwapchainKHR swapchain = m_swapchain;
		m_swapchain                = VK_NULL_HANDLE;

		release_swapchain();
		create_swapchain(&swapchain);

		API->m_device.destroySwapchainKHR(swapchain);

		return *this;
	}

	int_t VulkanSwapchain::acquire_image_index()
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

	int_t VulkanSwapchain::do_present()
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

	int_t VulkanSwapchain::try_present(int_t (VulkanSwapchain::*callback)(), bool skip_on_out_of_date)
	{
		if (m_need_recreate)
		{
			recreate_swapchain();
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
			recreate_swapchain();

			status = (this->*callback)();
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

	VulkanTexture* VulkanSwapchain::backbuffer()
	{
		if (m_image_index == -1)
		{
			trinex_profile_cpu_n("VulkanSwapchain::backbuffer");

			if (try_present(&VulkanSwapchain::acquire_image_index, false) < 0)
			{
				throw EngineException("Failed to acquire image index");
			}
		}

		return m_backbuffers[m_image_index];
	}

	void VulkanSwapchain::vsync(bool flag)
	{
		auto mode = API->present_mode_of(flag, m_surface);
		if (mode != m_present_mode)
		{
			m_present_mode  = mode;
			m_need_recreate = true;
		}
	}

	RHIRenderTargetView* VulkanSwapchain::as_rtv()
	{
		return m_render_buffer->as_rtv(nullptr);
	}

	RHITexture* VulkanSwapchain::as_texture()
	{
		return m_render_buffer;
	}

	void VulkanSwapchain::resize(const Vector2u& size)
	{
		m_need_recreate = true;
	}

	void VulkanSwapchain::present()
	{
		VulkanContext* ctx = static_cast<VulkanContext*>(RHIContextPool::global_instance()->request_context());
		ctx->begin();

		RHITexture* dst = backbuffer();
		RHITexture* src = m_render_buffer;

		RHITextureRegion region;
		region.extent = {m_size, 1u};

		ctx->barrier(src, RHIAccess::TransferSrc);
		ctx->barrier(dst, RHIAccess::TransferDst);
		ctx->copy_texture_to_texture(src, region, dst, region);
		ctx->barrier(dst, RHIAccess::PresentSrc);

		VulkanCommandHandle* handle = ctx->end();
		{
			auto wait_semaphore          = image_present_semaphore();
			vk::PipelineStageFlags flags = vk::PipelineStageFlagBits::eAllCommands;
			auto finish_semaphore        = render_finished_semaphore();

			vk::CommandBuffer& cmd = *handle;
			vk::SubmitInfo info(wait_semaphore, flags, cmd, finish_semaphore);
			API->m_graphics_queue->submit(info);
		}

		handle->release();
		RHIContextPool::global_instance()->return_context(ctx);

		try_present(&VulkanSwapchain::do_present, true);
	}

	void VulkanSwapchain::destroy()
	{
		trx_delete this;
	}

	RHISwapchain* VulkanAPI::create_swapchain(Window* window, bool vsync)
	{
		return trx_new VulkanSwapchain(window, vsync);
	}

	VulkanAPI& VulkanAPI::present(RHISwapchain* swapchain)
	{
		static_cast<VulkanSwapchain*>(swapchain)->present();
		return *this;
	}
}// namespace Engine
