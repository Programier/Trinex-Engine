#include <Core/class.hpp>
#include <Engine/ActorComponents/point_light_component.hpp>
#include <Engine/ActorComponents/sprite_component.hpp>
#include <Engine/Actors/point_light_actor.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
    implement_engine_class_default_init(PointLightActor);

    PointLightActor::PointLightActor()
    {
        m_point_light_component = create_component<PointLightComponent>(("PointLightComponent0"));
        create_component<SpriteComponent>(("SpriteComponent0"))->texture = Object::find_object_checked<Texture2D>("Editor::PointLightSprite");
    }

    PointLightComponent* PointLightActor::point_light_component() const
    {
        return m_point_light_component;
    }
}// namespace Engine
