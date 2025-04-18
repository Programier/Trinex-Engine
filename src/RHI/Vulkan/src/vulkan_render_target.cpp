#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/profiler.hpp>
#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_texture.hpp>
#include <vulkan_viewport.hpp>

namespace Engine
{
	VulkanRenderTargetBase& VulkanRenderTargetBase::post_init(vk::ImageView* image_views, uint32_t count)
	{
		vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_render_pass->m_render_pass, count,
		                                                  image_views, m_size.x, m_size.y, 1);
		m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);

		return *this;
	}

	void VulkanRenderTargetBase::bind()
	{
		if (API->m_state.m_render_target != this)
		{
			API->m_state.m_next_render_target = this;
		}
		else
		{
			API->m_state.m_next_render_target = nullptr;
		}
	}

	bool VulkanRenderTargetBase::is_swapchain_render_target()
	{
		return false;
	}

	VulkanRenderTargetBase& VulkanRenderTargetBase::size(uint32_t width, uint32_t height)
	{
		m_size.x = width;
		m_size.y = height;
		return *this;
	}

	VulkanRenderTargetBase::~VulkanRenderTargetBase()
	{
		DESTROY_CALL(destroyFramebuffer, m_framebuffer);
	}

	VulkanRenderTarget::VulkanRenderTarget() {}

	TreeMap<VulkanRenderTarget::Key, VulkanRenderTarget*> VulkanRenderTarget::m_render_targets;

	void VulkanRenderTarget::Key::init(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		m_surfaces[0] = targets[0];
		m_surfaces[1] = targets[1];
		m_surfaces[2] = targets[2];
		m_surfaces[3] = targets[3];
		m_depth       = depth;
	}

	bool VulkanRenderTarget::Key::operator<(const Key& key) const
	{
		return std::memcmp(this, &key, sizeof(Key)) < 0;
	}

	VulkanRenderTarget* VulkanRenderTarget::find_or_create(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		trinex_profile_cpu_n("VulkanRenderTarget::find_or_create");
		Key key;
		key.init(targets, depth);
		VulkanRenderTarget*& render_target = m_render_targets[key];

		if (render_target != nullptr)
			return render_target;

		render_target = new VulkanRenderTarget();
		render_target->init(targets, depth);

		return render_target;
	}

	VulkanRenderTarget& VulkanRenderTarget::init(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		m_key.init(targets, depth);
		m_render_pass                  = VulkanRenderPass::find_or_create(targets, depth);
		VulkanTextureRTV* base_surface = targets[0] ? targets[0] : targets[4];

		if (base_surface == nullptr)
		{
			throw EngineException("Vulkan: Cannot initialize vulkan render target! No targets found!");
		}

		auto extent = base_surface->extent();
		m_size      = {extent.width, extent.height};
		Index index = 0;

		vk::ImageView attachments[5] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};

		for (int i = 0; i < 4; ++i)
		{
			if (targets[i] == nullptr)
				continue;

			VulkanTextureRTV* rtv = targets[i];
			attachments[index++]  = rtv->m_view;
			rtv->m_render_targets.insert(this);
		}

		if (depth)
		{
			attachments[index++] = depth->m_view;
			depth->m_render_targets.insert(this);
		}

		post_init(attachments, index);
		return *this;
	}

	VulkanRenderTargetBase& VulkanRenderTarget::lock_surfaces()
	{
		for (auto& surface : m_key.m_surfaces)
		{
			if (surface == nullptr)
				continue;

			surface->change_layout(vk::ImageLayout::eColorAttachmentOptimal);
		}

		if (m_key.m_depth)
		{
			m_key.m_depth->change_layout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
		return *this;
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		m_render_targets.erase(m_key);

		for (VulkanTextureRTV* surface : m_key.m_surfaces)
		{
			if (surface)
				surface->m_render_targets.erase(this);
		}

		if (m_key.m_depth)
		{
			m_key.m_depth->m_render_targets.erase(this);
		}
	}

	VulkanSwapchainRenderTarget::VulkanSwapchainRenderTarget(vk::Image image, vk::ImageView view, Size2D size, vk::Format format)
	{
		m_image  = image;
		m_view   = view;
		m_layout = vk::ImageLayout::eUndefined;

		m_size        = size;
		m_render_pass = VulkanRenderPass::swapchain_render_pass(format);
		post_init(&view, 1);

		change_layout(vk::ImageLayout::ePresentSrcKHR);
	}

	bool VulkanSwapchainRenderTarget::is_swapchain_render_target()
	{
		return true;
	}

	VulkanSwapchainRenderTarget& VulkanSwapchainRenderTarget::lock_surfaces()
	{
		change_layout(vk::ImageLayout::eColorAttachmentOptimal);
		return *this;
	}

	VulkanSwapchainRenderTarget& VulkanSwapchainRenderTarget::change_layout(vk::ImageLayout new_layout)
	{
		if (m_layout != new_layout)
		{
			vk::ImageMemoryBarrier barrier;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.oldLayout           = m_layout;
			barrier.newLayout           = new_layout;
			barrier.image               = m_image;
			barrier.subresourceRange    = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

			Barrier::transition_image_layout(barrier);
			m_layout = new_layout;
		}
		return *this;
	}

	VulkanSwapchainRenderTarget::~VulkanSwapchainRenderTarget()
	{
		DESTROY_CALL(destroyImageView, m_view);
	}

	VulkanAPI& VulkanAPI::bind_render_target(RHI_RenderTargetView* rt1, RHI_RenderTargetView* rt2, RHI_RenderTargetView* rt3,
	                                         RHI_RenderTargetView* rt4, RHI_DepthStencilView* depth_stencil)
	{
		VulkanTextureRTV* surfaces[4] = {static_cast<VulkanTextureRTV*>(rt1), static_cast<VulkanTextureRTV*>(rt2),
		                                 static_cast<VulkanTextureRTV*>(rt3), static_cast<VulkanTextureRTV*>(rt4)};

		VulkanRenderTarget* rt = VulkanRenderTarget::find_or_create(surfaces, static_cast<VulkanTextureDSV*>(depth_stencil));
		rt->bind();
		return *this;
	}

	VulkanAPI& VulkanAPI::viewport(const ViewPort& viewport)
	{
		auto& m_viewport = m_state.m_viewport;
		auto new_mode    = find_current_viewport_mode();

		if (new_mode != m_state.m_viewport_mode || m_viewport != viewport)
		{
			if (new_mode != VulkanViewportMode::Undefined)
			{
				int_t vp_height = viewport.size.y;
				int_t vp_y      = viewport.pos.y;

				if (new_mode == VulkanViewportMode::Flipped)
				{
					vp_height               = -vp_height;
					auto render_target_size = m_state.render_target()->m_size;
					vp_y                    = render_target_size.y - vp_y;
				}

				{
					vk::Viewport vulkan_viewport;
					vulkan_viewport.setWidth(viewport.size.x);
					vulkan_viewport.setHeight(vp_height);
					vulkan_viewport.setX(viewport.pos.x);
					vulkan_viewport.setY(vp_y);
					vulkan_viewport.setMinDepth(viewport.min_depth);
					vulkan_viewport.setMaxDepth(viewport.max_depth);
					current_command_buffer_handle().setViewport(0, vulkan_viewport);
				}
			}

			m_viewport = viewport;
		}
		return *this;
	}

	ViewPort VulkanAPI::viewport()
	{
		return m_state.m_viewport;
	}

	VulkanAPI& VulkanAPI::scissor(const Scissor& scissor)
	{
		auto& m_scissor = m_state.m_scissor;
		auto new_mode   = find_current_viewport_mode();

		if (new_mode != m_state.m_viewport_mode || m_scissor != scissor)
		{
			if (new_mode != VulkanViewportMode::Undefined)
			{
				const auto& render_target_size = m_state.render_target()->m_size;
				int_t sc_y                     = scissor.pos.y;

				if (new_mode == VulkanViewportMode::Flipped)
				{
					sc_y = render_target_size.y - sc_y - scissor.size.y;
				}

				vk::Rect2D vulkan_scissor;

				constexpr auto& clamp = glm::clamp<int_t>;
				vulkan_scissor.offset.setX(clamp(scissor.pos.x, 0, render_target_size.x));
				vulkan_scissor.offset.setY(clamp(sc_y, 0, render_target_size.y));
				vulkan_scissor.extent.setWidth(clamp(scissor.size.x, 0, render_target_size.x - vulkan_scissor.offset.x));
				vulkan_scissor.extent.setHeight(clamp(scissor.size.y, 0, render_target_size.y - vulkan_scissor.offset.y));
				current_command_buffer_handle().setScissor(0, vulkan_scissor);
			}

			m_scissor = scissor;
		}
		return *this;
	}

	Scissor VulkanAPI::scissor()
	{
		return m_state.m_scissor;
	}
}// namespace Engine
