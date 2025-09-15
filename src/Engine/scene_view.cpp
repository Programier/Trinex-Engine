#include <Core/math/math.hpp>
#include <Engine/scene_view.hpp>

namespace Engine
{
	SceneView::SceneView(ShowFlags show_flags) : m_show_flags(show_flags) {}

	SceneView::SceneView(const CameraView& view, const Vector2u& view_size, ShowFlags show_flags)
	    : SceneView(view, RHIViewport(view_size), RHIScissors(view_size), show_flags)
	{}

	SceneView::SceneView(const CameraView& view, const RHIViewport& viewport, const RHIScissors& scissor, ShowFlags show_flags)
	    : m_camera_view(view),                                    //
	      m_projection(view.projection_matrix(viewport.aspect())),//
	      m_view(view.view_matrix()),                             //
	      m_viewport(viewport),                                   //
	      m_scissor(scissor),                                     //
	      m_show_flags(show_flags)
	{
		m_projview     = m_projection * m_view;
		m_inv_projview = Math::inverse(m_projview);
	}

	default_copy_constructors_cpp(SceneView);

	SceneView& SceneView::camera_view(const CameraView& view)
	{
		m_camera_view = view;

		m_projection   = view.projection_matrix(m_viewport.aspect());
		m_view         = view.view_matrix();
		m_projview     = m_projection * m_view;
		m_inv_projview = Math::inverse(m_projview);
		return *this;
	}

	SceneView& SceneView::viewport(const RHIViewport& viewport)
	{
		m_viewport = viewport;

		m_projection   = m_camera_view.projection_matrix(m_viewport.aspect());
		m_projview     = m_projection * m_view;
		m_inv_projview = Math::inverse(m_projview);
		return *this;
	}

	SceneView& SceneView::scissor(const RHIScissors& scissor)
	{
		m_scissor = scissor;
		return *this;
	}

	SceneView& SceneView::transform(const Matrix4f& transform)
	{
		m_projection   = transform * m_projection;
		m_projview     = m_projection * m_view;
		m_inv_projview = Math::inverse(m_projview);
		return *this;
	}

	SceneView& SceneView::show_flags(ShowFlags flags)
	{
		m_show_flags = flags;
		return *this;
	}

	Vector3f SceneView::screen_to_ray_direction(const Vector2f& screen_point) const
	{
		int32_t x = Math::trunc(screen_point.x), y = Math::trunc(screen_point.y);
		Vector2f m_size = view_size();

		float u = (x - m_size.x / 2.f) / (m_size.x / 2.f);
		float v = (y - m_size.y / 2.f) / (m_size.y / 2.f);

		return uv_to_ray_direction({u, v});
	}

	Vector3f SceneView::uv_to_ray_direction(const Vector2f& uv) const
	{
		Matrix4f inverse_view       = Math::inverse(m_view);
		Matrix4f inverse_projection = Math::inverse(m_projection);

		Vector2f ndc                        = uv * 2.0f - Vector2f(1.0f);
		Vector4f ray_start_projection_space = Vector4f(ndc, 0.f, 1.0f);
		Vector4f ray_end_projection_space   = Vector4f(ndc, 0.5f, 1.0f);

		Vector4f hg_ray_start_view_space = inverse_projection * ray_start_projection_space;
		Vector4f hg_ray_end_view_space   = inverse_projection * ray_end_projection_space;

		Vector3f ray_start_view_space(hg_ray_start_view_space.x, hg_ray_start_view_space.y, hg_ray_start_view_space.z);
		Vector3f ray_end_view_space(hg_ray_end_view_space.x, hg_ray_end_view_space.y, hg_ray_end_view_space.z);

		if (hg_ray_start_view_space.w != 0.0f)
			ray_start_view_space /= hg_ray_start_view_space.w;

		if (hg_ray_end_view_space.w != 0.0f)
			ray_end_view_space /= hg_ray_end_view_space.w;

		return Math::normalize(inverse_view * Vector4f(Math::normalize(ray_end_view_space - ray_start_view_space), 0.f));
	}

	Vector3f SceneView::world_to_screen(const Vector3f& world) const
	{
		Vector4f result = m_projview * Vector4f(world, 1.f);
		return Vector3f(result.x / result.w, result.y / result.w, result.z / result.w);
	}

	Vector3f SceneView::screen_to_world(const Vector3f& screen) const
	{
		Vector4f result = m_inv_projview * Vector4f(screen, 1.f);
		return Vector3f(result.x / result.w, result.y / result.w, result.z / result.w);
	}

}// namespace Engine
