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

    public:
        enum Flag
        {
            Selected = BIT(0),
        };

        Flags<Flag> actor_flags;

    private:
        Pointer<class SceneComponent> m_root_component;
        Vector<class ActorComponent*> m_owned_components;
        ScriptObject m_script_object;

        class World* m_world      = nullptr;
        bool m_is_playing         = false;
        bool m_is_being_destroyed = false;

    protected:
        Actor& add_component(ActorComponent* component);
        Actor& remove_component(ActorComponent* component);

    public:
        ActorComponent* create_component(Class* self, const Name& name = {});

        template<typename ComponentType>
        FORCE_INLINE ComponentType* create_component(const Name& name = {})
        {
            return create_component(ComponentType::static_class_instance(), name)->template instance_cast<ComponentType>();
        }

        virtual Actor& update(float dt);
        virtual Actor& start_play();
        virtual Actor& stop_play();
        virtual Actor& spawned();
        virtual Actor& destroyed();
        Actor& destroy();
        Actor& update_drawing_data();

        bool is_playing() const;
        bool is_selected() const;

        const Vector<class ActorComponent*>& owned_components() const;
        Actor& destroy_script_object(ScriptObject* object) override;
        const Transform& transfrom() const;
        SceneComponent* scene_component() const;

        class World* world() const;
        class Scene* scene() const;
        bool archive_process(Archive& archive) override;

        friend class World;
    };
}// namespace Engine
