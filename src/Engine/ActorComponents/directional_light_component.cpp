#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>


namespace Engine
{
	trinex_implement_engine_class_default_init(DirectionalLightComponent, 0);

	DirectionalLightComponent::Type DirectionalLightComponent::light_type() const
	{
		return Type::Directional;
	}

	DirectionalLightComponent::Proxy* DirectionalLightComponent::create_proxy()
	{
		return new Proxy();
	}

	SceneRenderer& SceneRenderer::render_component(DirectionalLightComponent* component)
	{
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::render_component(DirectionalLightComponent* component)
	{
		render_base_component(component);
		auto* proxy = component->proxy();

		if (!(scene_view().show_flags() & ShowFlags::DirectionalLights) || !proxy->is_enabled() ||
		    !component->leaf_class_is<DirectionalLightComponent>())
			return *this;

		if (component->is_shadows_enabled())
		{
			shadow_pass()->add_light(m_depth_renderer, component);
		}

		if (auto pass = deferred_lighting_pass())
		{
			pass->add_light(component);
		}
		return *this;
	}

	DirectionalLightComponent& DirectionalLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}
}// namespace Engine
