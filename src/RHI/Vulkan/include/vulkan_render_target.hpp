#pragma once
#include <Core/build.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanRenderTargetBase {
		vk::Framebuffer m_framebuffer;
		struct VulkanRenderPass* m_render_pass = nullptr;
		Size2D m_size;

		VulkanRenderTargetBase& post_init(const Vector<vk::ImageView>& image_views);
		VulkanRenderTargetBase& size(uint32_t width, uint32_t height);

		void bind();
		virtual bool is_main_render_target();
		virtual VulkanRenderTargetBase& lock_surfaces()   = 0;
		virtual VulkanRenderTargetBase& unlock_surfaces() = 0;

		virtual size_t color_attachments_count() const         = 0;
		virtual size_t depth_stencil_attachments_count() const = 0;
		virtual ~VulkanRenderTargetBase();
	};

	struct VulkanRenderTarget : VulkanRenderTargetBase {
		struct Key {
			struct VulkanSurface* m_color_attachments[RHI_MAX_RT_BINDED] = {};
			struct VulkanSurface* m_depth_stencil = nullptr;

			void init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
			void init(const Span<VulkanSurface*>& attachments);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderTarget*> m_render_targets;
		Vector<struct VulkanSurface*> m_surfaces;
		Vector<vk::ImageView> m_attachments;

		static VulkanRenderTarget* find_or_create(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);

		VulkanRenderTarget();
		VulkanRenderTarget& init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);

		VulkanRenderTargetBase& lock_surfaces() override;
		VulkanRenderTargetBase& unlock_surfaces() override;
		size_t color_attachments_count() const override;
		size_t depth_stencil_attachments_count() const override;

		~VulkanRenderTarget();
	};

	struct VulkanSwapchainRenderTarget : public VulkanRenderTargetBase {
		vk::Image m_image;
		vk::ImageView m_view;

		VulkanSwapchainRenderTarget(vk::Image image, vk::ImageView view, Size2D size, vk::Format format);

		virtual bool is_main_render_target() override;
		VulkanSwapchainRenderTarget& lock_surfaces() override;
		VulkanSwapchainRenderTarget& unlock_surfaces() override;

		size_t color_attachments_count() const override;
		size_t depth_stencil_attachments_count() const override;
		~VulkanSwapchainRenderTarget();
	};
}// namespace Engine
