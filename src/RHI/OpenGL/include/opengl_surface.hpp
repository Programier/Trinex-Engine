#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_headers.hpp>
#include <opengl_resource.hpp>

namespace Engine
{
	struct OpenGL_Surface : public RHI_DefaultDestroyable<RHI_Surface> {
		GLuint m_id     = 0;
		GLuint m_format = 0;

		OpenGL_Surface& init(ColorFormat format, Vector2u size);
		RHI_RenderTargetView* create_rtv() override;
		RHI_DepthStencilView* create_dsv() override;
		RHI_ShaderResourceView* create_srv() override;
		RHI_UnorderedAccessView* create_uav() override;
		~OpenGL_Surface();
	};

	struct OpenGL_SurfaceSRV : OpenGL_SRV {
		OpenGL_Surface* const m_surface;

		OpenGL_SurfaceSRV(OpenGL_Surface* surface);
		void bind(BindLocation location) override;
		void bind_combined(byte location, struct RHI_Sampler* sampler) override;
		~OpenGL_SurfaceSRV();
	};

	struct OpenGL_SurfaceUAV : public RHI_DefaultDestroyable<RHI_UnorderedAccessView> {
		OpenGL_Surface* const m_surface;

		OpenGL_SurfaceUAV(OpenGL_Surface* surface);
		void bind(BindLocation location) override;
		~OpenGL_SurfaceUAV();
	};

	struct OpenGL_SurfaceRTV : public RHI_DefaultDestroyable<RHI_RenderTargetView> {
		mutable Set<struct OpenGL_RenderTarget*> m_render_targets;
		OpenGL_Surface* const m_surface;

		OpenGL_SurfaceRTV(OpenGL_Surface* surface);
		void clear(const Color& color) override;
		void blit(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~OpenGL_SurfaceRTV();
	};

	struct OpenGL_SurfaceDSV : public RHI_DefaultDestroyable<RHI_DepthStencilView> {
		mutable Set<struct OpenGL_RenderTarget*> m_render_targets;
		OpenGL_Surface* const m_surface;

		OpenGL_SurfaceDSV(OpenGL_Surface* surface);
		void clear(float depth, byte stencil) override;
		void blit(RHI_DepthStencilView* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~OpenGL_SurfaceDSV();
	};
}// namespace Engine
