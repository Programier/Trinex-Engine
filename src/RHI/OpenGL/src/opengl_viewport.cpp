#include <Core/exception.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_viewport.hpp>

namespace Engine
{
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

	void OpenGL_Viewport::on_resize(const Size2D& new_size)
	{}

	void OpenGL_Viewport::on_orientation_changed(Orientation orientation)
	{}

	void OpenGL_Viewport::clear_color(const Color& color)
	{
		GLint current_fbo;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &current_fbo);

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id());

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glClearColor(color.r, color.g, color.b, color.a);
		glClear(GL_COLOR_BUFFER_BIT);

		if (OPENGL_API->m_state.render_target)
		{
			OPENGL_API->m_state.render_target->bind();
		}

		glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
	}

	void OpenGL_Viewport::blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
	                                  SamplerFilter filter)
	{
		RenderSurface* surface_array[] = {surface};
		auto render_target             = OpenGL_RenderTarget::find_or_create(surface_array, nullptr);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer_id());
		glBindFramebuffer(GL_READ_FRAMEBUFFER, render_target->m_framebuffer);

		auto src_start = src_rect.position;
		auto src_end   = src_start + src_rect.size;
		auto dst_start = dst_rect.position;
		auto dst_end   = dst_start + dst_rect.size;

		glBlitFramebuffer(src_start.x, src_start.y, src_end.x, src_end.y, dst_start.x, dst_start.y, dst_end.x, dst_end.y,
		                  GL_COLOR_BUFFER_BIT, filter_of(filter));

		if (OPENGL_API->m_state.render_target)
		{
			OPENGL_API->m_state.render_target->bind();
		}
	}


	// Surface Viewport
	void OpenGL_SurfaceViewport::begin_render()
	{}

	void OpenGL_SurfaceViewport::end_render()
	{}


	void OpenGL_SurfaceViewport::init(SurfaceRenderViewport* viewport)
	{
		m_surface[0] = viewport->rhi_object<OpenGL_RenderSurface>();
	}

	void OpenGL_SurfaceViewport::vsync(bool flag)
	{}

	void OpenGL_SurfaceViewport::bind()
	{
		if (m_surface[0])
			OPENGL_API->bind_render_target(m_surface, nullptr);
	}

	int_t OpenGL_SurfaceViewport::framebuffer_id()
	{
		if (!m_surface[0])
			throw EngineException("Invalid framebuffer!");

		return OpenGL_RenderTarget::find_or_create(m_surface, nullptr)->m_framebuffer;
	}

	// Window Viewport

	void make_window_current(Window* window, void* context);
	bool has_window_vsync(Window* window, void* context);
	void set_window_vsync(Window* window, void* context, bool flag);
	void swap_window_buffers(Window* window, void* context);

	void OpenGL_WindowViewport::init(WindowRenderViewport* viewport, bool vsync)
	{
		m_viewport = viewport;
		m_vsync    = vsync;
	}


	static OpenGL_WindowViewport* m_current_viewport = nullptr;

	void OpenGL_WindowViewport::make_current()
	{
		if (m_current_viewport != this)
		{
			m_current_viewport = this;
			make_window_current(m_viewport->window(), OPENGL_API->context());
			vsync(m_vsync);
		}
	}

	OpenGL_WindowViewport* OpenGL_WindowViewport::current()
	{
		return m_current_viewport;
	}

	void OpenGL_WindowViewport::vsync(bool flag)
	{
		static bool current = !flag;

		if (current != flag)
		{
			set_window_vsync(m_viewport->window(), OPENGL_API->context(), flag);
			current = flag;
		}
	}

	void OpenGL_WindowViewport::begin_render()
	{
		OPENGL_API->reset_state();
		make_current();
	}

	void OpenGL_WindowViewport::end_render()
	{
		swap_window_buffers(m_viewport->window(), OPENGL_API->context());
	}

	void OpenGL_WindowViewport::bind()
	{
		OPENGL_API->m_state.render_target = nullptr;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	int_t OpenGL_WindowViewport::framebuffer_id()
	{
		return 0;
	}

	OpenGL_WindowViewport::~OpenGL_WindowViewport()
	{}

	RHI_Viewport* OpenGL::create_viewport(SurfaceRenderViewport* engine_viewport)
	{
		OpenGL_SurfaceViewport* viewport = new OpenGL_SurfaceViewport();
		viewport->init(engine_viewport);
		return viewport;
	}

	RHI_Viewport* OpenGL::create_viewport(WindowRenderViewport* engine_viewport, bool vsync)
	{
		OpenGL_WindowViewport* viewport = new OpenGL_WindowViewport();
		viewport->init(engine_viewport, vsync);
		return viewport;
	}
}// namespace Engine
