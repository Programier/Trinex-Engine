#pragma once
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
    class OverlaySceneLayer;
    class EditorSceneRenderer : public ColorSceneRenderer
    {
    private:
        OverlaySceneLayer* m_overlay_layer;

    public:
        EditorSceneRenderer();

        // Add components to scene layers
        EditorSceneRenderer& add_component(LightComponent* component, Scene* scene) override;

        // Components rendering
        EditorSceneRenderer& render_component(PrimitiveComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
    };
}// namespace Engine
