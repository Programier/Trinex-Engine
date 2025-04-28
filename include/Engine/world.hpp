#pragma once
#include <Core/callback.hpp>
#include <Core/etl/list.hpp>
#include <Core/etl/set.hpp>
#include <Core/pointer.hpp>
#include <Systems/system.hpp>

namespace Engine
{
	class Scene;

	class ENGINE_EXPORT World : public System
	{
		trinex_declare_class(World, System);

		struct ENGINE_EXPORT DestroyActorInfo {
			Pointer<class Actor> actor;
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

	protected:
		World& create() override;

	public:
		static World* current;

		CallBacks<void(World*, Actor*)> on_actor_select;
		CallBacks<void(World*, Actor*)> on_actor_unselect;

		World& wait() override;
		World& update(float dt) override;
		World& shutdown() override;

		World& start_play();
		World& stop_play();
		Actor* spawn_actor(class Refl::Class* self, const Vector3f& location = {}, const Vector3f& rotation = {},
		                   const Vector3f& scale = {1, 1, 1}, const Name& name = {});

		template<typename T>
		T* spawn_actor(const Vector3f& location = {}, const Vector3f& rotation = {}, const Vector3f& scale = {1, 1, 1},
		               const Name& name = {})
		{
			return instance_cast<T>(spawn_actor(T::static_class_instance(), location, rotation, scale, name));
		}

		World& destroy_actor(Actor* actor);
		Scene* scene() const;
		World& select_actor(Actor* actor);
		World& unselect_actor(Actor* actor);
		World& select_actors(const Vector<Actor*>& actors);
		World& unselect_actors(const Vector<Actor*>& actors);
		World& unselect_actors();
		const TreeSet<Actor*>& selected_actors() const;
		bool is_selected(Actor* actor) const;

		const Vector<class Actor*>& actors() const;
		~World();
	};
}// namespace Engine
