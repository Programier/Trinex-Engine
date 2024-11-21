#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
	class Window;
	struct OpenGL_RenderSurface;

	struct OpenGL_Viewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void clear_color(const Color& color) override;
		void blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect, SamplerFilter filter) override;

		virtual int_t framebuffer_id() = 0;
	};

	struct OpenGL_SurfaceViewport : OpenGL_Viewport {
		OpenGL_RenderSurface* m_surface[1];
		
		void begin_render() override;
		void end_render() override;
		
		void init(SurfaceRenderViewport* viewport);
		void vsync(bool flag) override;

		virtual void bind() override;
		virtual int_t framebuffer_id() override;
	};

	struct OpenGL_WindowViewport : OpenGL_Viewport {
		WindowRenderViewport* m_viewport = nullptr;
		bool m_vsync                     = false;

		void begin_render() override;
		void end_render() override;

		void init(WindowRenderViewport* viewport, bool vsync);
		void vsync(bool flag) override;
		void make_current();
		static OpenGL_WindowViewport* current();
		void bind() override;
		


		int_t framebuffer_id() override;
		~OpenGL_WindowViewport();
	};

}// namespace Engine
