#pragma once
#include <Systems/system.hpp>

namespace Engine
{
    class Scene;

    class ENGINE_EXPORT World : public System
    {
        declare_class(World, System);

    private:
        Vector<class Actor*> m_actors;
        Vector<class Actor*> m_actors_to_destroy;
        bool m_is_playing;

        Scene* m_scene = nullptr;

        World& destroy_actor(Actor* actor, bool ignore_playing);

    public:
        World& create() override;
        World& wait() override;
        World& update(float dt) override;
        World& shutdown() override;

        World& start_play();
        World& stop_play();
        Actor* spawn_actor(class Class* self, const Vector3D& location = {}, const Vector3D& rotation = {},
                           const Name& name = {});

        World& destroy_actor(Actor* actor);
        Scene* scene() const;
        ~World();

        static World* global();
    };
}// namespace Engine
