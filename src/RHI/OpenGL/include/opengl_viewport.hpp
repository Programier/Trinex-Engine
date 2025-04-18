#pragma once
#include <Graphics/rhi.hpp>

namespace Engine
{
	class Window;
	struct OpenGL_RenderSurface;

	struct OpenGL_Viewport : public RHI_DefaultDestroyable<RHI_Viewport> {
		WindowRenderViewport* m_viewport = nullptr;
		bool m_vsync                     = false;

		void present() override;
		void init(WindowRenderViewport* viewport, bool vsync);
		void make_current();
		void vsync(bool flag) override;
		void bind() override;

		void on_resize(const Size2D& new_size) override;
		void on_orientation_changed(Orientation orientation) override;
		void clear_color(const Color& color) override;
		void blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
		                 SamplerFilter filter) override;

		int_t framebuffer_id();
	};
}// namespace Engine
