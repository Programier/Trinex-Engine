#pragma once
#include <Core/etl/deque.hpp>
#include <Core/object.hpp>
#include <Core/pointer.hpp>


namespace Engine
{
	class World;

	class ENGINE_EXPORT Level : public Object
	{
		trinex_class(Level, Object);

	private:
		struct ENGINE_EXPORT DestroyActorInfo {
			Pointer<class Actor> actor;
			byte skip_frames;
		};

		Vector<class Actor*> m_actors;
		Deque<DestroyActorInfo> m_actors_to_destroy;
		bool m_is_playing;


	private:
		void scriptable_update(float dt);
		void scriptable_start_play();
		void scriptable_stop_play();

	protected:
		Level& destroy_actor(Actor* actor, bool ignore_playing);
		Level& destroy_all_actors();

		Actor* spawn_actor(Actor* actor, const Vector3f& location = {}, const Vector3f& rotation = {},
		                   const Vector3f& scale = {1, 1, 1});


	public:
		template<typename NativeType>
		struct Scriptable : Super::Scriptable<NativeType> {
			Scriptable& update(float dt) override
			{
				static_cast<Level*>(this)->scriptable_update(dt);
				return *this;
			}

			Scriptable& start_play() override
			{
				static_cast<Level*>(this)->scriptable_start_play();
				return *this;
			}

			Scriptable& stop_play() override
			{
				static_cast<Level*>(this)->scriptable_stop_play();
				return *this;
			}
		};

	public:
		Level();
		~Level();

		virtual Level& start_play();
		virtual Level& stop_play();
		virtual Level& update(float dt);
		virtual World* world();

		Actor* spawn_actor(class Refl::Class* self, const Vector3f& location = {}, const Vector3f& rotation = {},
		                   const Vector3f& scale = {1, 1, 1}, const Name& name = {});

		template<typename T>
		T* spawn_actor(const Vector3f& location = {}, const Vector3f& rotation = {}, const Vector3f& scale = {1, 1, 1},
		               const Name& name = {})
		{
			T* actor = new_instance<T>(name, this);
			return static_cast<T*>(spawn_actor(actor, location, rotation, scale));
		}

		Level& destroy_actor(Actor* actor);

		inline bool is_playing() const { return m_is_playing; }
		inline const Vector<class Actor*>& actors() const { return m_actors; }
	};
}// namespace Engine
