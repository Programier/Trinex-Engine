#pragma once
#include <Systems/system.hpp>

namespace Engine
{
    class Scene;

    class ENGINE_EXPORT World : public System
    {
        declare_class(World, System);

        struct ENGINE_EXPORT DestroyActorInfo {
            class Actor* actor;
            byte skip_frames;
        };

    private:
        Vector<class Actor*> m_actors;
        List<DestroyActorInfo> m_actors_to_destroy;
        TreeSet<class Actor*> m_selected_actors;
        bool m_is_playing;

        Scene* m_scene = nullptr;

        World& destroy_actor(Actor* actor, bool ignore_playing);
        World& destroy_all_actors();

    public:
        World& create() override;
        World& wait() override;
        World& update(float dt) override;
        World& shutdown() override;

        World& start_play();
        World& stop_play();
        Actor* spawn_actor(class Class* self, const Vector3D& location = {}, const Vector3D& rotation = {},
                           const Vector3D& scale = {1, 1, 1}, const Name& name = {});

        World& destroy_actor(Actor* actor);
        Scene* scene() const;
        World& select_actor(Actor* actor);
        World& unselect_actor(Actor* actor);
        World& select_actors(const Vector<Actor*>& actors);
        World& unselect_actors(const Vector<Actor*>& actors);
        World& unselect_actors();
        const TreeSet<Actor*>& selected_actors() const;
        bool is_selected(Actor* actor) const;
        ~World();

        static World* global();
    };
}// namespace Engine