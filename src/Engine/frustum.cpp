#include <Core/constants.hpp>
#include <Engine/aabb.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/frustum.hpp>

namespace Engine
{
	Plane::Plane() : m_normal(Constants::zero_vector), m_distance(0.f) {}
	Plane::Plane(const Vector3f& normal, float distance) : m_normal(glm::normalize(normal)), m_distance(distance) {}
	Plane::Plane(const Vector3f& normal, const Vector3f& location)
	    : m_normal(glm::normalize(normal)), m_distance(glm::dot(m_normal, location))
	{}

	float Plane::distance_to(const Vector3f& point) const
	{
		return glm::dot(m_normal, point) - m_distance;
	}

	float Plane::distance_to(const AABB_3Df& box) const
	{
		Vector3f extents     = box.extents();
		const float distance = distance_to(box.center());
		const float r = extents.x * std::abs(m_normal.x) + extents.y * std::abs(m_normal.y) + extents.z * std::abs(m_normal.z);
		return distance + r;
	}

	Frustum::Frustum(const CameraView& camera)
	{
		*this = camera;
	}

	Frustum& Frustum::operator=(const CameraView& view)
	{
		const float half_v_side = view.far_clip_plane * glm::tan(glm::radians(view.fov) * 0.5f);
		const float half_h_side = half_v_side * view.aspect_ratio;

		const Vector3f front_mult_far = view.far_clip_plane * view.forward_vector;

		near = Plane(view.forward_vector, view.location + view.near_clip_plane * view.forward_vector);
		far  = Plane(-view.forward_vector, view.location + front_mult_far);

		right  = Plane(glm::cross(view.up_vector, front_mult_far + view.right_vector * half_h_side), view.location);
		left   = Plane(glm::cross(front_mult_far - view.right_vector * half_h_side, view.up_vector), view.location);
		top    = Plane(glm::cross(view.right_vector, front_mult_far - view.up_vector * half_v_side), view.location);
		bottom = Plane(glm::cross(front_mult_far + view.up_vector * half_v_side, view.right_vector), view.location);

		return *this;
	}

	bool Frustum::in_frustum(const AABB_3Df& box) const
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (plane->distance_to(box) < 0.f)
				return false;
		}

		return true;
	}
}// namespace Engine
