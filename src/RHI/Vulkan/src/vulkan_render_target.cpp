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
	VulkanRenderTargetBase& VulkanRenderTargetBase::post_init(const Vector<vk::ImageView>& image_views)
	{
		vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_render_pass->m_render_pass,
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
	}

	bool VulkanRenderTargetBase::is_main_render_target()
	{
		return false;
	}

	VulkanRenderTargetBase& VulkanRenderTargetBase::size(uint32_t width, uint32_t height)
	{
		m_size.x = static_cast<float>(width);
		m_size.y = static_cast<float>(height);
		return *this;
	}

	VulkanRenderTargetBase::~VulkanRenderTargetBase()
	{
		DESTROY_CALL(destroyFramebuffer, m_framebuffer);
	}

	VulkanRenderTarget::VulkanRenderTarget()
	{}

	TreeMap<VulkanRenderTarget::Key, VulkanRenderTarget*> VulkanRenderTarget::m_render_targets;

	void VulkanRenderTarget::Key::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		size_t index = 0;

		for (size_t count = color_attachments.size(); index < count; ++index)
		{
			m_color_attachments[index] = color_attachments[index]->rhi_object<VulkanSurface>();
		}

		for (; index < RHI_MAX_RT_BINDED; ++index)
		{
			m_color_attachments[index] = nullptr;
		}

		m_depth_stencil = depth_stencil ? depth_stencil->rhi_object<VulkanSurface>() : nullptr;
	}

	void VulkanRenderTarget::Key::init(const Span<VulkanSurface*>& attachments)
	{
		size_t index = 0;

		for (auto surface : attachments)
		{
			if (surface->is_render_target_color_image())
			{
				m_color_attachments[index++] = surface;
			}
			else if (surface->is_depth_stencil_image())
			{
				m_depth_stencil = surface;
			}
		}

		for (; index < RHI_MAX_RT_BINDED; ++index)
		{
			m_color_attachments[index] = nullptr;
		}
	}

	bool VulkanRenderTarget::Key::operator<(const Key& key) const
	{
		return std::memcmp(this, &key, sizeof(*this)) < 0;
	}

	VulkanRenderTarget* VulkanRenderTarget::find_or_create(const Span<RenderSurface*>& color_attachments,
	                                                       RenderSurface* depth_stencil)
	{
		trinex_profile_cpu();
		Key key;
		key.init(color_attachments, depth_stencil);
		VulkanRenderTarget*& render_target = m_render_targets[key];

		if (render_target != nullptr)
			return render_target;

		if (API->m_properties.limits.maxColorAttachments < static_cast<uint32_t>(color_attachments.size()))
		{
			throw EngineException(Strings::format("Max color attachments is {}", API->m_properties.limits.maxColorAttachments));
		}
		render_target = new VulkanRenderTarget();
		render_target->init(color_attachments, depth_stencil);

		return render_target;
	}

	VulkanRenderTarget& VulkanRenderTarget::init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		m_render_pass               = VulkanRenderPass::find_or_create(color_attachments, depth_stencil);
		RenderSurface* base_surface = color_attachments.empty() ? depth_stencil : color_attachments[0];

		if (base_surface == nullptr)
		{
			throw EngineException("Vulkan: Cannot initialize vulkan render target! No targets found!");
		}

		m_size = base_surface->rhi_object<VulkanSurface>()->size();


		m_attachments.resize(color_attachments.size() + (depth_stencil ? 1 : 0));
		m_surfaces.resize(m_attachments.size());

		Index index = 0;
		for (auto& attachment : color_attachments)
		{
			const Texture2D* color_binding = attachment;
			VulkanSurface* texture         = color_binding->rhi_object<VulkanSurface>();

			trinex_check(texture, "Vulkan API: Cannot attach color texture: Texture is NULL");
			bool usage_check = texture->is_render_target_color_image();
			trinex_check(usage_check, "Vulkan API: Pixel type for color attachment must be RGBA");

			vk::ImageSubresourceRange range(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);
			m_attachments[index] = texture->create_image_view(range);
			m_surfaces[index]    = texture;
			texture->m_render_targets.insert(this);
			++index;
		}

		if (depth_stencil)
		{
			const Texture2D* binding = depth_stencil;
			VulkanSurface* texture   = binding->rhi_object<VulkanSurface>();
			trinex_check(texture, "Vulkan API: Cannot depth attach texture: Texture is NULL");

			bool check_status = texture->is_depth_stencil_image();
			trinex_check(check_status, "Vulkan API: Pixel type for depth attachment must be Depth* or Stencil*");

			vk::ImageSubresourceRange range(texture->aspect(), 0, 1, 0, 1);
			m_attachments[index] = texture->create_image_view(range);
			m_surfaces[index]    = texture;

			texture->m_render_targets.insert(this);
		}

		post_init(m_attachments);
		return *this;
	}

	VulkanRenderTargetBase& VulkanRenderTarget::lock_surfaces()
	{
		auto& cmd = API->current_command_buffer_handle();
		for (auto& surface : m_surfaces)
		{
			if (surface->is_depth_stencil_image())
			{
				if (is_in<ColorFormat::DepthStencil>(surface->engine_format()))
				{
					surface->change_layout(vk::ImageLayout::eDepthStencilAttachmentOptimal, cmd);
				}
				else
				{
					surface->change_layout(vk::ImageLayout::eDepthAttachmentOptimal, cmd);
				}
			}
			else
			{
				surface->change_layout(vk::ImageLayout::eColorAttachmentOptimal, cmd);
			}
		}
		return *this;
	}

	VulkanRenderTargetBase& VulkanRenderTarget::unlock_surfaces()
	{
		auto& cmd = API->current_command_buffer_handle();
		for (auto& surface : m_surfaces)
		{
			surface->change_layout(vk::ImageLayout::eShaderReadOnlyOptimal, cmd);
		}

		return *this;
	}

	size_t VulkanRenderTarget::color_attachments_count() const
	{
		return m_surfaces.size() - depth_stencil_attachments_count();
	}

	size_t VulkanRenderTarget::depth_stencil_attachments_count() const
	{
		if (m_surfaces.empty())
			return 0;

		return m_surfaces.back()->is_depth_stencil_image() ? 1 : 0;
	}

	VulkanRenderTarget::~VulkanRenderTarget()
	{
		Key key;
		key.init(m_surfaces);
		m_render_targets.erase(key);

		for (auto& image_view : m_attachments)
		{
			DESTROY_CALL(destroyImageView, image_view);
		}

		for (VulkanSurface* surface : m_surfaces)
		{
			surface->m_render_targets.erase(this);
		}
	}

	VulkanSwapchainRenderTarget::VulkanSwapchainRenderTarget(vk::Image image, vk::ImageView view, Size2D size, vk::Format format)
	{
		m_image = image;
		m_view  = view;

		m_size        = size;
		m_render_pass = VulkanRenderPass::swapchain_render_pass(format);
		post_init({view});
	}

	bool VulkanSwapchainRenderTarget::is_main_render_target()
	{
		return true;
	}

	VulkanSwapchainRenderTarget& VulkanSwapchainRenderTarget::lock_surfaces()
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout           = vk::ImageLayout::ePresentSrcKHR;
		barrier.newLayout           = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.image               = m_image;
		barrier.subresourceRange    = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		Barrier::transition_image_layout(API->current_command_buffer_handle(), barrier);
		return *this;
	}

	VulkanSwapchainRenderTarget& VulkanSwapchainRenderTarget::unlock_surfaces()
	{
		vk::ImageMemoryBarrier barrier;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.oldLayout           = vk::ImageLayout::eColorAttachmentOptimal;
		barrier.newLayout           = vk::ImageLayout::ePresentSrcKHR;
		barrier.image               = m_image;
		barrier.subresourceRange    = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

		Barrier::transition_image_layout(API->current_command_buffer_handle(), barrier);
		return *this;
	}

	size_t VulkanSwapchainRenderTarget::color_attachments_count() const
	{
		return 1;
	}

	size_t VulkanSwapchainRenderTarget::depth_stencil_attachments_count() const
	{
		return 0;
	}

	VulkanSwapchainRenderTarget::~VulkanSwapchainRenderTarget()
	{
		DESTROY_CALL(destroyImageView, m_view);
	}

	VulkanAPI& VulkanAPI::bind_render_target(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil)
	{
		VulkanRenderTarget* rt = VulkanRenderTarget::find_or_create(color_attachments, depth_stencil);
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
				float vp_height = viewport.size.y;
				float vp_y      = viewport.pos.y;

				if (new_mode == VulkanViewportMode::Flipped)
				{
					vp_height               = -vp_height;
					auto render_target_size = m_state.m_current_viewport->render_target()->m_size;
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
				const auto& render_target_size = m_state.m_render_target->m_size;
				float sc_y                     = scissor.pos.y;

				if (new_mode == VulkanViewportMode::Flipped)
				{
					sc_y = render_target_size.y - sc_y - scissor.size.y;
				}

				vk::Rect2D vulkan_scissor;
				vulkan_scissor.offset.setX(glm::clamp(scissor.pos.x, 0.f, render_target_size.x));
				vulkan_scissor.offset.setY(glm::clamp(sc_y, 0.f, render_target_size.y));
				vulkan_scissor.extent.setWidth(glm::clamp(scissor.size.x, 0.f, render_target_size.x - vulkan_scissor.offset.x));
				vulkan_scissor.extent.setHeight(glm::clamp(scissor.size.y, 0.f, render_target_size.y - vulkan_scissor.offset.y));
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
