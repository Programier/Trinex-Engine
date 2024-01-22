#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{

    class Window;
    class RenderTarget;


    class ENGINE_EXPORT ViewportClient : public Object
    {
        declare_class(ViewportClient, Object);

    public:
        virtual ViewportClient& on_bind_to_viewport(class RenderViewport* viewport);
        virtual ViewportClient& render(class RenderViewport* viewport);
        virtual ViewportClient& update(class RenderViewport* viewport, float dt);
        virtual ViewportClient& prepare_render(class RenderViewport* viewport);
    };

    class ENGINE_EXPORT RenderViewport : public RenderResource
    {
        declare_class(RenderViewport, RenderResource);

    public:
        enum class Type
        {
            Undefined,
            Window,
            RenderTarget
        };

    private:
        static List<RenderViewport*> _M_viewports;

        Type _M_type  = Type::Undefined;
        bool _M_vsync = true;

        union
        {
            void* _M_handle = nullptr;
            class Window* _M_window;
            class RenderTarget* _M_render_target;
        };

        Pointer<ViewportClient> _M_client;

    public:
        RenderViewport();
        ~RenderViewport();

        RenderViewport& rhi_create() override;
        Window* window() const;
        RenderTarget* render_target() const;
        RenderViewport& window(Window* window_interface, bool vsync);
        RenderViewport& render_target(RenderTarget* rt);
        Type type() const;
        Size2D size() const;

        bool vsync();
        RenderViewport& vsync(bool flag);
        RenderViewport& on_resize(const Size2D& new_size);
        RHI_RenderTarget* render_target();
        RenderViewport& render();

        ViewportClient* client() const;
        RenderViewport& client(ViewportClient* client);
        RenderViewport& update(float dt);
        RenderViewport& prepare_render();

        static const List<RenderViewport*>& viewports();
    };
}// namespace Engine
