#pragma once
#include <Core/enums.hpp>
#include <Core/flags.hpp>
#include <Core/implement.hpp>
#include <Engine/camera_types.hpp>

namespace Engine
{
    class ENGINE_EXPORT SceneView
    {
    private:
        CameraView m_camera_view;
        Matrix4f m_projection;
        Matrix4f m_view;
        Matrix4f m_projview;
        Matrix4f m_inv_projview;
        Size2D m_size;
        Flags<ShowFlags, BitMask> m_show_flags;

    public:
        SceneView(const Flags<ShowFlags, BitMask>& show_flags = ShowFlags::DefaultFlags);
        SceneView(const CameraView& view, const Size2D& size,
                  const Flags<ShowFlags, BitMask>& show_flags = ShowFlags::DefaultFlags);
        copy_constructors_hpp(SceneView);

    public:
        SceneView& camera_view(const CameraView& view);
        SceneView& view_size(const Size2D& size);
        SceneView& show_flags(const Flags<ShowFlags, BitMask>& flags);
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

        FORCE_INLINE const Flags<ShowFlags, BitMask>& show_flags() const
        {
            return m_show_flags;
        }
    };
}// namespace Engine
