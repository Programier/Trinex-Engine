#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/lighting_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/Render/shadow_pass.hpp>

namespace Engine
{
	implement_engine_class(SpotLightComponent, 0)
	{
		auto* self = static_class_instance();

		auto on_data_changed = [](const Refl::PropertyChangedEvent& event) {
			event.context_as<SpotLightComponent>()->submit_spot_light_data();
		};

		trinex_refl_prop(self, This, m_outer_cone_angle)
				->push_change_listener(on_data_changed)
				.display_name("Outer Cone Angle")
				.tooltip("Outer Cone Angle of this spot light");

		trinex_refl_prop(self, This, m_inner_cone_angle)
				->push_change_listener(on_data_changed)
				.display_name("Inner Cone Angle")
				.tooltip("Inner Cone Angle of this spot light");
	}

	SpotLightComponentProxy& SpotLightComponentProxy::update_spot_angles()
	{
		m_cos_outer_cone_angle    = glm::cos(m_outer_cone_angle);
		float cos_inner_cone      = glm::cos(m_inner_cone_angle);
		m_inv_cos_cone_difference = 1.0f / (cos_inner_cone - m_cos_outer_cone_angle);
		return *this;
	}

	float SpotLightComponentProxy::inner_cone_angle() const
	{
		return m_inner_cone_angle;
	}

	float SpotLightComponentProxy::outer_cone_angle() const
	{
		return m_outer_cone_angle;
	}

	float SpotLightComponentProxy::cos_outer_cone_angle() const
	{
		return m_cos_outer_cone_angle;
	}

	float SpotLightComponentProxy::inv_cos_cone_difference() const
	{
		return m_inv_cos_cone_difference;
	}

	SpotLightComponentProxy& SpotLightComponentProxy::inner_cone_angle(float value)
	{
		m_inner_cone_angle = value;
		return update_spot_angles();
	}

	SpotLightComponentProxy& SpotLightComponentProxy::outer_cone_angle(float value)
	{
		m_outer_cone_angle = value;
		return update_spot_angles();
	}

	Vector3f SpotLightComponentProxy::direction() const
	{
		return world_transform().forward_vector();
	}

	SpotLightComponent::SpotLightComponent() : m_inner_cone_angle(10.f), m_outer_cone_angle(43.f)
	{}

	class UpdateSpotLightDataCommand : public Task<UpdateSpotLightDataCommand>
	{
	private:
		float m_outer_cone_angle;
		float m_inner_cone_angle;
		SpotLightComponentProxy* m_proxy;

	public:
		UpdateSpotLightDataCommand(SpotLightComponent* component)
			: m_outer_cone_angle(glm::radians(component->outer_cone_angle())),
			  m_inner_cone_angle(glm::radians(component->inner_cone_angle())), m_proxy(component->proxy())
		{}

		void execute() override
		{
			m_proxy->outer_cone_angle(m_outer_cone_angle);
			m_proxy->inner_cone_angle(m_inner_cone_angle);
		}
	};

	SpotLightComponent& SpotLightComponent::submit_spot_light_data()
	{
		m_outer_cone_angle = glm::clamp(m_outer_cone_angle, 0.f, 89.f);
		m_inner_cone_angle = glm::clamp(m_inner_cone_angle, 0.f, m_outer_cone_angle);
		render_thread()->create_task<UpdateSpotLightDataCommand>(this);
		return *this;
	}

	float SpotLightComponent::inner_cone_angle() const
	{
		return m_inner_cone_angle;
	}

	float SpotLightComponent::outer_cone_angle() const
	{
		return m_outer_cone_angle;
	}

	SpotLightComponent& SpotLightComponent::inner_cone_angle(float value)
	{
		m_inner_cone_angle = value;
		return submit_spot_light_data();
	}

	SpotLightComponent& SpotLightComponent::outer_cone_angle(float value)
	{
		m_outer_cone_angle = value;
		return submit_spot_light_data();
	}

	Vector3f SpotLightComponent::direction() const
	{
		return world_transform().forward_vector();
	}

	SpotLightComponentProxy* SpotLightComponent::proxy() const
	{
		return typed_proxy<SpotLightComponentProxy>();
	}

	LightComponent::Type SpotLightComponent::light_type() const
	{
		return LightComponent::Type::Spot;
	}

	SpotLightComponent& SpotLightComponent::start_play()
	{
		Super::start_play();
		submit_spot_light_data();
		return *this;
	}

	SceneRenderer& SceneRenderer::render_component(SpotLightComponent* component)
	{
		return *this;
	}

	ColorSceneRenderer& ColorSceneRenderer::render_component(SpotLightComponent* component)
	{
		render_base_component(component);

		SpotLightComponentProxy* proxy = component->proxy();

		if (!(scene_view().show_flags() & ShowFlags::SpotLights) || !proxy->is_enabled() ||
			!component->leaf_class_is<SpotLightComponent>())
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

	SpotLightComponent& SpotLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	ActorComponentProxy* SpotLightComponent::create_proxy()
	{
		return new SpotLightComponentProxy();
	}
}// namespace Engine
