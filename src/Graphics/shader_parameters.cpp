#include <Core/base_engine.hpp>
#include <Core/math/math.hpp>
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
			camera.projection     = scene_view->projection();
			camera.view           = scene_view->view_matrix();
			camera.projview       = scene_view->projview();
			camera.inv_projection = Math::inverse(camera.projection);
			camera.inv_view       = Math::inverse(camera.view);
			camera.inv_projview   = scene_view->inv_projview();
			camera.location       = scene_view->camera_view().location;
			camera.near           = scene_view->camera_view().near;
			camera.far            = scene_view->camera_view().far;

			const auto& vp    = scene_view->viewport();
			viewport.pos      = Vector2f(vp.pos);
			viewport.size     = Vector2f(vp.size);
			viewport.inv_size = 1.f / Vector2f(vp.size);

			viewport.min_depth = vp.min_depth;
			viewport.max_depth = vp.max_depth;

			render_target.size     = Vector2f(target_size);
			render_target.inv_size = 1.f / Vector2f(target_size);
		}

		time       = engine_instance->time_seconds();
		delta_time = engine_instance->delta_time();
		return *this;
	}
}// namespace Engine
