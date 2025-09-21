#include <Core/etl/templates.hpp>
#include <Core/exception.hpp>
#include <Core/profiler.hpp>
#include <Graphics/render_surface.hpp>
#include <vulkan_api.hpp>
#include <vulkan_barriers.hpp>
#include <vulkan_context.hpp>
#include <vulkan_render_target.hpp>
#include <vulkan_renderpass.hpp>
#include <vulkan_resource_view.hpp>
#include <vulkan_state.hpp>

namespace Engine
{
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

		render_target = trx_new VulkanRenderTarget();
		render_target->init(targets, depth);

		return render_target;
	}

	static vk::Extent3D find_target_extent(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		for (int i = 0; i < 4; ++i)
		{
			if (targets[i])
				return targets[i]->extent();
		}

		if (depth)
			return depth->extent();

		throw EngineException("Vulkan: Cannot initialize vulkan render target! No targets found!");
	}

	VulkanRenderTarget& VulkanRenderTarget::init(VulkanTextureRTV** targets, VulkanTextureDSV* depth)
	{
		m_key.init(targets, depth);
		m_render_pass = VulkanRenderPass::find_or_create(targets, depth);

		auto extent = find_target_extent(targets, depth);
		m_width     = extent.width;
		m_height    = extent.height;
		Index index = 0;

		vk::ImageView attachments[5] = {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE};

		for (int i = 0; i < 4; ++i)
		{
			if (targets[i] == nullptr)
				continue;

			VulkanTextureRTV* rtv = targets[i];
			attachments[index++]  = rtv->view();
			rtv->add_target(this);
		}

		if (depth)
		{
			attachments[index++] = depth->view();
			depth->add_target(this);
		}

		vk::FramebufferCreateInfo framebuffer_create_info(vk::FramebufferCreateFlagBits(), m_render_pass->render_pass(), index,
		                                                  attachments, extent.width, extent.height, 1);
		m_framebuffer = API->m_device.createFramebuffer(framebuffer_create_info);
		return *this;
	}

	VulkanRenderTarget& VulkanRenderTarget::lock_surfaces()
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
				surface->remove_target(this);
		}

		if (m_key.m_depth)
		{
			m_key.m_depth->remove_target(this);
		}

		DESTROY_CALL(destroyFramebuffer, m_framebuffer);
	}

	VulkanAPI& VulkanAPI::bind_render_target(RHIRenderTargetView* rt1, RHIRenderTargetView* rt2, RHIRenderTargetView* rt3,
	                                         RHIRenderTargetView* rt4, RHIDepthStencilView* depth_stencil)
	{
		VulkanTextureRTV* surfaces[4] = {static_cast<VulkanTextureRTV*>(rt1), static_cast<VulkanTextureRTV*>(rt2),
		                                 static_cast<VulkanTextureRTV*>(rt3), static_cast<VulkanTextureRTV*>(rt4)};

		VulkanRenderTarget* rt = VulkanRenderTarget::find_or_create(surfaces, static_cast<VulkanTextureDSV*>(depth_stencil));

		m_state_manager->bind(rt);
		return *this;
	}
}// namespace Engine
