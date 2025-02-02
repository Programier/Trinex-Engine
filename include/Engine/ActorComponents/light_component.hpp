#pragma once
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/aabb.hpp>

namespace Engine
{
	class ENGINE_EXPORT LightComponentProxy : public SceneComponentProxy
	{
	protected:
		AABB_3Df m_bounds;
		Color3 m_light_color;
		float m_intensivity;
		bool m_is_enabled;
		bool m_is_shadows_enabled;

	public:
		const AABB_3Df& bounding_box() const;
		const Color3& light_color() const;
		float intensivity() const;
		bool is_enabled() const;
		bool is_shadows_enabled() const;

		LightComponentProxy& bounding_box(const AABB_3Df& bounds);
		LightComponentProxy& light_color(const Color3& color);
		LightComponentProxy& intensivity(float value);
		LightComponentProxy& is_enabled(bool enabled);
		LightComponentProxy& is_shadows_enabled(bool enabled);
		friend class LightComponent;
	};

	class ENGINE_EXPORT LightComponent : public SceneComponent
	{
		declare_class(LightComponent, SceneComponent);

	public:
		enum Type
		{
			Unknown     = -1,
			Point       = 0,
			Spot        = 1,
			Directional = 2,
			Num         = 3
		};

		static Name name_color;
		static Name name_intensivity;
		static Name name_location;
		static Name name_radius;
		static Name name_fall_off_exponent;
		static Name name_direction;
		static Name name_spot_angles;

	private:
		AABB_3Df m_bounds;
		Color3 m_light_color;
		float m_intensivity;
		bool m_is_enabled;
		bool m_is_shadows_enabled;


		LightComponent& submit_light_info_render_thread();

	public:
		LightComponent();
		const AABB_3Df& bounding_box() const;
		const Color3& light_color() const;
		float intensivity() const;
		bool is_enabled() const;
		bool is_shadows_enabled() const;

		LightComponent& light_color(const Color3& color);
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
