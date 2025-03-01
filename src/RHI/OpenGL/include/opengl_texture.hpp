#pragma once
#include <Core/etl/set.hpp>
#include <Graphics/rhi.hpp>
#include <opengl_color_format.hpp>
#include <opengl_headers.hpp>

namespace Engine
{
	struct OpenGL_Texture : public RHI_DefaultDestroyable<RHI_Texture2D> {
		OpenGL_ColorInfo m_format;
		GLuint m_type = 0;
		GLuint m_id   = 0;
		Size2D m_size;

		void bind(BindLocation location) override;
		void bind_combined(RHI_Sampler* sampler, BindLocation location) override;
		void init(const Texture2D* texture);

		~OpenGL_Texture();
	};

	struct OpenGL_RenderSurface : public OpenGL_Texture {
		mutable Set<struct OpenGL_RenderTarget*> m_render_targets;

		void clear_color(const Color& color) override;
		void clear_depth_stencil(float depth, byte stencil) override;
		void blit(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;
		~OpenGL_RenderSurface();
	};
}// namespace Engine
