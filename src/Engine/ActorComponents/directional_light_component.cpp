#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>

#include <Graphics/rhi.hpp>

namespace Engine
{
	trinex_implement_engine_class_default_init(DirectionalLightComponent, 0);

	static Vector3f get_direction(const Transform& world_transform)
	{
		return -world_transform.up_vector();
	}

	Vector3f DirectionalLightComponentProxy::direction() const
	{
		return get_direction(world_transform());
	}

	Vector3f DirectionalLightComponent::direction() const
	{
		return get_direction(world_transform());
	}

	DirectionalLightComponent::Type DirectionalLightComponent::light_type() const
	{
		return Type::Directional;
	}

	ActorComponentProxy* DirectionalLightComponent::create_proxy()
	{
		return new DirectionalLightComponentProxy();
	}

	DirectionalLightComponentProxy* DirectionalLightComponent::proxy() const
	{
		return typed_proxy<DirectionalLightComponentProxy>();
	}

	SceneRenderer& SceneRenderer::render_component(DirectionalLightComponent* component)
	{
		return *this;
	}

#define get_param(param_name, type)                                                                                              \
	reinterpret_cast<MaterialParameters::type*>(material->find_parameter(LightComponent::name_##param_name));
	ColorSceneRenderer& ColorSceneRenderer::render_component(DirectionalLightComponent* component)
	{
		render_base_component(component);

		RenderPass* pass                      = deferred_lighting_pass();
		DirectionalLightComponentProxy* proxy = component->proxy();


		if (!(scene_view().show_flags() & ShowFlags::DirectionalLights) || !proxy->is_enabled() ||
		    !component->leaf_class_is<DirectionalLightComponent>())
			return *this;

		Material* material = DefaultResources::Materials::directional_light;

		auto* color_parameter       = get_param(color, Float3);
		auto* direction_parameter   = get_param(direction, Float3);
		auto* intensivity_parameter = get_param(intensivity, Float);

		if (color_parameter)
		{
			pass->assign(color_parameter->value, proxy->light_color());
		}

		if (direction_parameter)
		{
			pass->assign(direction_parameter->value, proxy->direction());
		}

		if (intensivity_parameter)
		{
			pass->assign(intensivity_parameter->value, proxy->intensivity());
		}

		pass->bind_material(material, nullptr);
		pass->draw(6, 0);
		return *this;
	}

	DirectionalLightComponent& DirectionalLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}
}// namespace Engine
