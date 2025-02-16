#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>

namespace Engine
{
	class Archive;


	class ENGINE_EXPORT Transform
	{
	public:
		declare_struct(Transform, void);
		static const Transform transform_zero;

	private:
		mutable Matrix4f m_matrix;
		Vector3f m_location;
		Vector3f m_rotation;
		Vector3f m_scale;
		mutable bool m_is_dirty;

	private:
		Vector3f vector_of(const Vector3f& dir) const;

	public:
		Transform(const Vector3f& location = Vector3f(0.0f), const Vector3f& rotation = Vector3f(0.f),
		          const Vector3f& scale = Vector3f(1.0f));
		Transform(const Vector3f& location, const Quaternion& rotation, const Vector3f& scale = Vector3f(1.0f));
		Transform(const Matrix4f& matrix);

		copy_constructors_hpp(Transform);
		Transform& operator=(const Matrix4f& matrix);

		const Matrix4f& matrix() const;
		Matrix4f translation_matrix() const;
		Matrix4f rotation_matrix() const;
		Matrix4f scale_matrix() const;

		const Vector3f& location() const;
		const Vector3f& rotation() const;
		const Vector3f& scale() const;
		Quaternion quaternion() const;

		static Quaternion angles_to_quaternion(const Vector3f& angles);
		static Vector3f quaternion_to_angles(const Quaternion& quat);

		Transform& location(const Vector3f&);
		Transform& rotation(const Quaternion&);
		Transform& rotation(const Vector3f&);
		Transform& scale(const Vector3f&);

		Transform& add_location(const Vector3f& delta);
		Transform& add_rotation(const Vector3f& delta);
		Transform& add_rotation(const Quaternion& delta);
		Transform& add_scale(const Vector3f& delta);
		Transform& look_at(const Vector3f& position, const Vector3f& up = {0.0f, 1.0f, 0.f});


		Transform& operator+=(const Transform&);
		Transform& operator-=(const Transform&);
		Transform operator+(const Transform&) const;
		Transform operator-(const Transform&) const;

		Transform& operator*=(const Transform&);
		Transform& operator/=(const Transform&);
		Transform operator*(const Transform&) const;
		Transform operator/(const Transform&) const;

		Vector3f forward_vector() const;
		Vector3f right_vector() const;
		Vector3f up_vector() const;

		String as_string() const;
		bool is_dirty() const;
		const Transform& mark_dirty() const;

		bool serialize(Archive& ar);
	};
}// namespace Engine
