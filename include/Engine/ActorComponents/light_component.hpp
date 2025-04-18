#pragma once
#include <Core/pointer.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	class RenderSurface;

	class ENGINE_EXPORT LightComponentProxy : public SceneComponentProxy
	{
	protected:
		AABB_3Df m_bounds;
		Color m_light_color;
		float m_intensivity;
		float m_depth_bias;
		float m_slope_scale;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

		Pointer<RenderSurface> m_shadow_map;

	public:
		inline const AABB_3Df& bounding_box() const { return m_bounds; };
		inline const Color& light_color() const { return m_light_color; }
		inline float intensivity() const { return m_intensivity; }
		inline float depth_bias() const { return m_depth_bias; }
		inline float slope_scale() const { return m_slope_scale; }
		inline bool is_enabled() const { return m_is_enabled; }
		inline bool is_shadows_enabled() const { return m_is_shadows_enabled; }
		inline RenderSurface* shadow_map() const { return m_shadow_map; }

		friend class LightComponent;
	};

	class ENGINE_EXPORT LightComponent : public SceneComponent
	{
		trinex_declare_class(LightComponent, SceneComponent);

	public:
		enum Type
		{
			Unknown     = -1,
			Point       = 0,
			Spot        = 1,
			Directional = 2,
			Num         = 3
		};

	private:
		Pointer<RenderSurface> m_shadow_map;

		AABB_3Df m_bounds;
		Color m_light_color;
		float m_intensivity;
		float m_depth_bias;
		float m_slope_scale;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

		LightComponent& submit_light_info_render_thread();

	public:
		LightComponent();
		inline const AABB_3Df& bounding_box() const { return m_bounds; };
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
		virtual LightComponent& render(class SceneRenderer*);
		ActorComponentProxy* create_proxy() override;
		LightComponentProxy* proxy() const;

		LightComponent& on_transform_changed() override;
		LightComponent& start_play() override;
		LightComponent& stop_play() override;
		LightComponent& update_bounding_box();
		LightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;
		~LightComponent();
	};
}// namespace Engine
