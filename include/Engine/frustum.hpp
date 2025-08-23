#pragma once
#include <Core/engine_types.hpp>
#include <Core/math/vector.hpp>

namespace Engine
{
	struct CameraView;

	class ENGINE_EXPORT Plane
	{
	private:
		Vector3f m_normal;
		float m_distance;

	public:
		Plane();
		Plane(const Vector3f& normal, float distance);
		Plane(const Vector3f& normal, const Vector3f& location);

		inline const Vector3f& normal() const { return m_normal; }
		inline float distance() const { return m_distance; }
		inline Vector3f location() const { return m_normal * m_distance; }

		float distance_to(const Vector3f& point) const;
		float distance_to(const Box3f& box) const;
	};

	struct ENGINE_EXPORT Frustum {
		Plane top;
		Plane bottom;
		Plane right;
		Plane left;
		Plane far;
		Plane near;

		Frustum();
		Frustum(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float fov, float near, float far,
		        float aspect);
		Frustum(const CameraView& camera);
		Frustum& operator=(const CameraView& view);
		Frustum& initialize(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float fov, float near,
		                    float far, float aspect_ratio);
		bool in_frustum(const Box3f& box) const;
	};
}// namespace Engine
