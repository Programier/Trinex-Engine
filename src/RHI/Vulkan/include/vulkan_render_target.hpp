#pragma once
#include <Core/build.hpp>
#include <Core/etl/map.hpp>
#include <Core/etl/vector.hpp>
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
		virtual bool is_swapchain_render_target();
		virtual VulkanRenderTargetBase& lock_surfaces()   = 0;
		virtual VulkanRenderTargetBase& unlock_surfaces() = 0;

		virtual size_t color_attachments_count() const         = 0;
		virtual size_t depth_stencil_attachments_count() const = 0;
		virtual ~VulkanRenderTargetBase();
	};

	struct VulkanRenderTarget : VulkanRenderTargetBase {
		struct Key {
			struct VulkanSurface* m_surfaces[5] = {};

			void init(const RenderSurface* targets[5]);
			void init(VulkanSurface* targets[5]);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderTarget*> m_render_targets;
		Key m_key;
		Vector<vk::ImageView> m_attachments;

		static VulkanRenderTarget* find_or_create(const RenderSurface** targets);

		VulkanRenderTarget();
		VulkanRenderTarget& init(const RenderSurface** targets);

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

		virtual bool is_swapchain_render_target() override;
		VulkanSwapchainRenderTarget& lock_surfaces() override;
		VulkanSwapchainRenderTarget& unlock_surfaces() override;

		size_t color_attachments_count() const override;
		size_t depth_stencil_attachments_count() const override;
		~VulkanSwapchainRenderTarget();
	};
}// namespace Engine
