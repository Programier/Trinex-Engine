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
    class DepthRenderingLayer;
    class SceneLayer;
    class LightingSceneLayer;
    class DeferredLightingSceneLayer;
    class StaticMeshComponent;
    class SpriteComponent;
    class Scene;
    class RenderTargetBase;
    class PrimitiveComponent;
    class LightComponent;
    class LocalLightComponent;
    class PointLightComponent;
    class SpotLightComponent;
    class DirectionalLightComponent;
    class CommandBufferLayer;


#define implement_empty_rendering_methods_for(type)                                                                              \
    SceneRenderer& SceneRenderer::render_component(type* component)                                                              \
    {                                                                                                                            \
        return render_base_component(component);                                                                                 \
    }

    class ENGINE_EXPORT SceneRenderer
    {
    protected:
        SceneLayer* m_root_layer = nullptr;

        Vector<GlobalShaderParameters> m_global_shader_params;
        Vector<SceneView> m_scene_views;

        SceneRenderer& setup_parameters(RenderTargetBase* render_target = nullptr);

    public:
        Scene* scene;

        SceneRenderer();

        const GlobalShaderParameters& global_shader_parameters() const;
        const SceneView& scene_view() const;
        SceneRenderer& begin_rendering_target(RenderTargetBase* render_target, class RenderPass* render_pass = nullptr);
        SceneRenderer& end_rendering_target();

        virtual SceneRenderer& render(const SceneView& view, RenderTargetBase* render_target);

        // Components rendering
        template<typename ComponentType>
        FORCE_INLINE SceneRenderer& render_base_component(ComponentType* component)
        {
            return render_component(static_cast<ComponentType::Super*>(component));
        }

        virtual SceneRenderer& render_component(PrimitiveComponent* component);
        virtual SceneRenderer& render_component(StaticMeshComponent* component);
        virtual SceneRenderer& render_component(SpriteComponent* component);
        virtual SceneRenderer& render_component(LightComponent* component);
        virtual SceneRenderer& render_component(LocalLightComponent* component);
        virtual SceneRenderer& render_component(PointLightComponent* component);
        virtual SceneRenderer& render_component(SpotLightComponent* component);
        virtual SceneRenderer& render_component(DirectionalLightComponent* component);


        FORCE_INLINE SceneLayer* root_layer() const
        {
            return m_root_layer;
        }

        virtual ~SceneRenderer();
    };


    class DepthSceneRenderer : public SceneRenderer
    {
    };

    class ENGINE_EXPORT ColorSceneRenderer : public SceneRenderer
    {
    private:
        DepthSceneRenderer* m_depth_renderer = nullptr;

        // Layers
        SceneLayer* m_clear_layer                     = nullptr;
        DepthRenderingLayer* m_depth_layer            = nullptr;
        CommandBufferLayer* m_base_pass_layer         = nullptr;
        CommandBufferLayer* m_deferred_lighting_layer = nullptr;
        SceneLayer* m_scene_output                    = nullptr;
        SceneLayer* m_post_process_layer              = nullptr;


        ViewMode m_view_mode;

        void copy_gbuffer_to_scene_output();

    public:
        ColorSceneRenderer();
        ~ColorSceneRenderer();
        delete_copy_constructors(ColorSceneRenderer);

        static PolicyID policy_id();

        // Layers getters

        FORCE_INLINE SceneLayer* root_layer() const
        {
            return m_root_layer;
        }

        FORCE_INLINE SceneLayer* clear_layer() const
        {
            return m_clear_layer;
        }

        FORCE_INLINE DepthRenderingLayer* depth_layer() const
        {
            return m_depth_layer;
        }

        FORCE_INLINE CommandBufferLayer* base_pass_layer() const
        {
            return m_base_pass_layer;
        }

        FORCE_INLINE CommandBufferLayer* deferred_lighting_layer() const
        {
            return m_deferred_lighting_layer;
        }

        FORCE_INLINE SceneLayer* scene_output_layer() const
        {
            return m_scene_output;
        }

        FORCE_INLINE SceneLayer* post_process_layer() const
        {
            return m_post_process_layer;
        }

        virtual DepthSceneRenderer* create_depth_renderer();

        // Components rendering
        using SceneRenderer::render_component;
        ColorSceneRenderer& render_component(StaticMeshComponent* component) override;
        ColorSceneRenderer& render_component(SpriteComponent* component) override;
        ColorSceneRenderer& render_component(PointLightComponent* component) override;
        ColorSceneRenderer& render_component(SpotLightComponent* component) override;
        ColorSceneRenderer& render_component(DirectionalLightComponent* component) override;

        FORCE_INLINE ViewMode view_mode() const
        {
            return m_view_mode;
        }

        ColorSceneRenderer& view_mode(ViewMode new_mode);
    };

}// namespace Engine
