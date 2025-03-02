#include <Core/default_resources.hpp>
#include <Core/etl/templates.hpp>
#include <Core/reflection/render_pass_info.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
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

	trinex_impl_render_pass(Engine::ShadowlessLightingPass)
	{
		m_attachments_count = 1;

		m_shader_definitions = {
				{"TRINEX_SHADOWLESS_LIGHTING_PASS", "1"},
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

	trinex_impl_render_pass(Engine::DeferredLightingPass)
	{}

	DeferredLightingPass& DeferredLightingPass::initialize()
	{
		Super::initialize();
		m_shadowless_lighting_pass = create_subpass<ShadowlessLightingPass>();
		m_lighting_pass            = create_subpass<LightingPass>();
		return *this;
	}

	inline RenderPass* DeferredLightingPass::find_render_pass(LightComponentProxy* light)
	{
		if (light->is_shadows_enabled())
			return m_lighting_pass;
		return m_shadowless_lighting_pass;
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

#define get_param(param_name, type)                                                                                              \
	reinterpret_cast<MaterialParameters::type*>(material->find_parameter(LightComponent::name_##param_name));

	DeferredLightingPass& DeferredLightingPass::add_light(SpotLightComponent* spotlight)
	{
		auto proxy       = spotlight->proxy();
		RenderPass* pass = find_render_pass(proxy);

		pass->add_callabble([spotlight, pass]() {
			auto proxy         = spotlight->proxy();
			Material* material = DefaultResources::Materials::spot_light;

			auto* color_parameter       = get_param(color, Float3);
			auto* intensivity_parameter = get_param(intensivity, Float);
			auto* spot_angles_parameter = get_param(spot_angles, Float2);
			auto* location_parameter    = get_param(location, Float3);
			auto* direction_parameter   = get_param(direction, Float3);
			auto* radius_parameter      = get_param(radius, Float);
			auto* fall_off_parameter    = get_param(fall_off_exponent, Float);

			if (color_parameter)
			{
				color_parameter->value = proxy->light_color();
			}

			if (intensivity_parameter)
			{
				intensivity_parameter->value = proxy->intensivity();
			}

			if (location_parameter)
			{
				location_parameter->value = proxy->world_transform().location();
			}

			if (direction_parameter)
			{
				direction_parameter->value = proxy->direction();
			}

			if (spot_angles_parameter)
			{
				spot_angles_parameter->value = Vector2f(proxy->cos_outer_cone_angle(), proxy->inv_cos_cone_difference());
			}

			if (radius_parameter)
			{
				radius_parameter->value = proxy->attenuation_radius();
			}

			if (fall_off_parameter)
			{
				fall_off_parameter->value = proxy->fall_off_exponent();
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
