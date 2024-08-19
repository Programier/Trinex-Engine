#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/default_resources.hpp>
#include <Core/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/Render/command_buffer.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/Render/scene_renderer.hpp>
#include <Engine/scene.hpp>
#include <Graphics/material.hpp>
#include <Graphics/pipeline_buffers.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	implement_engine_class(PointLightComponent, 0)
	{
		Class* self = static_class_instance();

		auto update_data = [](void* object) {
			PointLightComponent* component = reinterpret_cast<PointLightComponent*>(object);
			component->submit_point_light_data();
		};

		auto fall_off_exponent_prop =
				new FloatProperty("Fall Off Exponent", "Fall Off Exponent of this light", &This::m_fall_off_exponent);
		fall_off_exponent_prop->on_prop_changed.push(update_data);
		self->add_property(fall_off_exponent_prop);
	}

	PointLightComponent::PointLightComponent() : m_fall_off_exponent(8.f)
	{}

	float PointLightComponent::fall_off_exponent() const
	{
		return m_fall_off_exponent;
	}

	PointLightComponent& PointLightComponent::fall_off_exponent(float value)
	{
		m_fall_off_exponent = value;
		submit_point_light_data();
		return *this;
	}

	LightComponent::Type PointLightComponent::light_type() const
	{
		return LightComponent::Type::Point;
	}

	float PointLightComponentProxy::fall_off_exponent() const
	{
		return m_fall_off_exponent;
	}

	PointLightComponentProxy& PointLightComponentProxy::fall_off_exponent(float value)
	{
		m_fall_off_exponent = value;
		return *this;
	}

	PointLightComponent& PointLightComponent::submit_point_light_data()
	{
		render_thread()->insert_new_task<UpdateVariableCommand<float>>(m_fall_off_exponent, proxy()->m_fall_off_exponent);
		return *this;
	}

	implement_empty_rendering_methods_for(PointLightComponent);

	PointLightComponent& PointLightComponent::start_play()
	{
		Super::start_play();
		submit_point_light_data();
		return *this;
	}

#define get_param(param_name, type) reinterpret_cast<type*>(material->find_parameter(Name::param_name));
	ColorSceneRenderer& ColorSceneRenderer::render_component(PointLightComponent* component)
	{
		render_base_component(component);

		PointLightComponentProxy* proxy = component->proxy();
		auto layer						= deferred_lighting_layer();

		if (!scene_view().show_flags().has_all(ShowFlags::PointLights) || !proxy->is_enabled() ||
			!component->leaf_class_is<PointLightComponent>())
			return *this;

		Material* material = DefaultResources::Materials::point_light;

		Vec3MaterialParameter* color_parameter		  = get_param(color, Vec3MaterialParameter);
		Vec3MaterialParameter* location_parameter	  = get_param(location, Vec3MaterialParameter);
		FloatMaterialParameter* intensivity_parameter = get_param(intensivity, FloatMaterialParameter);
		FloatMaterialParameter* radius_parameter	  = get_param(radius, FloatMaterialParameter);
		FloatMaterialParameter* fall_off_parameter	  = get_param(fall_off_exponent, FloatMaterialParameter);

		if (color_parameter)
		{
			layer->update_variable(color_parameter->param, proxy->light_color());
		}

		if (location_parameter)
		{
			layer->update_variable(location_parameter->param, proxy->world_transform().location());
		}

		if (intensivity_parameter)
		{
			layer->update_variable(intensivity_parameter->param, proxy->intensivity());
		}

		if (radius_parameter)
		{
			layer->update_variable(radius_parameter->param, proxy->attenuation_radius());
		}

		if (fall_off_parameter)
		{
			layer->update_variable(fall_off_parameter->param, proxy->fall_off_exponent());
		}

		layer->bind_material(material);
		layer->bind_vertex_buffer(DefaultResources::Buffers::screen_position, 0, 0);
		layer->draw(6, 0);
		return *this;
	}

	PointLightComponent& PointLightComponent::render(class SceneRenderer* renderer)
	{
		renderer->render_component(this);
		return *this;
	}

	ActorComponentProxy* PointLightComponent::create_proxy()
	{
		return new PointLightComponentProxy();
	}

	PointLightComponentProxy* PointLightComponent::proxy() const
	{
		return typed_proxy<PointLightComponentProxy>();
	}
}// namespace Engine
