#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class Archive;


    class ENGINE_EXPORT Transform
    {
    public:
        Matrix4f local_to_world = Matrix4f(1.f);

        Vector3D rotation = Vector3D(0.f);
        Vector3D location = Vector3D(0.0f);
        Vector3D scale    = Vector3D(1.0f);

    private:
        Vector3D vector_of(const Vector3D& dir, bool is_global) const;

    public:
        Matrix4f translation_matrix() const;
        Matrix4f rotation_matrix() const;
        Matrix4f scale_matrix() const;
        Transform& add_location(const Vector3D& delta);
        Transform& add_rotation(const Vector3D& delta);
        Transform& add_rotation(const Quaternion& delta);
        Transform& add_scale(const Vector3D& delta);
        Quaternion quaternion_rotation() const;

        Matrix4f matrix() const;
        Matrix4f world_to_local();
        Vector3D forward_vector(bool global = false) const;
        Vector3D right_vector(bool global = false) const;
        Vector3D up_vector(bool global = false) const;
        Vector3D global_location() const;

        Transform& update(class SceneComponent* parent_component = nullptr);

        String as_string() const;
    };

    bool operator&(Archive& ar, Transform& t);

}// namespace Engine
