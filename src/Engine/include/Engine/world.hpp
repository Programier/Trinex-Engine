#pragma once
#include <Core/system.hpp>

namespace Engine
{
    class SceneInterface;

    class ENGINE_EXPORT World : public System
    {
        declare_class(World, System);

    private:
        Vector<class Actor*> _M_actors;
        Vector<class Actor*> _M_actors_to_destroy;
        bool _M_is_playing;

        SceneInterface* _M_scene = nullptr;

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
        SceneInterface* scene() const;

        static World* global();
    };
}// namespace Engine
