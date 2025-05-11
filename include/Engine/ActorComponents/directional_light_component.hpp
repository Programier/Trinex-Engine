#pragma once
#include <Engine/ActorComponents/light_component.hpp>

namespace Engine
{
	class ENGINE_EXPORT DirectionalLightComponent : public LightComponent
	{
		trinex_declare_class(DirectionalLightComponent, LightComponent);

	public:
		class ENGINE_EXPORT Proxy : public Super::Proxy
		{
		public:
			inline Vector3f direction() const { return -world_transform().up_vector(); }
			friend class DirectionalLightComponent;
		};

	public:
		Vector3f direction() const;

		Type light_type() const override;
		Proxy* create_proxy() override;
		inline Proxy* proxy() const { return typed_proxy<Proxy>(); }
	};
}// namespace Engine
