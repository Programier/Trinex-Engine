#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>

namespace Engine
{
    class Archive;

    class ENGINE_EXPORT Transform
    {
    public:
        enum RotationMethod : EnumerateType
        {
            XYZ = 0,
            XZY = 1,
            YXZ = 2,
            YZX = 3,
            ZXY = 4,
            ZYX = 5
        };


        Matrix4f local_to_world;

        Vector3D rotation = Vector3D(0.f, 0.f, 0.f);
        Vector3D location = Vector3D(0.0f);
        Vector3D scale    = Vector3D(1.0f);

        RotationMethod rotation_method = RotationMethod::XYZ;


    private:
        Vector3D vector_of(const Vector3D& dir, bool is_global) const;

    public:
        Matrix4f translation_matrix() const;
        Matrix4f rotation_matrix() const;
        Matrix4f scale_matrix() const;

        Matrix4f matrix() const;
        Matrix4f world_to_local();
        Vector3D forward_vector(bool global = false) const;
        Vector3D right_vector(bool global = false) const;
        Vector3D up_vector(bool global = false) const;

        Vector3D global_location() const;

        Transform& update(class SceneComponent* scene_component, bool is_parent = false);
        Transform& update();

        String as_string() const;
    };

    bool operator&(Archive& ar, Transform& t);

}// namespace Engine
