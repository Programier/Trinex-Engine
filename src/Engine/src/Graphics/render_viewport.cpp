#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/render_thread.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>

namespace Engine
{

    implement_engine_class_default_init(RenderViewport);
    implement_engine_class_default_init(ViewportClient);

    static RenderViewport* _M_current_render_viewport = nullptr;

    ViewportClient& ViewportClient::on_bind_to_viewport(class RenderViewport* viewport)
    {
        return *this;
    }

    ViewportClient& ViewportClient::render(RenderViewport* viewport)
    {
        return *this;
    }

    ViewportClient& ViewportClient::update(class RenderViewport* viewport, float dt)
    {
        return *this;
    }

    ViewportClient* ViewportClient::create(const StringView& name)
    {
        Class* client_class = Class::static_find(name);
        if (client_class)
        {
            Object* object         = client_class->create_object();
            ViewportClient* client = object->instance_cast<ViewportClient>();
            if (client)
            {
                return client;
            }
            else if (object)
            {
                delete object;
            }
        }

        return nullptr;
    }

    List<RenderViewport*> RenderViewport::_M_viewports;

    RenderViewport::RenderViewport()
    {
        _M_viewports.push_back(this);
    }

    RenderViewport::~RenderViewport()
    {
        _M_viewports.remove(this);
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

    RenderTargetBase* RenderViewport::base_render_target() const
    {
        return _M_render_target_base;
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

    RHI_RenderTarget* RenderViewport::rhi_render_target()
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport)
        {
            return viewport->render_target();
        }
        return nullptr;
    }

    Size2D RenderViewport::size() const
    {
        RenderTargetBase* rt = reinterpret_cast<RenderTargetBase*>(_M_handle);
        return rt->viewport().size;
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
            if (_M_client)
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
        if (new_client)
        {
            new_client->on_bind_to_viewport(this);
        }
        return *this;
    }

    RenderViewport& RenderViewport::update(float dt)
    {
        if (_M_client)
        {
            _M_current_render_viewport = this;
            _M_client->update(this, dt);
            _M_current_render_viewport = nullptr;
        }
        return *this;
    }

    RenderViewport& RenderViewport::rhi_bind()
    {
        if (_M_handle)
        {
            reinterpret_cast<RenderTargetBase*>(_M_handle)->rhi_bind();
        }

        return *this;
    }

    RenderViewport* RenderViewport::current()
    {
        return _M_current_render_viewport;
    }

    const List<RenderViewport*>& RenderViewport::viewports()
    {
        return _M_viewports;
    }
}// namespace Engine
