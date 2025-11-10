#include <Core/math/math.hpp>
#include <Engine/camera_view.hpp>

namespace Engine
{
	static inline void static_initialize(CameraView& view, const Vector3f& origin, const Vector3f& forward, const Vector3f& up,
	                                     float near, float far)
	{
		view.view     = Math::look_at(origin, origin + forward, up);
		view.projview = view.projection * view.view;

		view.inv_projection = Math::inverse(view.projection);
		view.inv_view       = Math::inverse(view.view);
		view.inv_projview   = Math::inverse(view.projview);

		view.near = near;
		view.far  = far;
	}

	CameraView CameraView::static_perspective(const Vector3f& origin, const Vector3f& forward, const Vector3f& up, float fov,
	                                          float aspect, float near, float far)
	{
		CameraView view;
		view.projection = Math::perspective(fov, aspect, near, far);
		static_initialize(view, origin, forward, up, near, far);
		return view;
	}

	CameraView CameraView::static_ortho(const Vector3f& origin, const Vector3f& forward, const Vector3f& up, float left,
	                                    float right, float top, float bottom, float near, float far)
	{
		CameraView view;
		view.projection = Math::ortho(left, right, top, bottom, near, far);
		static_initialize(view, origin, forward, up, near, far);
		return view;
	}

	CameraView& CameraView::perspective(float fov, float aspect, float near, float far)
	{
		this->near = near;
		this->far  = far;

		projection = Math::perspective(fov, aspect, near, far);
		projview   = projection * view;

		inv_projection = Math::inverse(projection);
		inv_projview   = Math::inverse(projview);

		return *this;
	}

	CameraView& CameraView::ortho(float left, float right, float top, float bottom, float near, float far)
	{
		this->near = near;
		this->far  = far;

		projection = Math::ortho(left, right, top, bottom, near, far);
		projview   = projection * view;

		inv_projection = Math::inverse(projection);
		inv_projview   = Math::inverse(projview);
		return *this;
	}

	CameraView& CameraView::look(const Vector3f& origin, const Vector3f& forward, const Vector3f& up)
	{
		view     = Math::look_at(origin, origin + forward, up);
		projview = projection * view;

		inv_view     = Math::inverse(view);
		inv_projview = Math::inverse(projview);

		return *this;
	}

	float CameraView::linearize_depth(float depth) const
	{
		return Math::linearize_depth(depth, near, far);
	}

	Vector3f CameraView::reconstruct_position_ndc(Vector2f ndc, float depth) const
	{
		Vector4f ndc_pos  = Vector4f(ndc.x, ndc.y, depth, 1.0f);
		Vector4f view_pos = inv_projview * ndc_pos;
		view_pos /= view_pos.w;
		return {view_pos.x, view_pos.y, view_pos.z};
	}

}// namespace Engine
