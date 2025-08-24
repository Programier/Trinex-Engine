#include <Core/constants.hpp>
#include <Core/math/box.hpp>
#include <Core/math/math.hpp>
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

	float Plane::distance_to(const Box3f& box) const
	{
		Vector3f extents     = box.extents();
		const float distance = distance_to(box.center());
		const float r = extents.x * std::abs(m_normal.x) + extents.y * std::abs(m_normal.y) + extents.z * std::abs(m_normal.z);
		return distance + r;
	}

	Frustum::Frustum() = default;

	Frustum::Frustum(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float fov, float near, float far,
	                 float aspect)
	{
		initialize(location, forward, up, fov, near, far, aspect);
	}

	Frustum::Frustum(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float left, float right, float top,
	                 float bottom, float near, float far)
	{
		initialize(location, forward, up, left, right, top, bottom, near, far);
	}

	Frustum::Frustum(const CameraView& camera)
	{
		*this = camera;
	}

	Frustum& Frustum::operator=(const CameraView& view)
	{
		if (view.projection_mode == CameraProjectionMode::Perspective)
			return initialize(view.location, view.forward, view.up, Math::radians(view.perspective.fov), view.near, view.far,
			                  view.perspective.aspect_ratio);

		return initialize(view.location, view.forward, view.up, view.ortho.left, view.ortho.right, view.ortho.top,
		                  view.ortho.bottom, view.near, view.far);
	}

	Frustum& Frustum::initialize(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float fov, float near,
	                             float far, float aspect_ratio)
	{
		const float half_v_side = far * Math::tan(fov * 0.5f);
		const float half_h_side = half_v_side * aspect_ratio;

		const Vector3f front_mult_far = far * forward;
		const Vector3f right          = Math::cross(forward, up);

		this->near   = Plane(forward, location + near * forward);
		this->far    = Plane(-forward, location + front_mult_far);
		this->right  = Plane(Math::cross(up, front_mult_far + right * half_h_side), location);
		this->left   = Plane(Math::cross(front_mult_far - right * half_h_side, up), location);
		this->top    = Plane(Math::cross(right, front_mult_far - up * half_v_side), location);
		this->bottom = Plane(Math::cross(front_mult_far + up * half_v_side, right), location);

		return *this;
	}

	Frustum& Frustum::initialize(const Vector3f& location, const Vector3f& forward, const Vector3f& up, float left, float right,
	                             float top, float bottom, float near, float far)
	{
		const Vector3f right_vector = Math::cross(forward, up);

		const Vector3f near_center = location + near * forward;
		const Vector3f far_center  = location + far * forward;

		this->near   = Plane(forward, near_center);
		this->far    = Plane(-forward, far_center);
		this->right  = Plane(-right_vector, location + right_vector * right);
		this->left   = Plane(right_vector, location + right_vector * left);
		this->top    = Plane(-up, location + up * top);
		this->bottom = Plane(up, location + up * bottom);

		return *this;
	}

	bool Frustum::contains(const Box3f& box) const
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (plane->distance_to(box) < 0.f)
				return false;
		}

		return true;
	}

	bool Frustum::contains(const Vector3f& point)
	{
		for (const Plane* plane : {&left, &right, &top, &bottom, &near, &far})
		{
			if (plane->distance_to(point) < 0.f)
				return false;
		}

		return true;
	}
}// namespace Engine
