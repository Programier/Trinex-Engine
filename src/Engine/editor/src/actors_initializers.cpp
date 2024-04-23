#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Engine/ActorComponents/light_component.hpp>
#include <Engine/ActorComponents/primitive_component.hpp>
#include <Engine/ActorComponents/sprite_component.hpp>
#include <Engine/Actors/light_actor.hpp>
#include <Engine/Render/scene_layer.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Graphics/texture_2D.hpp>

namespace Engine
{
    class PrimitiveHitboxRenderingComponent : public PrimitiveComponent
    {
        declare_class(PrimitiveHitboxRenderingComponent, PrimitiveComponent);

    public:
        PrimitiveHitboxRenderingComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override
        {
            Actor* self_actor = actor();
            if (World* actor_world = self_actor->world())
            {
                if (actor_world->is_selected(self_actor))
                {
                    reinterpret_cast<PrimitiveComponent*>(actor()->scene_component())
                            ->proxy()
                            ->bounding_box()
                            .write_to_batcher(scene->scene_output_layer()->lines, {255, 255, 0, 255});
                }
            }
            return *this;
        }
    };

    implement_engine_class_default_init(PrimitiveHitboxRenderingComponent);

    class LightHitboxRenderingComponent : public PrimitiveComponent
    {
        declare_class(LightHitboxRenderingComponent, PrimitiveComponent);

    public:
        LightHitboxRenderingComponent& add_to_scene_layer(class Scene* scene, class SceneRenderer* renderer) override
        {
            Actor* self_actor = actor();
            if (World* actor_world = self_actor->world())
            {
                if (actor_world->is_selected(self_actor))
                {
                    for (ActorComponent* component : self_actor->owned_components())
                    {
                        static Name name_sprite = "SpriteComponent0";
                        if (component->name() == name_sprite)
                        {
                            reinterpret_cast<PrimitiveComponent*>(component)->proxy()->bounding_box().write_to_batcher(
                                    scene->scene_output_layer()->lines, {255, 255, 0, 255});
                            break;
                        }
                    }
                }
            }
            return *this;
        }
    };

    implement_engine_class_default_init(LightHitboxRenderingComponent);

    static void bind_initializers()
    {
        Class* self = Class::static_find("Engine::PointLightActor", true);

        self->on_create.push([](Object* object) {
            Actor* actor = object->instance_cast<Actor>();
            actor->create_component<SpriteComponent>(("SpriteComponent0"))
                    ->texture(Object::find_object_checked<Texture2D>("Editor::PointLightSprite"));
            actor->create_component<LightHitboxRenderingComponent>("HitboxRendering0")
                    ->component_flags(ActorComponent::DisableRaycast, true);
        });

        self = Class::static_find("Engine::StaticMeshActor", true);
        self->on_create.push([](Object* object) {
            Actor* actor = object->instance_cast<Actor>();
            actor->create_component<PrimitiveHitboxRenderingComponent>("HitboxRendering0")
                    ->component_flags(ActorComponent::DisableRaycast, true);
        });
    }

    static InitializeController on_init(bind_initializers);
}// namespace Engine
