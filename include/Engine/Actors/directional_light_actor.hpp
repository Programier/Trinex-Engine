#pragma once
#include <Engine/Actors/light_actor.hpp>

namespace Engine
{
    class ENGINE_EXPORT DirectionalLightActor : public LightActor
    {
        declare_class(DirectionalLightActor, LightActor);

    private:
        class DirectionalLightComponent* m_directional_light_component = nullptr;

    public:
        DirectionalLightActor();
        DirectionalLightComponent* directional_light_component() const;
    };
}// namespace Engine
