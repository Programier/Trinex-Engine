#pragma once
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
    class OverlaySceneLayer;
    class EditorSceneRenderer : public SceneRenderer
    {
    private:
        OverlaySceneLayer* m_overlay_layer;

    public:
        EditorSceneRenderer();

        // Components rendering
        EditorSceneRenderer& render_component(PrimitiveComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
        EditorSceneRenderer& render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
    };
}// namespace Engine
