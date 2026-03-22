#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/reflection/class.hpp>
#include <Core/reflection/enum.hpp>
#include <Core/reflection/property.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/Render/lighting.hpp>
#include <Engine/Render/render_pass.hpp>
#include <Engine/camera_view.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/rhi.hpp>

namespace Trinex
{
	trinex_implement_engine_enum(LightUnits, 0, Unitless, Candelas, Lumens, EV);

	trinex_implement_engine_class(LightComponent, 0)
	{
		trinex_refl_prop(m_light_color)->tooltip("Color of this light");
		trinex_refl_prop(m_is_enabled)->tooltip("Is this light enabled");
		trinex_refl_prop(m_is_shadows_enabled)->tooltip("The light source can cast real-time shadows");
		trinex_refl_prop(m_intensity)->tooltip("Intensivity of this light");
		trinex_refl_virtual_prop(m_intensity_units, intensity_units, intensity_units)->tooltip("Intencity units of this light");

		trinex_refl_prop(m_depth_bias);
		trinex_refl_prop(m_slope_scale);
	}

	LightComponent::LightComponent()
	    : m_light_color(255, 255, 255, 255), m_intensity_units(LightUnits::Lumens), m_intensity(250.f), m_depth_bias(0.5f),
	      m_slope_scale(0.5f), m_is_enabled(true), m_is_shadows_enabled(false)
	{}

	LightComponent& LightComponent::render_parameters(LightRenderParameters& out)
	{
		auto& transform = world_transform();

		out.color = LinearColor(m_light_color);
		out.color *= calculate_light_intensity();
		out.location = transform.location;
		return *this;
	}

	LightComponent& LightComponent::on_transform_changed()
	{
		Super::on_transform_changed();
		update_bounding_box();

		if (m_light_id != 0xFFFFFFFF)
		{
			world()->scene()->update_light(m_light_id, m_bounding_box);
		}

		return *this;
	}

	LightComponent& LightComponent::start_play()
	{
		Super::start_play();

		if (World* light_world = world())
		{
			m_light_id = light_world->scene()->add_light(this, m_bounding_box);
		}
		return *this;
	}

	LightComponent& LightComponent::stop_play()
	{
		Super::stop_play();

		if (m_light_id != 0xFFFFFFFF)
		{
			world()->scene()->remove_light(m_light_id);
			m_light_id = 0xFFFFFFFF;
		}

		return *this;
	}

	float LightComponent::calculate_light_intensity() const
	{
		if (m_intensity_units == LightUnits::EV)
		{
			return Math::ev100_to_luminance(m_intensity);
		}
		else if (m_intensity_units == LightUnits::Unitless)
		{
			return m_intensity * 16.f;
		}

		return m_intensity;
	}

	LightComponent& LightComponent::update_bounding_box()
	{
		static const Vector3f extents = {1.f, 1.f, 1.f};
		const Vector3f& location      = world_transform().location;
		m_bounding_box                = Box3f(location - extents, location + extents);
		return *this;
	}

	LightComponent::~LightComponent() {}
}// namespace Trinex
