#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>

namespace Engine
{
	trinex_implement_engine_class(PointLightComponent, 0)
	{
		auto* self = static_class_instance();

		trinex_refl_prop(self, This, m_fall_off_exponent)
		        ->display_name("Fall Off Exponent")
		        .tooltip("Fall Off Exponent of this light");
	}

	PointLightComponent::PointLightComponent() : m_fall_off_exponent(2.f) {}

	PointLightComponent& PointLightComponent::submit_point_light_data()
	{
		render_thread()->create_task<UpdateVariableCommand<float>>(m_fall_off_exponent, proxy()->m_fall_off_exponent);
		return *this;
	}

	PointLightComponent& PointLightComponent::start_play()
	{
		Super::start_play();
		submit_point_light_data();
		return *this;
	}

	SceneRenderer& SceneRenderer::render_component(PointLightComponent* component)
	{
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::render_component(PointLightComponent* component)
	{
		render_base_component(component);

		auto* proxy = component->proxy();

		if (!(scene_view().show_flags() & ShowFlags::PointLights) || !proxy->is_enabled() ||
		    !component->leaf_class_is<PointLightComponent>())
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

	PointLightComponent& PointLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	PointLightComponent::Proxy* PointLightComponent::create_proxy()
	{
		return new Proxy();
	}

	PointLightComponent& PointLightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_class_instance())
		{
			submit_point_light_data();
		}
		return *this;
	}
}// namespace Engine
