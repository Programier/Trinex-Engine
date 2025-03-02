#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
	static bool is_support_lighting_pass(const Material* material)
	{
		if (material->domain == MaterialDomain::Lighting)
			return true;

		return false;
	}

	trinex_impl_render_pass(Engine::ShadowPass) {}

	trinex_impl_render_pass(Engine::ShadowedLightingPass)
	{
		m_attachments_count = 1;

		m_shader_definitions = {
				{"TRINEX_SHADOWED_LIGHTING_PASS", "1"},
		};

		m_is_material_compatible = is_support_lighting_pass;
	}

	trinex_impl_render_pass(Engine::LightingPass)
	{
		m_attachments_count = 1;

		m_shader_definitions = {
				{"TRINEX_LIGHTING_PASS", "1"},
		};

		m_is_material_compatible = is_support_lighting_pass;
	}

	trinex_impl_render_pass(Engine::DeferredLightingPass) {}

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
		view.fov             = glm::degrees(component->outer_cone_angle()) * 2.f;
		view.near_clip_plane = 1.f;
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

	DeferredLightingPass& DeferredLightingPass::initialize()
	{
		Super::initialize();
		m_shadowed_lighting_pass = create_subpass<ShadowedLightingPass>();
		m_lighting_pass          = create_subpass<LightingPass>();
		return *this;
	}

	inline RenderPass* DeferredLightingPass::find_render_pass(LightComponentProxy* light)
	{
		if (light->is_shadows_enabled())
			return m_shadowed_lighting_pass;
		return m_lighting_pass;
	}

	bool DeferredLightingPass::is_empty() const
	{
		return is_not_in<ViewMode::Lit>(scene_renderer()->view_mode());
	}

	DeferredLightingPass& DeferredLightingPass::render(RenderViewport* vp)
	{
		SceneRenderTargets::instance()->bind_scene_color_ldr(false);

		auto renderer = scene_renderer();

		if (renderer->view_mode() == ViewMode::Unlit)
		{
		}
		else if (renderer->view_mode() == ViewMode::Lit)
		{
			static Name name_ambient_color = "ambient_color";
			Material* material             = DefaultResources::Materials::ambient_light;

			if (material)
			{
				auto ambient_param =
						Object::instance_cast<MaterialParameters::Float3>(material->find_parameter(name_ambient_color));

				if (ambient_param)
				{
					ambient_param->value = renderer->scene->environment.ambient_color;
				}

				material->apply(nullptr, this);
				rhi->draw(6, 0);
			}
		}

		Super::render(vp);
		return *this;
	}

#define lighting_param(param_name, type)                                                                                         \
	reinterpret_cast<MaterialParameters::type*>(material->find_parameter(LightComponent::name_##param_name));

	DeferredLightingPass& DeferredLightingPass::add_light(SpotLightComponent* spotlight)
	{
		auto proxy       = spotlight->proxy();
		RenderPass* pass = find_render_pass(proxy);

		pass->add_callabble([spotlight, pass]() {
			auto proxy         = spotlight->proxy();
			Material* material = DefaultResources::Materials::spot_light;

			auto* color_parameter           = lighting_param(color, Float3);
			auto* intensivity_parameter     = lighting_param(intensivity, Float);
			auto* spot_angles_parameter     = lighting_param(spot_angles, Float2);
			auto* location_parameter        = lighting_param(location, Float3);
			auto* direction_parameter       = lighting_param(direction, Float3);
			auto* radius_parameter          = lighting_param(radius, Float);
			auto* fall_off_parameter        = lighting_param(fall_off_exponent, Float);
			auto* shadow_map_parameter      = lighting_param(shadow_map_texture, Sampler2D);
			auto* shadow_projview_parameter = lighting_param(shadow_map_projview, Float4x4);
			auto* depth_bias_parameter      = lighting_param(depth_bias, Float);
			auto* slope_scale_parameter     = lighting_param(slope_scale, Float);

			color_parameter->value        = proxy->light_color();
			intensivity_parameter->value  = proxy->intensivity();
			location_parameter->value     = proxy->world_transform().location();
			direction_parameter->value    = proxy->direction();
			spot_angles_parameter->value  = Vector2f(proxy->cos_outer_cone_angle(), proxy->inv_cos_cone_difference());
			radius_parameter->value       = proxy->attenuation_radius();
			fall_off_parameter->value     = proxy->fall_off_exponent();

			if (proxy->is_shadows_enabled())
			{
				float shadow_map_size        = proxy->shadow_map()->width();
				depth_bias_parameter->value  = proxy->depth_bias() / shadow_map_size;
				slope_scale_parameter->value = proxy->slope_scale() / shadow_map_size;
				shadow_map_parameter->sampler = DefaultResources::Samplers::default_sampler;
				shadow_map_parameter->texture = proxy->shadow_map();

				auto view                        = camera_view(proxy);
				shadow_projview_parameter->value = view.projection_matrix() * view.view_matrix();
			}

			if (material->apply(spotlight, pass))
			{
				DefaultResources::Buffers::screen_quad->rhi_bind(0, 0);
				rhi->draw(6, 0);
			}
		});
		return *this;
	}

	DeferredLightingPass& DeferredLightingPass::add_light(PointLightComponent* spotlight)
	{
		return *this;
	}

	DeferredLightingPass& DeferredLightingPass::add_light(DirectionalLightComponent* spotlight)
	{
		return *this;
	}
}// namespace Engine
