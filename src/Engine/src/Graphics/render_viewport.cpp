#include <Core/class.hpp>
#include <Core/base_engine.hpp>
#include <Core/threading.hpp>
#include <Graphics/render_target_base.hpp>
#include <Graphics/render_viewport.hpp>
#include <Graphics/rhi.hpp>
#include <Window/window.hpp>

namespace Engine
{

    implement_engine_class_default_init(RenderViewport);
    implement_engine_class_default_init(ViewportClient);

    static RenderViewport* m_current_render_viewport = nullptr;

    ViewportClient& ViewportClient::on_bind_viewport(class RenderViewport* viewport)
    {
        return *this;
    }

    ViewportClient& ViewportClient::on_unbind_viewport(class RenderViewport* viewport)
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

    List<RenderViewport*> RenderViewport::m_viewports;

    RenderViewport::RenderViewport()
    {
        m_viewports.push_back(this);
    }

    RenderViewport::~RenderViewport()
    {
        m_viewports.remove(this);
    }

    RenderViewport& RenderViewport::rhi_create()
    {
        if (m_type == Type::Window)
        {
            m_rhi_object.reset(rhi->create_viewport(m_window->interface(), m_vsync));
        }
        else
        {
            m_rhi_object.reset(rhi->create_viewport(m_render_target));
        }
        return *this;
    }

    Window* RenderViewport::window() const
    {
        return m_type == Type::Window ? m_window : nullptr;
    }

    RenderTarget* RenderViewport::render_target() const
    {
        return m_type == Type::RenderTarget ? m_render_target : nullptr;
    }

    RenderTargetBase* RenderViewport::base_render_target() const
    {
        return m_render_target_base;
    }

    RenderViewport& RenderViewport::window(Window* window, bool vsync)
    {
        m_vsync  = vsync;
        m_type   = Type::Window;
        m_window = window;
        return *this;
    }

    RenderViewport& RenderViewport::render_target(RenderTarget* rt)
    {
        m_type          = Type::RenderTarget;
        m_render_target = rt;
        return *this;
    }

    RenderViewport::Type RenderViewport::type() const
    {
        return m_type;
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
        RenderTargetBase* rt = reinterpret_cast<RenderTargetBase*>(m_handle);
        return rt->viewport().size;
    }


    class StartRenderingViewport : public ExecutableObject
    {
    public:
        ViewportClient* m_client;
        RenderViewport* m_viewport;
        RHI_Viewport* m_rhi_viewport;

        StartRenderingViewport(ViewportClient* client, RenderViewport* viewport, RHI_Viewport* rhi_viewport)
            : m_client(client), m_viewport(viewport), m_rhi_viewport(rhi_viewport)
        {}

        int_t execute() override
        {
            m_rhi_viewport->begin_render();
            if (m_client)
                m_client->render(m_viewport);
            m_rhi_viewport->end_render();

            return sizeof(StartRenderingViewport);
        }
    };

    RenderViewport& RenderViewport::render()
    {
        RHI_Viewport* viewport = rhi_object<RHI_Viewport>();
        if (viewport == nullptr)
            return *this;
        render_thread()->insert_new_task<StartRenderingViewport>(m_client.ptr(), this, viewport);
        return *this;
    }

    ViewportClient* RenderViewport::client() const
    {
        return m_client.ptr();
    }

    RenderViewport& RenderViewport::client(ViewportClient* new_client)
    {
        if (m_client)
        {
            ViewportClient* tmp = m_client;
            m_client            = nullptr;
            tmp->on_unbind_viewport(this);
        }

        m_client = new_client;

        if (new_client)
        {
            new_client->on_bind_viewport(this);
        }
        return *this;
    }

    RenderViewport& RenderViewport::update(float dt)
    {
        if (m_client)
        {
            m_current_render_viewport = this;
            m_client->update(this, dt);
            m_current_render_viewport = nullptr;
        }
        return *this;
    }

    RenderViewport& RenderViewport::rhi_bind()
    {
        if (m_handle)
        {
            reinterpret_cast<RenderTargetBase*>(m_handle)->rhi_bind();
        }

        return *this;
    }

    RenderViewport* RenderViewport::current()
    {
        return m_current_render_viewport;
    }

    const List<RenderViewport*>& RenderViewport::viewports()
    {
        return m_viewports;
    }
}// namespace Engine
