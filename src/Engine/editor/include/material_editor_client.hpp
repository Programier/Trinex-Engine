#pragma once
#include <Graphics/render_viewport.hpp>

namespace Engine
{
    class MaterialEditorClient : public ViewportClient
    {
    public:
        declare_class(MaterialEditorClient, ViewportClient);

    public:
        MaterialEditorClient& on_bind_to_viewport(class RenderViewport* viewport) override;
        MaterialEditorClient& render(class RenderViewport* viewport) override;
        MaterialEditorClient& update(class RenderViewport* viewport, float dt) override;
        MaterialEditorClient& prepare_render(class RenderViewport* viewport) override;


        MaterialEditorClient& render_properties(float dt);
    };
}// namespace Engine
