#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT DirectionalLightComponent : public LightComponent
	{
		trinex_class(DirectionalLightComponent, LightComponent);

	private:
		float m_shadows_distance = 50.f;

	protected:
		float calculate_light_intensity() const override;

	public:
		Type light_type() const override;
		DirectionalLightComponent& shadows_distance(float value);
		DirectionalLightComponent& render_parameters(LightRenderParameters& out) override;

		inline Vector3f direction() const { return world_transform().forward_vector(); }
		inline float shadows_distance() const { return m_shadows_distance; }
	};
}// namespace Engine
