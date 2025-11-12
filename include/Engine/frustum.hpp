#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct CameraView;

	struct ENGINE_EXPORT Plane {
		Vector3f normal;
		float offset;

		Plane();
		Plane(const Vector4f& normal_distance);
		Plane(const Vector3f& normal, float distance);
		Plane(const Vector3f& normal, const Vector3f& location);

		Plane& normalize();
		float distance_to(const Vector3f& point) const;
		float distance_to(const Box3f& box) const;

		static Plane static_left(const Matrix4f& projview);
		static Plane static_right(const Matrix4f& projview);
		static Plane static_top(const Matrix4f& projview);
		static Plane static_bottom(const Matrix4f& projview);
		static Plane static_near(const Matrix4f& projview);
		static Plane static_far(const Matrix4f& projview);
	};

	struct ENGINE_EXPORT Frustum {
		enum ContaintmentType
		{
			Outside    = 0,
			Intersects = 1,
			Contains   = 2,
		};

		Plane left;
		Plane right;
		Plane top;
		Plane bottom;
		Plane near;
		Plane far;

		Frustum();
		Frustum(const Matrix4f& projview);
		Frustum& operator=(const Matrix4f& projview);

		bool contains(const Box3f& box) const;
		bool contains(const Vector3f& point);
		bool intersects(const Box3f& box) const;

		ContaintmentType containtment_type(const Box3f& box) const;
	};
}// namespace Engine
