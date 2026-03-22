#pragma once
#include <Core/math/box.hpp>
#include <Core/pointer.hpp>
#include <Core/types/color.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Trinex
{
	class RenderSurface;
	struct LightRenderParameters;

	struct ENGINE_EXPORT LightUnits {
		enum Enum : EnumerateType
		{
			Unitless = 0,
			Candelas = 1,
			Lumens   = 2,
			EV       = 3,
		};

		trinex_enum_struct(LightUnits);
		trinex_enum(LightUnits);
	};

	class ENGINE_EXPORT LightComponent : public SceneComponent
	{
		trinex_class(LightComponent, SceneComponent);

	public:
		enum Type
		{
			Point       = 0,
			Spot        = 1,
			Directional = 2,
		};

	protected:
		Box3f m_bounding_box;

	private:
		u32 m_light_id = 0xFFFFFFFF;
		Color m_light_color;
		LightUnits m_intensity_units;
		float m_intensity;
		float m_depth_bias;
		float m_slope_scale;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

	protected:
		virtual float calculate_light_intensity() const;

	public:
		LightComponent();
		inline const Box3f& bounding_box() const { return m_bounding_box; }
		inline const Color& light_color() const { return m_light_color; }
		inline LightUnits intensity_units() const { return m_intensity_units; }
		inline float intensity() const { return m_intensity; }
		inline float depth_bias() const { return m_depth_bias; }
		inline float slope_scale() const { return m_slope_scale; }
		inline bool is_enabled() const { return m_is_enabled; }
		inline bool is_shadows_enabled() const { return m_is_shadows_enabled; }

		inline LightComponent& light_color(const Color& color) { trinex_this_return(m_light_color = color); }
		inline LightComponent& intensity_units(LightUnits units) { trinex_this_return(m_intensity_units = units); }
		inline LightComponent& intensity(float value) { trinex_this_return(m_intensity = value); };
		inline LightComponent& is_enabled(bool enabled) { trinex_this_return(m_is_enabled = enabled); }
		inline LightComponent& is_shadows_enabled(bool enabled) { trinex_this_return(m_is_shadows_enabled = enabled); }
		inline LightComponent& intensity(float value, LightUnits units)
		{
			m_intensity       = value;
			m_intensity_units = units;
			return *this;
		}

		virtual LightComponent& render_parameters(LightRenderParameters& out);
		virtual Type light_type() const = 0;
		virtual LightComponent& update_bounding_box();

		LightComponent& on_transform_changed() override;
		LightComponent& start_play() override;
		LightComponent& stop_play() override;
		~LightComponent();
	};
}// namespace Trinex
