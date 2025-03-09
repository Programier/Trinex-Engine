#pragma once

#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <vulkan_headers.hpp>

namespace Engine
{
	struct VulkanTexture;

	struct VulkanSurface : public RHI_DefaultDestroyable<RHI_Surface> {
		VulkanTexture* m_texture;

		VulkanSurface& create(ColorFormat format, Vector2u size);
		RHI_RenderTargetView* create_rtv() override;
		RHI_DepthStencilView* create_dsv() override;
		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;

		Vector2u size() const;
		vk::Format format() const;
		vk::ImageLayout layout() const;
		vk::Image image() const;
		void change_layout(vk::ImageLayout layout, vk::CommandBuffer& cmd);

		~VulkanSurface();
	};

	struct VulkanSurfaceRTV : RHI_DefaultDestroyable<RHI_RenderTargetView> {
		Set<struct VulkanRenderTarget*> m_render_targets;
		VulkanSurface* m_surface;
		vk::ImageView m_view;

		VulkanSurfaceRTV(VulkanSurface* surface, vk::ImageView view);
		~VulkanSurfaceRTV();

		void clear(const Color& color) override;
		void blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE Vector2u size() const { return m_surface->size(); }
		FORCE_INLINE vk::Format format() const { return m_surface->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_surface->layout(); }
		FORCE_INLINE vk::Image image() const { return m_surface->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout, vk::CommandBuffer& cmd)
		{
			return m_surface->change_layout(layout, cmd);
		}
	};

	struct VulkanSurfaceDSV : RHI_DefaultDestroyable<RHI_DepthStencilView> {
		Set<struct VulkanRenderTarget*> m_render_targets;

		VulkanSurface* m_surface;
		vk::ImageView m_view;
		bool m_with_stencil;

		VulkanSurfaceDSV(VulkanSurface* surface, vk::ImageView view);
		~VulkanSurfaceDSV();

		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		FORCE_INLINE Vector2u size() const { return m_surface->size(); }
		FORCE_INLINE vk::Format format() const { return m_surface->format(); }
		FORCE_INLINE vk::ImageLayout layout() const { return m_surface->layout(); }
		FORCE_INLINE vk::Image image() const { return m_surface->image(); }
		FORCE_INLINE void change_layout(vk::ImageLayout layout, vk::CommandBuffer& cmd)
		{
			return m_surface->change_layout(layout, cmd);
		}
	};
}// namespace Engine
