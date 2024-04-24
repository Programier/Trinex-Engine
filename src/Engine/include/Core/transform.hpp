#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>
#include <Core/implement.hpp>

namespace Engine
{
    class Archive;


    class ENGINE_EXPORT Transform
    {
    public:
        static const Transform transform_zero;

    private:
        mutable Matrix4f m_matrix;
        Vector3D m_location;
        Vector3D m_rotation;
        Vector3D m_scale;
        mutable bool m_is_dirty;

    private:
        Vector3D vector_of(const Vector3D& dir) const;

    public:
        Transform(const Vector3D& location = Vector3D(0.0f), const Vector3D& rotation = Vector3D(0.f),
                  const Vector3D& scale = Vector3D(1.0f));
        Transform(const Vector3D& location, const Quaternion& rotation, const Vector3D& scale = Vector3D(1.0f));
        Transform(const Matrix4f& matrix);

        copy_constructors_hpp(Transform);
        Transform& operator=(const Matrix4f& matrix);

        const Matrix4f& matrix() const;
        Matrix4f translation_matrix() const;
        Matrix4f rotation_matrix() const;
        Matrix4f scale_matrix() const;

        const Vector3D& location() const;
        const Vector3D& rotation() const;
        const Vector3D& scale() const;
        Quaternion quaternion() const;

        static Quaternion angles_to_quaternion(const Vector3D& angles);
        static Vector3D quaternion_to_angles(const Quaternion& quat);

        Transform& location(const Vector3D&);
        Transform& rotation(const Quaternion&);
        Transform& rotation(const Vector3D&);
        Transform& scale(const Vector3D&);

        Transform& add_location(const Vector3D& delta);
        Transform& add_rotation(const Vector3D& delta);
        Transform& add_rotation(const Quaternion& delta);
        Transform& add_scale(const Vector3D& delta);
        Transform& look_at(const Vector3D& position, const Vector3D& up = {0.0f, 1.0f, 0.f});


        Transform& operator+=(const Transform&);
        Transform& operator-=(const Transform&);
        Transform operator+(const Transform&) const;
        Transform operator-(const Transform&) const;

        Transform& operator*=(const Transform&);
        Transform& operator/=(const Transform&);
        Transform operator*(const Transform&) const;
        Transform operator/(const Transform&) const;

        Vector3D forward_vector() const;
        Vector3D right_vector() const;
        Vector3D up_vector() const;

        String as_string() const;
        bool is_dirty() const;
        const Transform& mark_dirty() const;
        friend bool operator&(Archive& ar, Transform& t);
    };

    bool operator&(Archive& ar, Transform& t);

}// namespace Engine
