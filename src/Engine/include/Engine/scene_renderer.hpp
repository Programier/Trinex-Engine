#pragma once
#include <Core/engine_types.hpp>
#include <Core/implement.hpp>
#include <Core/name.hpp>
#include <Engine/camera_types.hpp>

namespace Engine
{
    class SceneRenderer;
    class RenderViewport;
    class SceneLayer;


    class ENGINE_EXPORT SceneRenderer final
    {
    private:
        class Scene* _M_scene;

        CameraView _M_camera_view;
        Matrix4f _M_projection;
        Matrix4f _M_view;
        Matrix4f _M_projview;
        Matrix4f _M_inv_projview;
        Size2D _M_size;


    public:
        SceneRenderer();
        delete_copy_constructors(SceneRenderer);

        SceneRenderer& scene(Scene* scene);
        Scene* scene() const;
        SceneRenderer& render(const CameraView& view, RenderViewport* viewport, const Size2D& size);

        const SceneRenderer& screen_to_world(const Vector2D& screen_point, Vector3D& world_origin,
                                             Vector3D& world_direction) const;
        Vector4D world_to_screen(const Vector3D& world_point) const;

        void clear_render_targets(RenderViewport*, SceneLayer*);


        FORCE_INLINE const Matrix4f& view_matrix() const
        {
            return _M_view;
        }

        FORCE_INLINE const Matrix4f& projection_matrix() const
        {
            return _M_projection;
        }

        FORCE_INLINE const Matrix4f& projview_matrix() const
        {
            return _M_projview;
        }

        FORCE_INLINE const Matrix4f& inv_projview_matrix() const
        {
            return _M_inv_projview;
        }

        FORCE_INLINE const CameraView& camera_view() const
        {
            return _M_camera_view;
        }

        FORCE_INLINE const Size2D& view_size() const
        {
            return _M_size;
        }

        ~SceneRenderer();
    };

}// namespace Engine
