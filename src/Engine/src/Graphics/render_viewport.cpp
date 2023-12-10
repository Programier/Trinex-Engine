#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_thread_call.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>

namespace Engine
{

    implement_engine_class_default_init(RenderViewport);
    implement_engine_class_default_init(ViewportClient);


    ViewportClient& ViewportClient::render(RenderViewport* viewport)
    {
        return *this;
    }

    Vector<RenderViewport*> RenderViewport::_M_viewports;

    RenderViewport::RenderViewport()
    {
        _M_viewports.push_back(this);
    }

    RenderViewport::~RenderViewport()
    {
        for (size_t i = 0, count = _M_viewports.size(); i < count; i++)
        {
            if (_M_viewports[i] == this)
            {
                _M_viewports.erase(_M_viewports.begin() + i);
                break;
            }
        }
    }

    RenderViewport& RenderViewport::rhi_create()
    {
        RHI* rhi = engine_instance->rhi();
        if (_M_type == Type::Window)
        {
            _M_rhi_object.reset(rhi->create_viewport(_M_window->interface(), _M_vsync));
        }
        else
        {
            _M_rhi_object.reset(rhi->create_viewport(_M_render_target));
        }
        return *this;
    }

    Window* RenderViewport::window() const
    {
        return _M_type == Type::Window ? _M_window : nullptr;
    }

    RenderTarget* RenderViewport::render_target() const
    {
        return _M_type == Type::RenderTarget ? _M_render_target : nullptr;
    }

    RenderViewport& RenderViewport::window(Window* window, bool vsync)
    {
        _M_vsync  = vsync;
        _M_type   = Type::Window;
        _M_window = window;
        return *this;
    }

    RenderViewport& RenderViewport::render_target(RenderTarget* rt)
    {
        _M_type          = Type::RenderTarget;
        _M_render_target = rt;
        return *this;
    }

    RenderViewport::Type RenderViewport::type() const
    {
        return _M_type;
    }


    bool RenderViewport::vsync()
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport)
        {
            return viewport->vsync();
        }
        return false;
    }

    RenderViewport& RenderViewport::vsync(bool flag)
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport)
        {
            viewport->vsync(flag);
        }
        return *this;
    }

    RenderViewport& RenderViewport::on_resize(const Size2D& new_size)
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport)
        {
            call_in_render_thread([viewport, new_size]() {//
                viewport->on_resize(new_size);
            });
        }
        return *this;
    }

    RHI_RenderTarget* RenderViewport::render_target()
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport)
        {
            return viewport->render_target();
        }
        return nullptr;
    }


    class StartRenderingViewport : public ExecutableObject
    {
    public:
        ViewportClient* _M_client;
        RenderViewport* _M_viewport;
        RHI_Viewport* _M_rhi_viewport;

        StartRenderingViewport(ViewportClient* client, RenderViewport* viewport, RHI_Viewport* rhi_viewport)
            : _M_client(client), _M_viewport(viewport), _M_rhi_viewport(rhi_viewport)
        {}

        int_t execute() override
        {
            _M_rhi_viewport->begin_render();
            _M_client->render(_M_viewport);
            _M_rhi_viewport->end_render();

            return sizeof(StartRenderingViewport);
        }
    };


    RenderViewport& RenderViewport::render()
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport == nullptr)
            return *this;

        engine_instance->thread(ThreadType::RenderThread)
                ->insert_new_task<StartRenderingViewport>(_M_client.ptr(), this, viewport);
        return *this;
    }

    ViewportClient* RenderViewport::client() const
    {
        return _M_client.ptr();
    }

    RenderViewport& RenderViewport::client(ViewportClient* new_client)
    {
        _M_client = new_client;
        return *this;
    }

    const Vector<RenderViewport*>& RenderViewport::viewports()
    {
        return _M_viewports;
    }
}// namespace Engine
