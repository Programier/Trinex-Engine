#include <Engine/scene_view.hpp>

namespace Engine
{

	SceneView::SceneView(ShowFlags show_flags) : m_show_flags(show_flags)
	{}

	SceneView::SceneView(const CameraView& view, const Size2D& view_size, ShowFlags show_flags)
	    : m_camera_view(view), m_projection(view.projection_matrix()), m_view(view.view_matrix()), m_show_flags(show_flags)
	{
		m_viewport.pos       = {0, 0};
		m_viewport.size      = view_size;
		m_viewport.min_depth = 0.f;
		m_viewport.max_depth = 1.f;
		m_scissor.pos        = {0, 0};
		m_scissor.size       = view_size;

		m_projview     = m_projection * m_view;
		m_inv_projview = glm::inverse(m_projview);
	}

	SceneView::SceneView(const CameraView& view, const ViewPort& viewport, const Scissor& scissor, ShowFlags show_flags)
	    : m_camera_view(view), m_projection(view.projection_matrix()), m_view(view.view_matrix()), m_viewport(viewport),
	      m_scissor(scissor), m_show_flags(show_flags)
	{
		m_projview     = m_projection * m_view;
		m_inv_projview = glm::inverse(m_projview);
	}

	default_copy_constructors_cpp(SceneView);

	SceneView& SceneView::camera_view(const CameraView& view)
	{
		m_camera_view  = view;
		m_projection   = view.projection_matrix();
		m_view         = view.view_matrix();
		m_projview     = m_projection * m_view;
		m_inv_projview = glm::inverse(m_projview);
		return *this;
	}

	SceneView& SceneView::viewport(const ViewPort& viewport)
	{
		m_viewport = viewport;
		return *this;
	}

	SceneView& SceneView::scissor(const Scissor& scissor)
	{
		m_scissor = scissor;
		return *this;
	}

	SceneView& SceneView::show_flags(ShowFlags flags)
	{
		m_show_flags = flags;
		return *this;
	}

	const SceneView& SceneView::screen_to_world(const Vector2f& screen_point, Vector3f& world_origin,
	                                            Vector3f& world_direction) const
	{
		int32_t x = glm::trunc(screen_point.x), y = glm::trunc(screen_point.y);

		Matrix4f inverse_view       = glm::inverse(m_view);
		Matrix4f inverse_projection = glm::inverse(m_projection);

		Size2D m_size = view_size();

		float screen_space_x                = (x - m_size.x / 2.f) / (m_size.x / 2.f);
		float screen_space_y                = (y - m_size.y / 2.f) / (m_size.y / 2.f);
		Vector4f ray_start_projection_space = Vector4f(screen_space_x, screen_space_y, 0.f, 1.0f);
		Vector4f ray_end_projection_space   = Vector4f(screen_space_x, screen_space_y, 0.5f, 1.0f);

		Vector4f hg_ray_start_view_space = inverse_projection * ray_start_projection_space;
		Vector4f hg_ray_end_view_space   = inverse_projection * ray_end_projection_space;

		Vector3f ray_start_view_space(hg_ray_start_view_space.x, hg_ray_start_view_space.y, hg_ray_start_view_space.z);
		Vector3f ray_end_view_space(hg_ray_end_view_space.x, hg_ray_end_view_space.y, hg_ray_end_view_space.z);

		if (hg_ray_start_view_space.w != 0.0f)
		{
			ray_start_view_space /= hg_ray_start_view_space.w;
		}
		if (hg_ray_end_view_space.w != 0.0f)
		{
			ray_end_view_space /= hg_ray_end_view_space.w;
		}

		Vector3f ray_dir_view_space = ray_end_view_space - ray_start_view_space;
		ray_dir_view_space          = glm::normalize(ray_dir_view_space);

		Vector3f ray_dir_world_space = inverse_view * Vector4f(ray_dir_view_space, 0.f);

		world_origin    = m_camera_view.location;
		world_direction = glm::normalize(ray_dir_world_space);
		return *this;
	}

	Vector4f SceneView::world_to_screen(const Vector3f& world_point) const
	{
		return m_projview * Vector4f(world_point, 1.f);
	}

}// namespace Engine
