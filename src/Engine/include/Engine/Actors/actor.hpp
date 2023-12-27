#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{

    class ENGINE_EXPORT Actor : public Object
    {
        declare_class(Actor, Object);

    private:
        Pointer<class SceneComponent> _M_root_component;
        Vector<Pointer<class ActorComponent>> _M_owned_components;
        ScriptObject _M_script_object;

        class World* _M_world = nullptr;
        bool _M_is_playing    = false;


    public:
        virtual Actor& update(float dt);
        virtual Actor& start_play();
        virtual Actor& stop_play();
        bool is_playing() const;
        virtual Actor& spawned();
        virtual Actor& destroyed();
        Actor& destroy_script_object(ScriptObject* object) override;
        Transform* transfrom() const;
        SceneComponent* scene_component() const;
        const Vector<Pointer<class ActorComponent>>& owned_components() const;

        class World* world() const;
        bool archive_process(Archive* archive) override;

        friend class World;
    };
}// namespace Engine
