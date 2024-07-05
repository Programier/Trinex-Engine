#pragma once
#include <Core/pointer.hpp>
#include <Core/render_resource.hpp>

namespace Engine
{

    class Window;
    class RenderTarget;
    class RenderSurface;


    class ENGINE_EXPORT ViewportClient : public Object
    {
        declare_class(ViewportClient, Object);

    public:
        virtual ViewportClient& on_bind_viewport(class RenderViewport* viewport);
        virtual ViewportClient& on_unbind_viewport(class RenderViewport* viewport);
        virtual ViewportClient& render(class RenderViewport* viewport);
        virtual ViewportClient& update(class RenderViewport* viewport, float dt);

        static ViewportClient* create(const StringView& name);
    };

    class ENGINE_EXPORT RenderViewport : public RenderResource
    {
        declare_class(RenderViewport, RenderResource);

    private:
        static List<RenderViewport*> m_viewports;
        Atomic<bool> m_vsync = true;
        class Window* m_window;
        Pointer<ViewportClient> m_client;

        RenderViewport& window(Window* window, bool vsync);

    public:
        RenderViewport();
        ~RenderViewport();

        RenderViewport& rhi_create() override;
        Window* window() const;
        Size2D size() const;

        bool vsync();
        RenderViewport& vsync(bool flag);
        RenderViewport& on_resize(const Size2D& new_size);
        RenderViewport& render();

        ViewportClient* client() const;
        RenderViewport& client(ViewportClient* client);
        RenderViewport& update(float dt);
        RenderViewport& rhi_bind();
        RenderViewport& rhi_blit_target(RenderSurface* surface, const Rect2D& src, const Rect2D& dst,
                                        SamplerFilter filter = SamplerFilter::Trilinear);
        RenderViewport& rhi_clear_color(const Color& color);

        static RenderViewport* current();
        static const List<RenderViewport*>& viewports();

        friend class Window;
    };
}// namespace Engine
