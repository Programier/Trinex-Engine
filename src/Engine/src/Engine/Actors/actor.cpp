#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
#include <Core/class.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>

namespace Engine
{
    ActorComponent* Actor::create_component(Class* self, const Name& component_name)
    {
        if (self == nullptr)
            return nullptr;


        ActorComponent* component_object = self->create_object()->instance_cast<ActorComponent>();
        if (!component_object)
            throw EngineException("Cannot create actor component from non component class!");
        component_object->name(component_name);

        add_component(component_object);
        return component_object;
    }

    Actor& Actor::add_component(ActorComponent* component)
    {
        {
            Actor* owner = component->actor();
            if (owner)
            {
                owner->remove_component(component);
            }
        }

        component->owner(this);

        {
            SceneComponent* scene_component = component->instance_cast<SceneComponent>();
            if (!_M_root_component && scene_component)
            {
                _M_root_component = scene_component;
            }
            else if (_M_root_component && scene_component)
            {
                _M_root_component->attach(scene_component);
            }
        }

        _M_owned_components.push_back(component);
        return *this;
    }

    Actor& Actor::remove_component(ActorComponent* component)
    {
        for (size_t i = 0, count = _M_owned_components.size(); i < count; i++)
        {
            ActorComponent* actor_component = _M_owned_components[i].ptr();
            if (actor_component == component)
            {
                if (_M_root_component == component)
                {
                    _M_root_component = nullptr;
                }


                component->owner(nullptr);
                _M_owned_components.erase(_M_owned_components.begin() + i);
                break;
            }
        }
        return *this;
    }


    Actor& Actor::update(float dt)
    {
        _M_script_object.update(dt);
        return *this;
    }

    Actor& Actor::start_play()
    {
        _M_is_playing = true;
        return *this;
    }

    Actor& Actor::stop_play()
    {
        _M_is_playing = false;
        return *this;
    }

    bool Actor::is_playing() const
    {
        return _M_is_playing;
    }

    Actor& Actor::spawned()
    {
        return *this;
    }

    Actor& Actor::destroyed()
    {
        return *this;
    }

    Actor& Actor::destroy_script_object(ScriptObject* object)
    {
        if (*object == _M_script_object)
        {
            _M_script_object.remove_reference();
        }

        return *this;
    }

    Transform* Actor::transfrom() const
    {
        return _M_root_component ? &_M_root_component->transform : nullptr;
    }

    SceneComponent* Actor::scene_component() const
    {
        return _M_root_component.ptr();
    }

    const Vector<Pointer<class ActorComponent>>& Actor::owned_components() const
    {
        return _M_owned_components;
    }

    class World* Actor::world() const
    {
        return _M_world;
    }

    bool Actor::archive_process(Archive* archive)
    {
        if (!Super::archive_process(archive))
            return false;
        return static_cast<bool>(*archive);
    }

    implement_class(Actor, Engine, 0);
    implement_initialize_class(Actor)
    {}
}// namespace Engine
