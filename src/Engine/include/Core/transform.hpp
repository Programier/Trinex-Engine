#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class Archive;

    class ENGINE_EXPORT Transform
    {
    public:
        Quaternion rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f);
        Vector3D location   = Vector3D(0.0f);
        Vector3D scale      = Vector3D(1.0f);

        Matrix4f local_to_world;

    public:
        Matrix4f matrix() const;
        Matrix4f world_to_local();
        Vector3D forward_vector() const;
        Vector3D right_vector() const;
        Vector3D up_vector() const;

        static Quaternion calc_rotation(const Vector3D& axis, float angle);
        static Quaternion calc_rotation(const Vector3D& angles);

        Transform& update(class SceneComponent* scene_component, bool is_parent = false);
        Transform& update();

        String as_string() const;
    };

    bool operator&(Archive& ar, Transform& t);

}// namespace Engine
