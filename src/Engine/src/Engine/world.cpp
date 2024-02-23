#include <Core/class.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <Systems/logic_system.hpp>

namespace Engine
{
    implement_engine_class_default_init(World);

    World& World::create()
    {
        Super::create();
        LogicSystem::new_system<LogicSystem>()->register_subsystem(this);
        m_is_playing = false;

        m_scene = new Scene();

        return *this;
    }

    World& World::wait()
    {
        Super::wait();
        return *this;
    }

    World& World::update(float dt)
    {
        Super::update(dt);

        if (!m_is_playing)
            return *this;

        for (size_t index = 0, count = m_actors.size(); index < count; ++index)
        {
            m_actors[index]->update(dt);
        }

        if (!m_actors_to_destroy.empty())
        {
            for (size_t index = 0, count = m_actors_to_destroy.size(); index < count; ++index)
            {
                destroy_actor(m_actors_to_destroy[index], true);
            }
            m_actors_to_destroy.clear();
        }

        return *this;
    }

    World& World::shutdown()
    {
        Super::shutdown();
        stop_play();
        delete m_scene;
        return *this;
    }

    World& World::start_play()
    {
        if (m_is_playing)
            return *this;

        for (size_t index = 0; index < m_actors.size(); index++)
        {
            m_actors[index]->start_play();
        }

        m_is_playing = true;
        return *this;
    }

    World& World::stop_play()
    {
        if (!m_is_playing)
            return *this;

        for (size_t index = 0; index < m_actors.size(); index++)
        {
            m_actors[index]->stop_play();
        }

        m_is_playing = true;
        return *this;
    }

    Actor* World::spawn_actor(class Class* self, const Vector3D& location, const Vector3D& rotation, const Name& actor_name)
    {
        if (!self)
            return nullptr;

        Actor* actor = self->create_object()->instance_cast<Actor>();
        if (actor == nullptr)
        {
            throw EngineException("Invalid class for actor!");
        }

        if (actor_name.is_valid())
        {
            actor->name(actor_name);
        }

        {
            SceneComponent* root = actor->scene_component();
            if (root)
            {
                root->transform.location = location;
                root->transform.rotation = rotation;
                m_scene->root_component()->attach(root);
                actor->scene_component()->on_transform_changed();
            }
        }
        actor->m_world = this;

        actor->spawned();

        if (m_is_playing)
        {
            actor->start_play();
        }

        m_actors.push_back(actor);
        return actor;
    }

    World& World::destroy_actor(Actor* actor, bool ignore_playing)
    {
        if (!actor || actor->world() != this)
            return *this;

        if (!ignore_playing && actor->is_playing())
        {
            m_actors_to_destroy.push_back(actor);
            return *this;
        }

        actor->destroy();

        for (size_t index = 0, count = m_actors.size(); index < count; ++index)
        {
            if (m_actors[index] == actor)
            {
                m_actors.erase(m_actors.begin() + index);
                break;
            }
        }

        return *this;
    }

    World& World::destroy_actor(Actor* actor)
    {
        return destroy_actor(actor, false);
    }

    Scene* World::scene() const
    {
        return m_scene;
    }

    World::~World()
    {
        if (!is_shutdowned())
        {
            shutdown();
        }
    }

    World* World::global()
    {
        static World* global_world = nullptr;
        if (global_world == nullptr)
        {
            System* system = Object::find_object_checked<System>("Engine::Systems::EngineSystem");
            if (!system)
                return nullptr;
            system = system->find_subsystem("LogicSystem::Global World");
            if (!system)
                return nullptr;

            global_world = system->instance_cast<World>();
        }

        return global_world;
    }
}// namespace Engine
