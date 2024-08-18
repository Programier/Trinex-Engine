#pragma once
#include <Core/build.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanRenderTargetState {
		struct VulkanRenderPass* m_render_pass = nullptr;
		vk::RenderPassBeginInfo m_render_pass_info;
		Size2D m_size;

		void init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
		void post_init();

		virtual bool is_main_render_target_state();
	};

	struct VulkanMainRenderTargetState : public VulkanRenderTargetState {
		struct VulkanWindowViewport* m_viewport = nullptr;
		bool is_main_render_target_state() override;
	};


	struct VulkanRenderTargetBase {
		vk::Framebuffer m_framebuffer;

		bool prepare_bind();

		virtual bool is_main_render_target();
		virtual VulkanRenderTargetBase& destroy();
		virtual VulkanRenderTargetState* state();

		VulkanRenderTargetBase& post_init(const Vector<vk::ImageView>& image_views);
		VulkanRenderTargetBase& size(uint32_t width, uint32_t height);

		virtual void bind();
		virtual VulkanRenderTargetBase& unbind();
		virtual ~VulkanRenderTargetBase();
	};


	struct VulkanWindowRenderTargetFrame : VulkanRenderTargetBase {
		VulkanMainRenderTargetState* m_state;
		bool is_main_render_target() override;
		VulkanMainRenderTargetState* state() override;

		void bind() override;
		VulkanRenderTargetBase& unbind() override;
	};

	struct VulkanRenderTarget : VulkanRenderTargetBase {
		struct Key {
			struct VulkanSurface* m_color_attachments[RHI_MAX_RT_BINDED];
			struct VulkanSurface* m_depth_stencil;

			void init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
			bool operator<(const Key& key) const;
		};

		static TreeMap<Key, VulkanRenderTarget*> m_render_targets;
		Vector<struct VulkanSurface*> m_surfaces;

		VulkanRenderTargetState m_state;
		Vector<vk::ImageView> m_attachments;

		static VulkanRenderTarget* find_or_create(const Span<RenderSurface*>& color_attachments,
												  RenderSurface* depth_stencil);

		VulkanRenderTarget();
		VulkanRenderTarget& init(const Span<RenderSurface*>& color_attachments, RenderSurface* depth_stencil);
		VulkanRenderTarget& destroy() override;
		VulkanRenderTargetState* state() override;

		void bind() override;
		VulkanRenderTargetBase& unbind() override;

		~VulkanRenderTarget();
	};


	struct VulkanWindowRenderTarget {
		VulkanMainRenderTargetState state;
		Vector<VulkanWindowRenderTargetFrame*> m_frames;

		VulkanWindowRenderTarget& init(struct VulkanWindowViewport* viewport);
		VulkanWindowRenderTarget& destroy();

		void resize_count(size_t new_count);
		VulkanWindowRenderTargetFrame* frame();

		void bind();
		~VulkanWindowRenderTarget();
	};
}// namespace Engine
