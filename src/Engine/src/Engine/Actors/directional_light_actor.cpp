#include <Core/class.hpp>
#include <Engine/ActorComponents/directional_light_component.hpp>
#include <Engine/Actors/directional_light_actor.hpp>

namespace Engine
{
    implement_engine_class(DirectionalLightActor, 0);
    implement_initialize_class(DirectionalLightActor)
    {}

    DirectionalLightActor::DirectionalLightActor()
    {
        m_directional_light_component = create_component<DirectionalLightComponent>("DirectionalLightComponent");
    }

    DirectionalLightComponent* DirectionalLightActor::directional_light_component() const
    {
        return m_directional_light_component;
    }
}// namespace Engine
