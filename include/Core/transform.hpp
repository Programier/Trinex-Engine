#pragma once
#include <Core/callback.hpp>
#include <Core/constants.hpp>
#include <Core/math/quaternion.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	class Archive;

	struct ENGINE_EXPORT Transform {
		trinex_struct(Transform, void);

	private:
		Vector3f vector_of(const Vector3f& dir) const;

	public:
		Vector3f location;
		Quaternion rotation;
		Vector3f scale;

		Transform(const Vector3f& location = Vector3f(0.0f), const Quaternion& rotation = Quaternion(1.f, 0.f, 0.f, 0.f),
		          const Vector3f& scale = Vector3f(1.0f));
		Transform(const Matrix4f& matrix);

		trinex_default_copyable(Transform);
		trinex_default_moveable(Transform);

		Transform& operator=(const Matrix4f& matrix);

		Matrix4f matrix() const;
		Matrix4f translation_matrix() const;
		Matrix4f rotation_matrix() const;
		Matrix4f scale_matrix() const;

		static Quaternion angles_to_quaternion(const Vector3f& angles);
		static Vector3f quaternion_to_angles(const Quaternion& quat);

		Transform& look_at(const Vector3f& position, const Vector3f& up = {0.0f, 1.0f, 0.f});

		Transform& operator+=(const Transform&);
		Transform& operator-=(const Transform&);
		Transform operator+(const Transform&) const;
		Transform operator-(const Transform&) const;

		Vector3f forward_vector() const;
		Vector3f right_vector() const;
		Vector3f up_vector() const;

		String as_string() const;
		bool serialize(Archive& ar);
	};
}// namespace Engine
