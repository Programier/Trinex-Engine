#include <Graphics/render_target.hpp>
#include <Window/window_interface.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_viewport.hpp>

namespace Engine
{
    bool OpenGL_Viewport::vsync()
    {
        return false;
    }

    void OpenGL_Viewport::vsync(bool flag)
    {}

    void OpenGL_Viewport::on_resize(const Size2D& new_size)
    {}

    RHI_RenderTarget* OpenGL_Viewport::render_target()
    {
        return m_render_target;
    }


    // Render Target Viewport

    void OpenGL_RenderTargetViewport::init(RenderTarget* render_target)
    {
        m_render_target = render_target->rhi_object<OpenGL_RenderTarget>();
    }


    void OpenGL_RenderTargetViewport::begin_render()
    {
        OPENGL_API->reset_state();
    }

    void OpenGL_RenderTargetViewport::end_render()
    {}

    // Window Viewport

    void OpenGL_WindowViewport::init(WindowInterface* window, bool vsync)
    {
        if (!(OPENGL_API->m_context))
        {
            m_context             = window->create_api_context("");
            OPENGL_API->m_context = m_context;
            window->bind_api_context(m_context);

            OPENGL_API->initialize();
        }
        else
        {
            window->bind_api_context(OPENGL_API->m_context);
        }

        m_window        = window;
        m_render_target = new OpenGL_MainRenderTarget();
    }


    bool OpenGL_WindowViewport::vsync()
    {
        return m_window->vsync();
    }

    void OpenGL_WindowViewport::vsync(bool flag)
    {
        m_window->vsync(flag);
    }

    void OpenGL_WindowViewport::begin_render()
    {
        OPENGL_API->reset_state();
        m_window->make_current();
    }

    void OpenGL_WindowViewport::end_render()
    {
        m_window->present();
    }

    OpenGL_WindowViewport::~OpenGL_WindowViewport()
    {
        if (m_context)
        {
            m_window->destroy_api_context();
            m_context = nullptr;
        }
        delete m_render_target;
    }

    RHI_Viewport* OpenGL::create_viewport(WindowInterface* interface, bool vsync)
    {
        OpenGL_WindowViewport* viewport = new OpenGL_WindowViewport();
        viewport->init(interface, vsync);
        return viewport;
    }

    RHI_Viewport* OpenGL::create_viewport(RenderTarget* render_target)
    {
        OpenGL_RenderTargetViewport* viewport = new OpenGL_RenderTargetViewport();
        viewport->init(render_target);
        return viewport;
    }
}// namespace Engine
