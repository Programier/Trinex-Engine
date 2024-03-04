#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/transform.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{
    class ActorComponent;

    class ENGINE_EXPORT Actor : public Object
    {
        declare_class(Actor, Object);

    private:
        Pointer<class SceneComponent> m_root_component;
        Vector<Pointer<class ActorComponent>> m_owned_components;
        ScriptObject m_script_object;

        class World* m_world      = nullptr;
        bool m_is_playing         = false;
        bool m_is_being_destroyed = false;

    protected:
        ActorComponent* create_component(Class* self, const Name& name = {});
        Actor& add_component(ActorComponent* component);
        Actor& remove_component(ActorComponent* component);

        template<typename ComponentType>
        FORCE_INLINE ComponentType* create_component(const Name& name = {})
        {
            return create_component(ComponentType::static_class_instance(), name)->template instance_cast<ComponentType>();
        }


    public:
        virtual Actor& update(float dt);
        virtual Actor& start_play();
        virtual Actor& stop_play();
        virtual Actor& spawned();
        virtual Actor& destroyed();
        Actor& destroy();

        bool is_playing() const;
        const Vector<Pointer<class ActorComponent>>& owned_components() const;
        Actor& destroy_script_object(ScriptObject* object) override;
        const Transform* transfrom() const;
        SceneComponent* scene_component() const;

        class World* world() const;
        class Scene* scene() const;
        bool archive_process(Archive& archive) override;

        friend class World;
    };
}// namespace Engine
