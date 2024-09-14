#pragma once
#include <Core/build.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>


namespace Engine
{
	struct VulkanRenderPass {
		struct Key {
			vk::Format m_color_attachments[RHI_MAX_RT_BINDED + 1];
			vk::Format m_depth_stencil;

			void init(vk::Format format);
			void init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderPass*> m_render_passes;

		vk::RenderPass m_render_pass;

		static VulkanRenderPass* find_or_create(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
		static VulkanRenderPass* swapchain_render_pass(vk::Format format);
		static void destroy_all();

	private:
		VulkanRenderPass(vk::RenderPass pass);
		~VulkanRenderPass();
	};
}// namespace Engine
