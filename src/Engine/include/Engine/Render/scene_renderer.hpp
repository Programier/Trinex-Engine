#pragma once
#include <Core/enums.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;
    class SceneLayer;
    class StaticMeshComponent;
    class SpriteComponent;
    class Scene;
    class RenderTargetBase;
    class PrimitiveComponent;
    class LightComponent;
    class PointLightComponent;


    class ENGINE_EXPORT SceneRenderer
    {
    private:
        Scene* m_scene;

        GlobalShaderParameters m_global_shader_params;
        SceneView m_scene_view;
        ViewMode m_view_mode;

        SceneRenderer& setup_parameters(RenderTargetBase* render_target = nullptr, SceneView* scene_view = nullptr);

        void copy_gbuffer_to_scene_output();

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(Scene* scene);
        Scene* scene() const;
        SceneRenderer& render(const SceneView& view, RenderTargetBase* render_target);

        // Render targets manipulation
        SceneRenderer& begin_rendering_target(RenderTargetBase* render_target, class RenderPass* render_pass = nullptr);
        void clear_render_targets(RenderTargetBase*, SceneLayer*);
        void begin_rendering_base_pass(RenderTargetBase*, SceneLayer*);
        void begin_deferred_lighting_pass(RenderTargetBase*, SceneLayer*);
        void begin_lighting_pass(RenderTargetBase*, SceneLayer*);
        void begin_scene_output_pass(RenderTargetBase*, SceneLayer*);
        void begin_postprocess_pass(RenderTargetBase*, SceneLayer*);
        void end_rendering_target(RenderTargetBase*, SceneLayer*);
        SceneRenderer& end_rendering_target();


        // Add components to scene layers
        virtual SceneRenderer& add_component(PrimitiveComponent* component, Scene* scene);
        virtual SceneRenderer& add_component(StaticMeshComponent* component, Scene* scene);
        virtual SceneRenderer& add_component(SpriteComponent* component, Scene* scene);
        virtual SceneRenderer& add_component(LightComponent* component, Scene* scene);
        virtual SceneRenderer& add_component(PointLightComponent* component, Scene* scene);

        // Components rendering
        virtual SceneRenderer& render_component(PrimitiveComponent* component, RenderTargetBase* rt, SceneLayer* layer);
        virtual SceneRenderer& render_component(StaticMeshComponent* component, RenderTargetBase* rt, SceneLayer* layer);
        virtual SceneRenderer& render_component(SpriteComponent* component, RenderTargetBase* rt, SceneLayer* layer);
        virtual SceneRenderer& render_component(LightComponent* component, RenderTargetBase* rt, SceneLayer* layer);
        virtual SceneRenderer& render_component(PointLightComponent* component, RenderTargetBase* rt, SceneLayer* layer);

        FORCE_INLINE const GlobalShaderParameters& shader_params() const
        {
            return m_global_shader_params;
        }

        FORCE_INLINE const SceneView& scene_view() const
        {
            return m_scene_view;
        }

        FORCE_INLINE ViewMode view_mode() const
        {
            return m_view_mode;
        }

        SceneRenderer& view_mode(ViewMode new_mode);
        virtual ~SceneRenderer();
    };

}// namespace Engine
