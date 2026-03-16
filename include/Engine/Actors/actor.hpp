#pragma once
#include <Core/object.hpp>
#include <Core/pointer.hpp>
#include <Core/transform.hpp>

namespace Trinex
{
	class ActorComponent;
	class ScriptFunction;
	class World;
	class LevelInstance;
	class Scene;

	class ENGINE_EXPORT Actor : public Object
	{
		trinex_class(Actor, Object);

	public:
		template<typename NativeType>
		struct Scriptable : Super::Scriptable<NativeType> {
			Scriptable& update(float dt) override
			{
				static_cast<Actor*>(this)->scriptable_update(dt);
				return *this;
			}

			Scriptable& start_play() override
			{
				static_cast<Actor*>(this)->scriptable_start_play();
				return *this;
			}

			Scriptable& stop_play() override
			{
				static_cast<Actor*>(this)->scriptable_stop_play();
				return *this;
			}

			Scriptable& spawned() override
			{
				static_cast<Actor*>(this)->scriptable_spawned();
				return *this;
			}

			Scriptable& despawned() override
			{
				static_cast<Actor*>(this)->scriptable_despawned();
				return *this;
			}
		};

	private:
		Pointer<class SceneComponent> m_root_component;
		Vector<class ActorComponent*> m_components;

		bool m_is_playing : 1;
		bool m_is_visible : 1;

		void scriptable_update(float dt);
		void scriptable_start_play();
		void scriptable_stop_play();
		void scriptable_spawned();
		void scriptable_despawned();

	protected:
		Object* register_child(Object* child, u32& index) override;
		bool unregister_child(Object* child) override;

	public:
		Actor();
		~Actor();

		using Super::new_instance;

		static Actor* new_instance(Refl::Class* self, const Vector3f& location = {0.f, 0.f, 0.f},
		                           const Quaternion& rotation = {0.f, 0.f, 0.f, 1.f}, const Vector3f& scale = {1.f, 1.f, 1.f},
		                           const Name& name = Name::none);

		virtual Actor& spawned();
		virtual Actor& start_play();
		virtual Actor& update(float dt);
		virtual Actor& stop_play();
		virtual Actor& despawned();

		bool is_visible() const;
		Actor& is_visible(bool visible);
		bool is_playing() const;

		inline const Vector<class ActorComponent*>& components() const { return m_components; }
		const Transform& transfrom() const;
		SceneComponent* scene_component() const;

		LevelInstance* level() const;
		World* world() const;
		Scene* scene() const;
		bool serialize(Archive& archive) override;

		friend class World;
	};
}// namespace Trinex
