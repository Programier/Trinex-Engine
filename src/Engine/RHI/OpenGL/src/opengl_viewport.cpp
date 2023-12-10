#include <Graphics/render_target.hpp>
#include <Window/window_interface.hpp>
#include <opengl_api.hpp>
#include <opengl_render_target.hpp>
#include <opengl_viewport.hpp>

namespace Engine
{

    void OpenGL_Viewport::init(WindowInterface* window, bool vsync)
    {
        if (!(OPENGL_API->_M_context))
            OPENGL_API->_M_context = window->create_surface("");

        _M_window        = window;
        _M_render_target = new OpenGL_MainRenderTarget();
    }

    void OpenGL_Viewport::init(RenderTarget* render_target)
    {
        _M_render_target = render_target->rhi_object<OpenGL_RenderTarget>();
    }

    void OpenGL_Viewport::begin_render()
    {
        OPENGL_API->reset_state();
        if (_M_window)
        {
            _M_window->make_current(OPENGL_API->_M_context);
        }
    }

    void OpenGL_Viewport::end_render()
    {
        if (_M_window)
        {
            _M_window->present();
        }
    }

    bool OpenGL_Viewport::vsync()
    {
        return _M_window ? _M_window->vsync() : false;
    }

    void OpenGL_Viewport::vsync(bool flag)
    {
        if (_M_window)
        {
            _M_window->vsync(flag);
        }
    }

    void OpenGL_Viewport::on_resize(const Size2D& new_size)
    {}

    RHI_RenderTarget* OpenGL_Viewport::render_target()
    {
        return _M_render_target;
    }

    OpenGL_Viewport::~OpenGL_Viewport()
    {
        if (_M_window)
        {
            delete _M_render_target;
        }
    }

    RHI_Viewport* OpenGL::create_viewport(WindowInterface* interface, bool vsync)
    {
        OpenGL_Viewport* viewport = new OpenGL_Viewport();
        viewport->init(interface, vsync);
        return viewport;
    }

    RHI_Viewport* OpenGL::create_viewport(RenderTarget* render_target)
    {
        OpenGL_Viewport* viewport = new OpenGL_Viewport();
        viewport->init(render_target);
        return viewport;
    }
}// namespace Engine
