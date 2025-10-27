#pragma once
#include <Core/math/box.hpp>
#include <Core/pointer.hpp>
#include <Core/types/color.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Engine
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
		trinex_declare_enum(LightUnits);
	};

	class ENGINE_EXPORT LightComponent : public SceneComponent
	{
		trinex_declare_class(LightComponent, SceneComponent);

	public:
		enum Type
		{
			Point       = 0,
			Spot        = 1,
			Directional = 2,
		};

		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		protected:
			LinearColor m_light_color;
			float m_intensity;
			float m_depth_bias;
			float m_slope_scale;
			bool m_is_enabled;
			bool m_is_shadows_enabled;

		public:
			inline const LinearColor& light_color() const { return m_light_color; }
			inline float intensity() const { return m_intensity; }
			inline float depth_bias() const { return m_depth_bias; }
			inline float slope_scale() const { return m_slope_scale; }
			inline bool is_enabled() const { return m_is_enabled; }
			inline bool is_shadows_enabled() const { return m_is_shadows_enabled; }

			virtual Proxy& render_parameters(LightRenderParameters& out);
			virtual Type light_type() const = 0;
			friend class LightComponent;
		};


	protected:
		Box3f m_bounding_box;

	private:
		Color m_light_color;
		LightUnits m_intensity_units;
		float m_intensity;
		float m_depth_bias;
		float m_slope_scale;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

		LightComponent& submit_light_info_render_thread();

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

		LightComponent& light_color(const Color& color);
		LightComponent& intensity_units(LightUnits units);
		LightComponent& intensity(float value);
		LightComponent& intensity(float value, LightUnits units);
		LightComponent& is_enabled(bool enabled);
		LightComponent& is_shadows_enabled(bool enabled);

		virtual Type light_type() const = 0;
		virtual LightComponent& update_bounding_box();
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		LightComponent& on_transform_changed() override;
		LightComponent& start_play() override;
		LightComponent& stop_play() override;
		LightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
		~LightComponent();
	};
}// namespace Engine
