#include <Core/constants.hpp>
#include <Engine/aabb.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/frustum.hpp>

namespace Engine
{
	Plane::Plane() : normal(Constants::zero_vector), distance(0.f)
	{}

	Plane::Plane(const Vector3D& _p1, const Vector3D& _normal) : normal(glm::normalize(_normal)), distance(glm::dot(normal, _p1))
	{}

	float Plane::signed_distance_to_plane(const Point3D& point) const
	{
		return glm::dot(normal, point) - distance;
	}

	bool Plane::is_on_or_forward(const Point3D& point) const
	{
		return signed_distance_to_plane(point) >= 0.f;
	}

	bool Plane::is_on_or_forward(const AABB_3Df& box) const
	{
		const auto extents = box.extents();
		const float r      = extents.x * std::abs(normal.x) + extents.y * std::abs(normal.y) + extents.z * std::abs(normal.z);
		return -r <= signed_distance_to_plane(box.center());
	}

	Frustum::Frustum(const CameraView& camera)
	{
		*this = camera;
	}

	Frustum& Frustum::operator=(const CameraView& view)
	{
		const float half_v_side = view.far_clip_plane * glm::tan(glm::radians(view.fov) * 0.5f);
		const float half_h_side = half_v_side * view.aspect_ratio;

		const Vector3D front_mult_far = view.far_clip_plane * view.forward_vector;

		near = {view.location + view.near_clip_plane * view.forward_vector, view.forward_vector};
		far  = {view.location + front_mult_far, -view.forward_vector};

		right  = {view.location, glm::cross(view.up_vector, front_mult_far + view.right_vector * half_h_side)};
		left   = {view.location, glm::cross(front_mult_far - view.right_vector * half_h_side, view.up_vector)};
		top    = {view.location, glm::cross(view.right_vector, front_mult_far - view.up_vector * half_v_side)};
		bottom = {view.location, glm::cross(front_mult_far + view.up_vector * half_v_side, view.right_vector)};

		return *this;
	}

	bool Frustum::in_frustum(const AABB_3Df& box) const
	{
		bool r1 = left.is_on_or_forward(box);
		bool r2 = right.is_on_or_forward(box);
		bool r3 = top.is_on_or_forward(box);
		bool r4 = bottom.is_on_or_forward(box);
		bool r5 = near.is_on_or_forward(box);
		bool r6 = far.is_on_or_forward(box);

		return r1 && r2 && r3 && r4 && r5 && r6;
	}
}// namespace Engine
