#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/pipelines.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Engine/settings.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/render_surface.hpp>
#include <Graphics/rhi.hpp>
#include <Graphics/sampler.hpp>
#include <Graphics/scene_render_targets.hpp>

namespace Engine
{
	static bool is_support_lighting_pass(const Material* material)
	{
		if (material->domain == MaterialDomain::Lighting)
			return true;

		return false;
	}

	static inline void bind_surface(RenderSurface* surface, byte location)
	{
		surface->rhi_shader_resource_view()->bind_combined(location, DefaultResources::Samplers::default_sampler->rhi_sampler());
	}

	static inline void bind_scene_render_target(SceneRenderTargets* rt, SceneRenderTargets::Surface surface, byte location)
	{
		bind_surface(rt->surface_of(surface), location);
	}

	template<typename T>
	static void bind_scene_render_targets(T* pipeline)
	{
		auto rt = SceneRenderTargets::instance();
		bind_scene_render_target(rt, SceneRenderTargets::BaseColor, pipeline->base_color_texture->location);
		bind_scene_render_target(rt, SceneRenderTargets::Normal, pipeline->normal_texture->location);
		bind_scene_render_target(rt, SceneRenderTargets::Emissive, pipeline->emissive_texture->location);
		bind_scene_render_target(rt, SceneRenderTargets::MSRA, pipeline->msra_texture->location);
		bind_scene_render_target(rt, SceneRenderTargets::SceneDepthZ, pipeline->depth_texture->location);
	}

	trinex_impl_render_pass(Engine::ShadowPass) {}

	trinex_impl_render_pass(Engine::ShadowedLightingPass)
	{
		m_attachments_count = 1;

		m_shader_definitions = {
				{"TRINEX_SHADOWED_LIGHTING_PASS", "1"},
				{"TRINEX_LIGHTING_PASS", "1"},
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

			shadow_map->rhi_depth_stencil_view()->clear(1.0, 255);
			rhi->bind_depth_stencil_target(shadow_map->rhi_depth_stencil_view());

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

	template<typename PipelineType>
	static void add_spot_light(RenderPass* pass, SpotLightComponent* spotlight)
	{
		pass->add_callabble([spotlight, pass]() {
			auto proxy = spotlight->proxy();

			PipelineType* pipeline = PipelineType::instance();
			pipeline->rhi_bind();
			bind_scene_render_targets(pipeline);
			pass->scene_renderer()->bind_global_parameters(pipeline->globals->location);

			const auto& color             = proxy->light_color();
			const auto& intensivity       = proxy->intensivity();
			const auto& location          = proxy->world_transform().location();
			const auto& direction         = proxy->direction();
			const auto& spot_angles       = Vector2f(proxy->cos_outer_cone_angle(), proxy->inv_cos_cone_difference());
			const auto& radius            = proxy->attenuation_radius();
			const auto& fall_off_exponent = proxy->fall_off_exponent();

			// clang-format off
			rhi->update_scalar_parameter(&color, sizeof(color), pipeline->color->offset, pipeline->color->location);
			rhi->update_scalar_parameter(&intensivity, sizeof(intensivity), pipeline->intensivity->offset, pipeline->intensivity->location);
			rhi->update_scalar_parameter(&location, sizeof(location), pipeline->location->offset, pipeline->location->location);
			rhi->update_scalar_parameter(&direction, sizeof(direction), pipeline->direction->offset, pipeline->direction->location);
			rhi->update_scalar_parameter(&spot_angles, sizeof(spot_angles), pipeline->spot_angles->offset, pipeline->spot_angles->location);
			rhi->update_scalar_parameter(&radius, sizeof(radius), pipeline->radius->offset, pipeline->radius->location);
			rhi->update_scalar_parameter(&fall_off_exponent, sizeof(fall_off_exponent), pipeline->fall_off_exponent->offset, pipeline->fall_off_exponent->location);
			// clang-format on

			if constexpr (std::is_same_v<PipelineType, Pipelines::DeferredSpotLightShadowed>)
			{
				float shadow_map_size = static_cast<float>(proxy->shadow_map()->size().x);
				auto view             = camera_view(proxy);

				float depth_bias     = proxy->depth_bias() / shadow_map_size;
				float slope_scale    = proxy->slope_scale() / shadow_map_size;
				auto shadow_projview = view.projection_matrix() * view.view_matrix();

				// clang-format off
				bind_surface(proxy->shadow_map(), pipeline->shadow_map_texture->location);
				rhi->update_scalar_parameter(&depth_bias, sizeof(depth_bias), pipeline->depth_bias->offset, pipeline->depth_bias->location);
				rhi->update_scalar_parameter(&slope_scale, sizeof(slope_scale), pipeline->slope_scale->offset, pipeline->slope_scale->location);
				rhi->update_scalar_parameter(&shadow_projview, sizeof(shadow_projview), pipeline->shadow_map_projview->offset, pipeline->shadow_map_projview->location);
				// clang-format on
			}


			DefaultResources::Buffers::screen_quad->rhi_bind(0, 0);
			rhi->draw(6, 0);
		});
	}

	DeferredLightingPass& DeferredLightingPass::add_light(SpotLightComponent* spotlight)
	{
		if (spotlight->proxy()->is_shadows_enabled())
		{
			add_spot_light<Pipelines::DeferredSpotLightShadowed>(m_shadowed_lighting_pass, spotlight);
		}
		else
		{
			add_spot_light<Pipelines::DeferredSpotLight>(m_lighting_pass, spotlight);
		}
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
