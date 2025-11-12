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
#include <Graphics/gpu_buffers.hpp>
#include <Graphics/material.hpp>
#include <Graphics/render_pools.hpp>
#include <RHI/rhi.hpp>

namespace Engine
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

		out.color = m_light_color;
		out.color *= calculate_light_intensity();
		out.location = transform.location;
		return *this;
	}

	LightComponent& LightComponent::on_transform_changed()
	{
		Super::on_transform_changed();

		if (Scene* world_scene = scene())
		{
			world_scene->update_light_transform(this);
		}

		return *this;
	}

	LightComponent& LightComponent::start_play()
	{
		Super::start_play();
		Scene* world_scene = scene();
		if (world_scene)
		{
			world_scene->add_light(this);
		}
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::stop_play()
	{
		Super::stop_play();

		Scene* world_scene = scene();

		if (world_scene)
		{
			world_scene->remove_light(this);
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

	LightComponent& LightComponent::light_color(const Color& color)
	{
		m_light_color = color;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::intensity_units(LightUnits units)
	{
		if (m_intensity_units == units)
			return *this;

		m_intensity_units = units;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::intensity(float value)
	{
		m_intensity = value;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::intensity(float value, LightUnits units)
	{
		m_intensity       = value;
		m_intensity_units = units;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::is_enabled(bool enabled)
	{
		m_is_enabled = enabled;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::is_shadows_enabled(bool enabled)
	{
		m_is_shadows_enabled = enabled;
		return submit_light_info_render_thread();
	}

	LightComponent& LightComponent::submit_light_info_render_thread()
	{
		// render_thread()->call([proxy           = proxy(),                    //
		//                        color           = m_light_color,              //
		//                        intensity       = calculate_light_intensity(),//
		//                        depth_bias      = m_depth_bias,               //
		//                        slope_scale     = m_slope_scale,              //
		//                        enabled         = m_is_enabled,               //
		//                        shadows_enabled = m_is_shadows_enabled]() {
		// 	proxy->m_light_color        = color;
		// 	proxy->m_intensity          = intensity;
		// 	proxy->m_depth_bias         = depth_bias;
		// 	proxy->m_slope_scale        = slope_scale;
		// 	proxy->m_is_enabled         = enabled;
		// 	proxy->m_is_shadows_enabled = shadows_enabled;
		// });
		return *this;
	}

	LightComponent& LightComponent::update_bounding_box()
	{
		static constexpr Vector3f extents = {1.f, 1.f, 1.f};
		const Vector3f& location          = world_transform().location;
		m_bounding_box                    = Box3f(location - extents, location + extents);
		return *this;
	}

	LightComponent& LightComponent::on_property_changed(const Refl::PropertyChangedEvent& event)
	{
		Super::on_property_changed(event);

		if (event.property->owner() == static_reflection())
		{
			if (event.property->address(this) == &m_is_shadows_enabled)
			{
				is_shadows_enabled(m_is_shadows_enabled);
			}
			else
			{
				submit_light_info_render_thread();
			}
		}

		return *this;
	}

	LightComponent::~LightComponent() {}
}// namespace Engine
