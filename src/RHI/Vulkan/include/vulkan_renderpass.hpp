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
		vk::SubpassDescription m_subpass;
		vk::SubpassDependency m_dependency;

		Vector<vk::AttachmentDescription> m_attachment_descriptions;
		Vector<vk::AttachmentReference> m_color_attachment_references;
		vk::AttachmentReference m_depth_attachment_renference;
		bool m_has_depth_attachment = false;


		static VulkanRenderPass* find_or_create(const Span<RenderSurface*>& color_attachments,
												RenderSurface* depth_stencil);
		static VulkanRenderPass* swapchain_render_pass(vk::Format format);
		static void destroy_all();

		VulkanRenderPass& init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);

		VulkanRenderPass& create_attachment_descriptions(const Span<RenderSurface*>& color_attachments,
														 RenderSurface* depth_stencil);
		VulkanRenderPass& create_attachment_references(const Span<RenderSurface*>& color_attachments,
													   RenderSurface* depth_stencil);

		VulkanRenderPass& create();
		uint_t attachments_count() const;
		uint_t color_attachments_count() const;

		VulkanRenderPass& destroy();
		~VulkanRenderPass();
	};
}// namespace Engine
