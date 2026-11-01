#include <Core/etl/algorithm.hpp>
#include <Core/memory.hpp>
#include <Core/profiler.hpp>
#include <Core/window.hpp>
#include <Graphics/render_pools.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
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
			m_flags        = RHITextureFlags::RenderTarget;
			m_image        = image;
			m_access       = RHIAccess::Undefined;
			m_format       = format;
			m_extent       = vk::Extent3D(size.x, size.y, 1);
			m_mips_count   = 1;
			m_layers_count = 1;
		}

		~VulkanSwapchainTexture() { m_image = vk::Image(); }
	};

	static void initialize_semaphores(Array<RHIResourcePtr<VulkanSemaphore>>& semaphores, usize size)
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

	VulkanSwapchain::VulkanSwapchain(Window* window, RHIPresentMode mode, u32 images)
	    : m_surface(VulkanAPI::instance()->create_surface(window)), m_window(window), m_desired_image_count(images)
	{
		m_present_mode = VulkanAPI::instance()->present_mode_of(mode, m_surface);
		create_swapchain();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
		release_swapchain();
		VulkanAPI::instance()->m_instance.destroySurfaceKHR(m_surface);
	}

	VulkanSwapchain& VulkanSwapchain::create_swapchain(vk::SwapchainKHR* old)
	{
		trinex_info(Log::RHI, "Creating new swapchain");
		m_need_recreate = false;

		VulkanAPI* api    = VulkanAPI::instance();
		auto capabilities = vk::verify(api->m_physical_device.getSurfaceCapabilitiesKHR(m_surface));
		auto formats      = vk::verify(api->m_physical_device.getSurfaceFormatsKHR(m_surface));
		trinex_verify_msg(!formats.empty(), "Failed to find a surface format");

		vk::SurfaceFormatKHR surface_format = formats.front();
		for (const vk::SurfaceFormatKHR& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				surface_format = format;
				break;
			}
		}

		vk::Extent2D extent = capabilities.currentExtent;
		if (extent.width == UINT32_MAX)
		{
			const Vector2u size = m_window->size();
			extent.width        = etl::clamp(size.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height       = etl::clamp(size.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		u32 image_count = m_desired_image_count ? m_desired_image_count : capabilities.minImageCount + 1;
		image_count     = etl::max(image_count, capabilities.minImageCount);
		if (capabilities.maxImageCount)
			image_count = etl::min(image_count, capabilities.maxImageCount);

		vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment;
		if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferSrc)
			usage |= vk::ImageUsageFlagBits::eTransferSrc;
		if (capabilities.supportedUsageFlags & vk::ImageUsageFlagBits::eTransferDst)
			usage |= vk::ImageUsageFlagBits::eTransferDst;

		vk::CompositeAlphaFlagBitsKHR composite_alpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
		if (!(capabilities.supportedCompositeAlpha & composite_alpha))
		{
			constexpr vk::CompositeAlphaFlagBitsKHR modes[] = {
			        vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
			        vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
			        vk::CompositeAlphaFlagBitsKHR::eInherit,
			};

			for (vk::CompositeAlphaFlagBitsKHR mode : modes)
			{
				if (capabilities.supportedCompositeAlpha & mode)
				{
					composite_alpha = mode;
					break;
				}
			}
		}

		vk::SwapchainCreateInfoKHR info;
		info.setSurface(m_surface)
		        .setMinImageCount(image_count)
		        .setImageFormat(surface_format.format)
		        .setImageColorSpace(surface_format.colorSpace)
		        .setImageExtent(extent)
		        .setImageArrayLayers(1)
		        .setImageUsage(usage)
		        .setImageSharingMode(vk::SharingMode::eExclusive)
		        .setPreTransform((capabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
		                                 ? vk::SurfaceTransformFlagBitsKHR::eIdentity
		                                 : capabilities.currentTransform)
		        .setCompositeAlpha(composite_alpha)
		        .setPresentMode(m_present_mode)
		        .setClipped(true)
		        .setOldSwapchain(old ? *old : VK_NULL_HANDLE);

		m_swapchain = vk::verify(VulkanAPI::device()->createSwapchainKHR(info));
		auto images = vk::verify(VulkanAPI::device()->getSwapchainImagesKHR(m_swapchain));

		m_backbuffers.resize(images.size());
		initialize_semaphores(m_image_present_semaphores, images.size() + 1);
		initialize_semaphores(m_render_finished_semaphores, images.size());

		m_size = {extent.width, extent.height};

		for (i32 i = 0; auto& backbuffer : m_backbuffers)
		{
			backbuffer = trx_new VulkanSwapchainTexture(images[i], surface_format.format, m_size);
			++i;
		}
		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::release_swapchain()
	{
		VulkanAPI::instance()->wait();

		for (VulkanTexture* backbuffer : m_backbuffers)
		{
			trx_delete backbuffer;
		}

		m_backbuffers.clear();

		if (m_swapchain)
			VulkanAPI::device()->destroySwapchainKHR(m_swapchain);

		m_swapchain   = VK_NULL_HANDLE;
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

		VulkanAPI::device()->destroySwapchainKHR(swapchain);

		return *this;
	}

	VulkanSwapchain& VulkanSwapchain::recreate_surface()
	{
		release_swapchain();
		VulkanAPI::instance()->m_instance.destroySurfaceKHR(m_surface);
		m_surface = VulkanAPI::instance()->create_surface(m_window);
		return create_swapchain();
	}

	i32 VulkanSwapchain::acquire_image_index()
	{
		if (m_image_index >= 0)
			return m_image_index;

		trinex_profile_cpu_n("VulkanSwapchain::acquire_image_index");
		trinex_assert(m_image_index == -1);

		do
		{
			if (m_need_recreate)
			{
				recreate_swapchain();
			}

			m_sync_index = (m_sync_index + 1) % m_image_present_semaphores.size();

			VulkanSemaphore* semaphore = m_image_present_semaphores[m_sync_index];
			trinex_assert(!semaphore->is_signaled());

			vk::ResultValue<u32> result =
			        VulkanAPI::device()->acquireNextImageKHR(m_swapchain, UINT64_MAX, semaphore->semaphore());

			if (result.result == vk::Result::eErrorOutOfDateKHR)
			{
				recreate_swapchain();
				continue;
			}

			if (result.result == vk::Result::eErrorSurfaceLostKHR)
			{
				trinex_warning(Log::RHI, "Swapchain surface lost");
				recreate_surface();
				continue;
			}

			if (result.result == vk::Result::eErrorValidationFailedEXT)
			{
				trinex_error(Log::RHI, "vkAcquireNextImageKHR failed with validation error");
			}
			else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
			{
				trinex_unreachable_msg("Failed to acquire image index");
			}

			semaphore->is_signaled(true);
			m_image_index = result.value;
			m_need_recreate |= result.result == vk::Result::eSuboptimalKHR;
			return m_image_index;
		} while (true);
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

	void VulkanSwapchain::mode(RHIPresentMode mode)
	{
		vk::PresentModeKHR present_mode = VulkanAPI::instance()->present_mode_of(mode, m_surface);

		if (present_mode != m_present_mode)
		{
			m_present_mode  = present_mode;
			m_need_recreate = true;
		}
	}

	void VulkanSwapchain::resize(Vector2u size)
	{
		m_need_recreate = true;
	}

	RHITexture* VulkanSwapchain::backbuffer()
	{
		const i32 index = acquire_image_index();
		trinex_verify_msg(index >= 0, "Failed to acquire image index");
		return m_backbuffers[index];
	}

	vk::Result VulkanSwapchain::present(vk::Queue* queue)
	{
		if (m_image_index == -1)
			return vk::Result::eSuccess;

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
			vk::Result result = queue->presentKHR(info);
			m_image_index     = -1;

			if (result == vk::Result::eErrorOutOfDateKHR)
			{
				m_need_recreate = true;
				return result;
			}

			if (result == vk::Result::eErrorSurfaceLostKHR)
			{
				return result;
			}

			if (result == vk::Result::eSuboptimalKHR)
				m_need_recreate = true;
			else if (result != vk::Result::eSuccess)
				trinex_unreachable_msg("Failed to present swapchain");

			return vk::Result::eSuccess;
		}
	}

	void VulkanSwapchain::destroy()
	{
		trx_delete this;
	}

	RHISwapchain* VulkanAPI::create_swapchain(Window* window, RHIPresentMode mode, u32 images)
	{
		return trx_new VulkanSwapchain(window, mode, images);
	}
} // namespace Trinex
