#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Engine/camera_types.hpp>
#include <Graphics/shader_parameters.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;
    class SceneLayer;


    class ENGINE_EXPORT SceneView
    {
    private:
        CameraView m_camera_view;
        Matrix4f m_projection;
        Matrix4f m_view;
        Matrix4f m_projview;
        Matrix4f m_inv_projview;
        Size2D m_size;

    public:
        SceneView();
        SceneView(const CameraView& view, const Size2D& size);
        copy_constructors_hpp(SceneView);

    public:
        SceneView& camera_view(const CameraView& view);
        SceneView& view_size(const Size2D& size);
        const SceneView& screen_to_world(const Vector2D& screen_point, Vector3D& world_origin, Vector3D& world_direction) const;
        Vector4D world_to_screen(const Vector3D& world_point) const;

        FORCE_INLINE const Matrix4f& view_matrix() const
        {
            return m_view;
        }

        FORCE_INLINE const Matrix4f& projection_matrix() const
        {
            return m_projection;
        }

        FORCE_INLINE const Matrix4f& projview_matrix() const
        {
            return m_projview;
        }

        FORCE_INLINE const Matrix4f& inv_projview_matrix() const
        {
            return m_inv_projview;
        }

        FORCE_INLINE const CameraView& camera_view() const
        {
            return m_camera_view;
        }

        FORCE_INLINE const Size2D& view_size() const
        {
            return m_size;
        }
    };

    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        class Scene* m_scene;

        GlobalShaderParameters m_global_shader_params;
        SceneView m_scene_view;


        SceneRenderer& setup_viewport(RenderViewport* viewport);

    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(Scene* scene);
        Scene* scene() const;
        SceneRenderer& render(const SceneView& view, RenderViewport* viewport);

        void clear_render_targets(RenderViewport*, SceneLayer*);
        void begin_rendering_base_pass(RenderViewport*, SceneLayer*);
        void begin_lighting_pass(RenderViewport*, SceneLayer*);
        void begin_scene_output_pass(RenderViewport*, SceneLayer*);
        void begin_postprocess_pass(RenderViewport*, SceneLayer*);

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
