#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT DirectionalLightComponentProxy : public LightComponentProxy
	{
	public:
		Vector3D direction() const;
		friend class DirectionalLightComponent;
	};

	class ENGINE_EXPORT DirectionalLightComponent : public LightComponent
	{
		declare_class(DirectionalLightComponent, LightComponent);

	public:
		Vector3D direction() const;

		Type light_type() const override;
		ActorComponentProxy* create_proxy() override;
		DirectionalLightComponentProxy* proxy() const;
		DirectionalLightComponent& render(class SceneRenderer*) override;
		DirectionalLightComponent& update(float dt) override;
	};
}// namespace Engine
