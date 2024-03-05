#pragma once
#include <Core/engine_types.hpp>
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


    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        class Scene* m_scene;

        GlobalShaderParameters m_global_shader_params;
        SceneView m_scene_view;

        SceneRenderer& setup_parameters(RenderTargetBase* render_target = nullptr, SceneView* scene_view = nullptr);

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(Scene* scene);
        Scene* scene() const;
        SceneRenderer& render(const SceneView& view, RenderTargetBase* render_target);

        void clear_render_targets(RenderTargetBase*, SceneLayer*);

        SceneRenderer& begin_rendering_target(RenderTargetBase* render_target, class RenderPass* render_pass = nullptr);
        void begin_rendering_base_pass(RenderTargetBase*, SceneLayer*);
        void begin_gbuffer_to_scene_output(RenderTargetBase*, SceneLayer*);
        void begin_deferred_lighting_pass(RenderTargetBase*, SceneLayer*);
        void begin_lighting_pass(RenderTargetBase*, SceneLayer*);
        void begin_scene_output_pass(RenderTargetBase*, SceneLayer*);
        void begin_postprocess_pass(RenderTargetBase*, SceneLayer*);
        void end_rendering_target(RenderTargetBase*, SceneLayer*);
        SceneRenderer& end_rendering_target();

        FORCE_INLINE const GlobalShaderParameters& shader_params() const
        {
            return m_global_shader_params;
        }

        FORCE_INLINE const SceneView& scene_view() const
        {
            return m_scene_view;
        }


        ~SceneRenderer();
    };

}// namespace Engine
