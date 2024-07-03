#pragma once
#include <Graphics/render_viewport.hpp>

namespace Engine
{
    // This viewport can be used for testing something
    class ENGINE_EXPORT DefaultClient : public ViewportClient
    {
        declare_class(DefaultClient, ViewportClient);

    public:
        DefaultClient();
        DefaultClient& render(class RenderViewport* viewport) override;
        DefaultClient& update(class RenderViewport* viewport, float dt) override;
    };
}// namespace Engine
