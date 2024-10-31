#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/transform.hpp>

namespace Engine
{
	class ActorComponent;
	class ScriptFunction;


	class ENGINE_EXPORT Actor : public Object
	{
		declare_class(Actor, Object);

	public:
		template<typename NativeType>
		struct Scriptable : Super::Scriptable<NativeType> {
			Actor& update(float dt) override
			{
				static_cast<Actor*>(this)->scriptable_update(dt);
				return *this;
			}

			Actor& start_play() override
			{
				static_cast<Actor*>(this)->scriptable_start_play();
				return *this;
			}

			Actor& stop_play() override
			{
				static_cast<Actor*>(this)->scriptable_stop_play();
				return *this;
			}

			Actor& spawned() override
			{
				static_cast<Actor*>(this)->scriptable_spawned();
				return *this;
			}

			Actor& destroyed() override
			{
				static_cast<Actor*>(this)->scriptable_destroyed();
				return *this;
			}
		};


		enum Flag
		{
			Selected = BIT(0),
		};

		Flags<Flag, Atomic<BitMask>> actor_flags;

	private:
		Pointer<class SceneComponent> m_root_component;
		Vector<class ActorComponent*> m_owned_components;

		bool m_is_playing         = false;
		bool m_is_being_destroyed = false;
		bool m_is_visible         = true;

		void scriptable_update(float dt);
		void scriptable_start_play();
		void scriptable_stop_play();
		void scriptable_spawned();
		void scriptable_destroyed();

	protected:
		Actor& add_component(ActorComponent* component);
		Actor& remove_component(ActorComponent* component);

	public:
		ActorComponent* create_component(Refl::Class* self, const Name& name = {});

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

		bool is_visible() const;
		Actor& is_visible(bool visible);
		bool is_playing() const;
		bool is_selected() const;

		const Vector<class ActorComponent*>& owned_components() const;
		const Transform& transfrom() const;
		SceneComponent* scene_component() const;

		class World* world() const;
		class Scene* scene() const;
		bool archive_process(Archive& archive) override;

		friend class World;
	};
}// namespace Engine
