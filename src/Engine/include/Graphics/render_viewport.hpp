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

        static ViewportClient* create(const StringView& name);
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
        static List<RenderViewport*> m_viewports;

        Type m_type  = Type::Undefined;
        bool m_vsync = true;

        union
        {
            void* m_handle = nullptr;
            class Window* m_window;
            class RenderTarget* m_render_target;
            class RenderTargetBase* m_render_target_base;
        };

        Pointer<ViewportClient> m_client;

    public:
        RenderViewport();
        ~RenderViewport();

        RenderViewport& rhi_create() override;
        Window* window() const;
        RenderTarget* render_target() const;
        RenderTargetBase* base_render_target() const;
        RenderViewport& window(Window* window_interface, bool vsync);
        RenderViewport& render_target(RenderTarget* rt);
        Type type() const;
        Size2D size() const;

        bool vsync();
        RenderViewport& vsync(bool flag);
        RenderViewport& on_resize(const Size2D& new_size);
        RHI_RenderTarget* rhi_render_target();
        RenderViewport& render();

        ViewportClient* client() const;
        RenderViewport& client(ViewportClient* client);
        RenderViewport& update(float dt);
        RenderViewport& rhi_bind();

        static RenderViewport* current();

        static const List<RenderViewport*>& viewports();
    };
}// namespace Engine
