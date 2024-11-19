#include <Core/base_engine.hpp>
#include <Core/default_resources.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/spot_light_component.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/material_parameter.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	static FORCE_INLINE Vector3D calc_spot_light_direction(const Transform& transform)
	{
		return transform.up_vector() * -1.f;
	}

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

	Vector3D SpotLightComponentProxy::direction() const
	{
		return calc_spot_light_direction(world_transform());
	}

	SpotLightComponent::SpotLightComponent() : m_inner_cone_angle(10.f), m_outer_cone_angle(43.f)
	{}

	class UpdateSpotLightDataCommand : public ExecutableObject
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

		int_t execute() override
		{
			m_proxy->outer_cone_angle(m_outer_cone_angle);
			m_proxy->inner_cone_angle(m_inner_cone_angle);
			return sizeof(UpdateSpotLightDataCommand);
		}
	};

	SpotLightComponent& SpotLightComponent::submit_spot_light_data()
	{
		m_outer_cone_angle = glm::clamp(m_outer_cone_angle, 0.f, 89.f);
		m_inner_cone_angle = glm::clamp(m_inner_cone_angle, 0.f, m_outer_cone_angle);
		render_thread()->insert_new_task<UpdateSpotLightDataCommand>(this);
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

	Vector3D SpotLightComponent::direction() const
	{
		return calc_spot_light_direction(world_transform());
	}

	SpotLightComponentProxy* SpotLightComponent::proxy() const
	{
		return typed_proxy<SpotLightComponentProxy>();
	}

	LightComponent::Type SpotLightComponent::light_type() const
	{
		return LightComponent::Type::Spot;
	}

	implement_empty_rendering_methods_for(SpotLightComponent);

	SpotLightComponent& SpotLightComponent::start_play()
	{
		Super::start_play();
		submit_spot_light_data();
		return *this;
	}


#define get_param(param_name, type) reinterpret_cast<MaterialParameters::type*>(material->find_parameter(Name::param_name));
	ColorSceneRenderer& ColorSceneRenderer::render_component(SpotLightComponent* component)
	{
		render_base_component(component);

		SpotLightComponentProxy* proxy = component->proxy();

		if (!(scene_view().show_flags() & ShowFlags::SpotLights) || !proxy->is_enabled() ||
			!component->leaf_class_is<SpotLightComponent>())
			return *this;

		auto pass = deferred_pass();

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
			pass->update_variable(color_parameter->value, proxy->light_color());
		}

		if (intensivity_parameter)
		{
			pass->update_variable(intensivity_parameter->value, proxy->intensivity());
		}

		if (location_parameter)
		{
			pass->update_variable(location_parameter->value, proxy->world_transform().location());
		}

		if (direction_parameter)
		{
			pass->update_variable(direction_parameter->value, proxy->direction());
		}

		if (spot_angles_parameter)
		{
			pass->update_variable(spot_angles_parameter->value,
								  Vector2D(proxy->cos_outer_cone_angle(), proxy->inv_cos_cone_difference()));
		}

		if (radius_parameter)
		{
			pass->update_variable(radius_parameter->value, proxy->attenuation_radius());
		}

		if (fall_off_parameter)
		{
			pass->update_variable(fall_off_parameter->value, proxy->fall_off_exponent());
		}

		pass->bind_material(material, nullptr);
		pass->bind_vertex_buffer(DefaultResources::Buffers::screen_position, 0, 0);
		pass->draw(6, 0);
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
