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


    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        class Scene* m_scene;

        GlobalShaderParameters m_global_shader_params;
        SceneView m_scene_view;
        Color3 m_ambient_light;
        ViewMode m_view_mode;

        SceneRenderer& setup_parameters(RenderTargetBase* render_target = nullptr, SceneView* scene_view = nullptr);

        void copy_gbuffer_to_scene_output();

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(Scene* scene);
        Scene* scene() const;
        SceneRenderer& render(const SceneView& view, RenderTargetBase* render_target);

        void clear_render_targets(RenderTargetBase*, SceneLayer*);

        SceneRenderer& begin_rendering_target(RenderTargetBase* render_target, class RenderPass* render_pass = nullptr);
        void begin_rendering_base_pass(RenderTargetBase*, SceneLayer*);
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

        FORCE_INLINE ViewMode view_mode() const
        {
            return m_view_mode;
        }

        FORCE_INLINE const Color3& ambient_light() const
        {
            return m_ambient_light;
        }

        SceneRenderer& view_mode(ViewMode new_mode);
        SceneRenderer& ambient_light(const Color3& new_mode);

        ~SceneRenderer();
    };

}// namespace Engine
