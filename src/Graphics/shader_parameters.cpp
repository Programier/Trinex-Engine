#include <Core/base_engine.hpp>
#include <Engine/ActorComponents/camera_component.hpp>
#include <Engine/camera_types.hpp>
#include <Engine/scene_view.hpp>
#include <Graphics/shader_parameters.hpp>
#include <RHI/rhi.hpp>


namespace Engine
{
	ENGINE_EXPORT GlobalShaderParameters& GlobalShaderParameters::update(const SceneView* scene_view, Vector2u target_size)
	{
		if (scene_view)
		{
			projection   = scene_view->projection_matrix();
			view         = scene_view->view_matrix();
			projview     = scene_view->projview_matrix();
			inv_projview = scene_view->inv_projview_matrix();

			const auto& vp    = scene_view->viewport();
			viewport.pos      = Vector2f(vp.pos);
			viewport.size     = Vector2f(vp.size);
			viewport.inv_size = 1.f / Vector2f(vp.size);

			viewport.min_depth = vp.min_depth;
			viewport.max_depth = vp.max_depth;

			render_target.size     = Vector2f(target_size);
			render_target.inv_size = 1.f / Vector2f(target_size);

			auto& camera_view = scene_view->camera_view();
			camera.location   = camera_view.location;
			camera.forward    = camera_view.forward_vector;
			camera.right      = camera_view.right_vector;
			camera.up         = camera_view.up_vector;

			camera.fov             = camera_view.fov;
			camera.ortho_width     = camera_view.ortho_width;
			camera.ortho_height    = camera_view.ortho_height;
			camera.near            = camera_view.near_clip_plane;
			camera.far             = camera_view.far_clip_plane;
			camera.aspect_ratio    = camera_view.aspect_ratio;
			camera.projection_mode = static_cast<Camera::Projection>(camera_view.projection_mode);
		}

		time       = engine_instance->time_seconds();
		delta_time = engine_instance->delta_time();
		return *this;
	}
}// namespace Engine
