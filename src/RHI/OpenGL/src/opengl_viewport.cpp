#include <Core/exception.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_texture.hpp>
#include <opengl_viewport.hpp>

namespace Engine
{
	static OpenGL_Viewport* m_current_viewport = nullptr;

	void make_window_current(Window* window, void* context);
	bool has_window_vsync(Window* window, void* context);
	void set_window_vsync(Window* window, void* context, bool flag);
	void swap_window_buffers(Window* window, void* context);

	static FORCE_INLINE GLenum filter_of(SamplerFilter filter)
	{
		switch (filter)
		{
			case SamplerFilter::Bilinear:
			case SamplerFilter::Trilinear:
				return GL_LINEAR;

			default:
				return GL_NEAREST;
		}
	}

	void OpenGL_Viewport::present()
	{
		make_current();
		swap_window_buffers(m_viewport->window(), OPENGL_API->context());
		OPENGL_API->reset_state();

		glFlush();
	}

	void OpenGL_Viewport::make_current()
	{
		if (m_current_viewport != this)
		{
			m_current_viewport = this;
			make_window_current(m_viewport->window(), OPENGL_API->context());
			vsync(m_vsync);
		}
	}

	void OpenGL_Viewport::init(WindowRenderViewport* viewport, bool vsync)
	{
		m_viewport = viewport;
		m_vsync    = vsync;
	}

	void OpenGL_Viewport::on_resize(const Size2D& new_size) {}

	void OpenGL_Viewport::on_orientation_changed(Orientation orientation) {}

	void OpenGL_Viewport::clear_color(const LinearColor& color)
	{
		GLint current_fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

		if (current_fbo != 0)
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);

		if (current_fbo != 0)
			glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
	}

	void OpenGL_Viewport::blit_target(RHI_RenderTargetView* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
	                                  SamplerFilter filter)
	{
		auto render_target = OpenGL_RenderTarget::find_or_create(static_cast<OpenGL_TextureRTV*>(surface));

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, render_target->m_framebuffer);

		auto src_start = src_rect.pos;
		auto src_end   = src_start + Vector2i(src_rect.size);
		auto dst_start = dst_rect.pos;
		auto dst_end   = dst_start + Vector2i(dst_rect.size);

		glBlitFramebuffer(src_start.x, src_start.y, src_end.x, src_end.y, dst_start.x, dst_start.y, dst_end.x, dst_end.y,
		                  GL_COLOR_BUFFER_BIT, filter_of(filter));

		if (OPENGL_API->m_state.render_target)
		{
			OPENGL_API->m_state.render_target->bind();
		}
	}

	int_t OpenGL_Viewport::framebuffer_id()
	{
		return 0;
	}

	void OpenGL_Viewport::vsync(bool flag)
	{
		static bool current = !flag;

		if (current != flag)
		{
			set_window_vsync(m_viewport->window(), OPENGL_API->context(), flag);
			current = flag;
		}
	}

	void OpenGL_Viewport::bind()
	{
		OPENGL_API->m_state.render_target = nullptr;
		make_current();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	RHI_Viewport* OpenGL::create_viewport(WindowRenderViewport* engine_viewport, bool vsync)
	{
		auto* viewport = new OpenGL_Viewport();
		viewport->init(engine_viewport, vsync);
		return viewport;
	}
}// namespace Engine
