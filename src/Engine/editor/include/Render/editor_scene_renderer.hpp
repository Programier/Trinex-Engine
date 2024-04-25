#pragma once
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
    class EditorSceneRenderer : public SceneRenderer
    {
    public:
        // Components rendering
        EditorSceneRenderer& render_component(PrimitiveComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
        EditorSceneRenderer& render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
        EditorSceneRenderer& render_component(PointLightComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
        EditorSceneRenderer& render_component(SpotLightComponent* component, RenderTargetBase* rt, SceneLayer* layer) override;
    };
}// namespace Engine
