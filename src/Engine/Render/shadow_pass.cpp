#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/Render/shadow_pass.hpp>
#include <Engine/settings.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_impl_render_pass(Engine::ShadowPass)
	{}


	static CameraView camera_view(SpotLightComponentProxy* component)
	{
		CameraView view;
		const Transform& transform = component->world_transform();

		view.location        = transform.location();
		view.rotation        = transform.rotation();
		view.forward_vector  = transform.forward_vector();
		view.up_vector       = transform.up_vector();
		view.right_vector    = transform.right_vector();
		view.projection_mode = CameraProjectionMode::Perspective;
		view.fov             = component->outer_cone_angle();
		view.near_clip_plane = 0.05f;
		view.far_clip_plane  = component->attenuation_radius();
		view.aspect_ratio    = 1.0f;

		return view;
	}

	bool ShadowPass::is_empty() const
	{
		return false;
	}

	ShadowPass& ShadowPass::render(RenderViewport* vp)
	{
		auto viewport = rhi->viewport();
		auto scissors = rhi->scissor();

		Super::render(vp);

		rhi->viewport(viewport);
		rhi->scissor(scissors);
		return *this;
	}

	ShadowPass& ShadowPass::add_light(DepthSceneRenderer* renderer, SpotLightComponent* component)
	{
		add_callabble([=, scene = scene_renderer()->scene]() {
			auto shadow_map = component->proxy()->shadow_map();

			shadow_map->rhi_clear_depth_stencil(1.0, 255);
			rhi->bind_depth_stencil_target(shadow_map);

			Vector2f size = {Settings::Rendering::shadow_map_size, Settings::Rendering::shadow_map_size};
			SceneView view(camera_view(component->proxy()), size);

			renderer->scene = scene;
			renderer->render(view, nullptr);
		});
		return *this;
	}
}// namespace Engine
