#include <Graphics/render_viewport.hpp>
#include <Window/window.hpp>
#include <opengl_api.hpp>
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
    }

    void OpenGL_WindowViewport::end_render()
    {
        swap_window_buffers(m_viewport->window(), OPENGL_API->context());
    }

    void OpenGL_WindowViewport::bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    OpenGL_WindowViewport::~OpenGL_WindowViewport()
    {}

    RHI_Viewport* OpenGL::create_viewport(RenderViewport* engine_viewport)
    {
        if (m_context == nullptr)
        {
            initialize_rhi();
        }

        OpenGL_WindowViewport* viewport = new OpenGL_WindowViewport();
        viewport->init(engine_viewport);
        return viewport;
    }
}// namespace Engine
