#include <Graphics/render_surface.hpp>
#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_viewport.hpp>

namespace Engine
{
    void OpenGL_Viewport::vsync(bool flag)
    {}

    void OpenGL_Viewport::on_resize(const Size2D& new_size)
    {}

    // Window Viewport

    void make_window_current(Window* window, void* context);
    bool has_window_vsync(Window* window, void* context);
    void set_window_vsync(Window* window, void* context, bool flag);
    void swap_window_buffers(Window* window, void* context);

    void OpenGL_WindowViewport::init(RenderViewport* viewport)
    {
        m_viewport = viewport;
    }


    static OpenGL_WindowViewport* m_current_viewport = nullptr;

    void OpenGL_WindowViewport::make_current()
    {
        if (m_current_viewport != this)
        {
            m_current_viewport = this;
            make_window_current(m_viewport->window(), OPENGL_API->context());
            vsync(m_viewport->vsync());
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

        ViewPort viewport;
        viewport.pos       = {0, 0};
        viewport.size      = m_viewport->window()->cached_size();
        viewport.min_depth = 0.f;
        viewport.max_depth = 1.f;
        OPENGL_API->viewport(viewport);

        Scissor scissor;
        scissor.pos  = {0.f, 0.f};
        scissor.size = viewport.size;
        OPENGL_API->scissor(scissor);
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

    void OpenGL_WindowViewport::blit_target(RenderSurface* surface, const Rect2D& src_rect, const Rect2D& dst_rect,
                                            SamplerFilter filter)
    {
        RenderSurface* surface_array[] = {surface};
        auto render_target             = OpenGL_RenderTarget::find_or_create(surface_array, nullptr);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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

    void OpenGL_WindowViewport::clear_color(const Color& color)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(color.r, color.g, color.b, color.a);
        glClear(GL_COLOR_BUFFER_BIT);

        if (OPENGL_API->m_state.render_target)
        {
            OPENGL_API->m_state.render_target->bind();
        }
    }

    OpenGL_WindowViewport::~OpenGL_WindowViewport()
    {}

    RHI_Viewport* OpenGL::create_viewport(RenderViewport* engine_viewport)
    {
        OpenGL_WindowViewport* viewport = new OpenGL_WindowViewport();
        viewport->init(engine_viewport);
        return viewport;
    }
}// namespace Engine
