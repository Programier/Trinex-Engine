#pragma once
#include <Engine/ActorComponents/local_light_component.hpp>


namespace Engine
{
	class ENGINE_EXPORT PointLightComponent : public LocalLightComponent
	{
		trinex_declare_class(PointLightComponent, LocalLightComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		private:
			float m_fall_off_exponent;

		public:
			inline float fall_off_exponent() const { return m_fall_off_exponent; }
			virtual Proxy& render_parameters(LightRenderParameters& out);
			friend class PointLightComponent;
		};

	private:
		float m_fall_off_exponent;

		PointLightComponent& submit_point_light_data();

	public:
		PointLightComponent();

		PointLightComponent& start_play() override;
		Proxy* create_proxy() override;
		PointLightComponent& on_property_changed(const Refl::PropertyChangedEvent& event) override;

		inline float fall_off_exponent() const { return m_fall_off_exponent; }
		inline Type light_type() const override { return Type::Point; }
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }

		inline PointLightComponent& fall_off_exponent(float value)
		{
			m_fall_off_exponent = value;
			return submit_point_light_data();
		}
	};

}// namespace Engine
