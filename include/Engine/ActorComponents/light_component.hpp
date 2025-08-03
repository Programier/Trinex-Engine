#pragma once
#include <Core/math/box.hpp>
#include <Core/pointer.hpp>
#include <Core/types/color.hpp>
#include <Engine/ActorComponents/scene_component.hpp>

namespace Engine
{
	class RenderSurface;
	struct LightRenderParameters;

	class ENGINE_EXPORT LightComponent : public SceneComponent
	{
		trinex_declare_class(LightComponent, SceneComponent);

	public:
		enum Type
		{
			Undefined   = 0,
			Point       = 1,
			Spot        = 2,
			Directional = 3,
		};

		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		protected:
			LinearColor m_light_color;
			float m_intensivity;
			float m_depth_bias;
			float m_slope_scale;
			bool m_is_enabled;
			bool m_is_shadows_enabled;

		public:
			inline const LinearColor& light_color() const { return m_light_color; }
			inline float intensivity() const { return m_intensivity; }
			inline float depth_bias() const { return m_depth_bias; }
			inline float slope_scale() const { return m_slope_scale; }
			inline bool is_enabled() const { return m_is_enabled; }
			inline bool is_shadows_enabled() const { return m_is_shadows_enabled; }

			virtual Proxy& render_parameters(LightRenderParameters& out);
			virtual Type light_type() const;
			friend class LightComponent;
		};


	protected:
		Box3f m_bounding_box;

	private:
		Color m_light_color;
		float m_intensivity;
		float m_depth_bias;
		float m_slope_scale;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

		LightComponent& submit_light_info_render_thread();

	public:
		LightComponent();
		inline const Box3f& bounding_box() const { return m_bounding_box; }
		inline const Color& light_color() const { return m_light_color; }
		inline float intensivity() const { return m_intensivity; }
		inline float depth_bias() const { return m_depth_bias; }
		inline float slope_scale() const { return m_slope_scale; }
		inline bool is_enabled() const { return m_is_enabled; }
		inline bool is_shadows_enabled() const { return m_is_shadows_enabled; }

		LightComponent& light_color(const Color& color);
		LightComponent& intensivity(float value);
		LightComponent& is_enabled(bool enabled);
		LightComponent& is_shadows_enabled(bool enabled);

		virtual Type light_type() const = 0;
		virtual LightComponent& update_bounding_box();
		Proxy* create_proxy() override;
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		LightComponent& on_transform_changed() override;
		LightComponent& start_play() override;
		LightComponent& stop_play() override;
		LightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
		~LightComponent();
	};
}// namespace Engine
