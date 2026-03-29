#include <Core/logger.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/threading.hpp>
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
#include <vulkan_resource_view.hpp>
#include <vulkan_swapchain.hpp>
#include <vulkan_texture.hpp>

namespace Trinex
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

	static void initialize_semaphores(Vector<RHIResourcePtr<VulkanSemaphore>>& semaphores, usize size)
	{
		semaphores.resize(size);

		for (auto& semaphore : semaphores)
		{
			if (semaphore == nullptr)
			{
				semaphore = trx_new VulkanSemaphore();
			}
		}
	}

	VulkanSwapchain::VulkanSwapchain(Window* window, bool vsync) : m_surface(VulkanAPI::instance()->create_surface(window))
	{
		m_present_mode = VulkanAPI::instance()->present_mode_of(vsync, m_surface);
		create_swapchain();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		release_swapchain();
		vk::Instance(VulkanAPI::instance()->m_instance.instance).destroySurfaceKHR(m_surface);
	}

	VulkanSwapchain& VulkanSwapchain::create_swapchain(vk::SwapchainKHR* old)
	{
		info_log("Vulkan API", "Creating new swapchain");
		m_need_recreate = false;

		vkb::SwapchainBuilder builder(VulkanAPI::instance()->m_physical_device, VulkanAPI::instance()->m_device, m_surface,
		                              VulkanAPI::instance()->m_graphics_queue->index(),
		                              VulkanAPI::instance()->m_graphics_queue->index());

		builder.set_desired_present_mode(static_cast<VkPresentModeKHR>(m_present_mode));
		auto capabilities = vk::check_result(VulkanAPI::instance()->m_physical_device.getSurfaceCapabilitiesKHR(m_surface));

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

		trinex_verify_msg(swapchain.has_value(), swapchain.error().message().c_str());

		auto images_result = swapchain->get_images();
		trinex_verify_msg(images_result.has_value(), images_result.error().message().c_str());
		auto& images = images_result.value();

		m_backbuffers.resize(images.size());
		initialize_semaphores(m_image_present_semaphores, images.size() + 1);
		initialize_semaphores(m_render_finished_semaphores, images.size());

		m_size = {swapchain->extent.width, swapchain->extent.height};

		for (i32 i = 0; auto& backbuffer : m_backbuffers)
		{
			backbuffer = trx_new VulkanSwapchainTexture(images[i], vk::Format(swapchain->image_format), m_size);
			++i;
		}

		m_swapchain = swapchain->swapchain;
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::release_swapchain()
	{
		VulkanAPI::instance()->idle();

		for (VulkanTexture* backbuffer : m_backbuffers)
		{
			trx_delete backbuffer;
		}

		m_backbuffers.clear();

		if (m_swapchain)
			VulkanAPI::instance()->m_device.destroySwapchainKHR(m_swapchain);

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

		VulkanAPI::instance()->m_device.destroySwapchainKHR(swapchain);

		return *this;
	}

	i32 VulkanSwapchain::acquire_image_index()
	{
		trinex_profile_cpu_n("VulkanSwapchain::acquire_image_index");

		const auto prev_sync_index = m_sync_index;
		m_sync_index               = (m_sync_index + 1) % m_image_present_semaphores.size();

		VulkanSemaphore* semaphore = m_image_present_semaphores[m_sync_index];
		semaphore->is_signaled(true);

		vk::ResultValue<u32> result =
		        VulkanAPI::instance()->m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, semaphore->semaphore());

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
			trinex_unreachable_msg("Failed to acquire image index");
		}

		m_image_index = result.value;
		return m_image_index;
	}

	i32 VulkanSwapchain::do_present()
	{
		if (m_image_index == -1)
			return Status::Success;

		auto image_index = static_cast<u32>(m_image_index);

		VulkanSemaphore* semaphore = present_semaphore();
		vk::PresentInfoKHR info({}, m_swapchain, image_index);

		if (semaphore->is_signaled())
		{
			info.setWaitSemaphores(semaphore->semaphore());
			semaphore->is_signaled(false);
		}

		{
			trinex_profile_cpu_n("VulkanSwapchain::Present KHR");
			vk::Result result = VulkanAPI::instance()->m_graphics_queue->present(info);
			m_image_index     = -1;

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
	}

	i32 VulkanSwapchain::try_present(i32 (VulkanSwapchain::*callback)(), bool skip_on_out_of_date)
	{
		if (m_need_recreate)
		{
			recreate_swapchain();
			return try_present(callback, skip_on_out_of_date);
		}

		i32 status = (this->*callback)();

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
				trinex_unreachable_msg("Failed to present swapchain");
			}

			Thread::static_sleep_for(0.1);
			recreate_swapchain();

			status = (this->*callback)();
		}

		return status;
	}

	VulkanSemaphore* VulkanSwapchain::acquire_semaphore()
	{
		backbuffer();
		return m_image_present_semaphores[m_sync_index].get();
	}

	VulkanSemaphore* VulkanSwapchain::present_semaphore()
	{
		backbuffer();
		return m_render_finished_semaphores[m_image_index].get();
	}

	VulkanTexture* VulkanSwapchain::backbuffer()
	{
		if (m_image_index == -1)
		{
			trinex_profile_cpu_n("VulkanSwapchain::backbuffer");

			if (try_present(&VulkanSwapchain::acquire_image_index, false) < 0)
			{
				trinex_unreachable_msg("Failed to acquire image index");
			}
		}

		return m_backbuffers[m_image_index];
	}

	void VulkanSwapchain::vsync(bool flag)
	{
		auto mode = VulkanAPI::instance()->present_mode_of(flag, m_surface);
		if (mode != m_present_mode)
		{
			m_present_mode  = mode;
			m_need_recreate = true;
		}
	}

	RHIRenderTargetView* VulkanSwapchain::as_rtv()
	{
		return backbuffer()->as_rtv(nullptr);
	}

	RHITexture* VulkanSwapchain::as_texture()
	{
		return backbuffer();
	}

	void VulkanSwapchain::resize(const Vector2u& size)
	{
		m_need_recreate = true;
	}

	void VulkanSwapchain::present()
	{
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
}// namespace Trinex
