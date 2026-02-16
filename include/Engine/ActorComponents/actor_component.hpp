#pragma once
#include <Core/flags.hpp>
#include <Core/object.hpp>


namespace Engine
{
	class Actor;
	class ScriptFunction;

	class ENGINE_EXPORT ActorComponent : public Object
	{
		trinex_class(ActorComponent, Object);

	private:
		bool m_is_playing = false;

	private:
		void script_update(float dt);
		void script_sync();
		void script_start_play();
		void script_stop_play();
		void script_spawned();
		void script_despawned();

	public:
		template<typename NativeType>
		struct Scriptable : public Super::Scriptable<NativeType> {
			Scriptable& spawned() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_spawned();
				return *this;
			}

			Scriptable& start_play() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_start_play();
				return *this;
			}

			Scriptable& update(float dt) override
			{
				reinterpret_cast<ActorComponent*>(this)->script_update(dt);
				return *this;
			}

			Scriptable& stop_play() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_stop_play();
				return *this;
			}

			Scriptable& despawned() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_despawned();
				return *this;
			}

			Scriptable& sync() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_sync();
				return *this;
			}
		};

		ActorComponent();
		~ActorComponent();

		virtual ActorComponent& spawned();
		virtual ActorComponent& start_play();
		virtual ActorComponent& update(float dt);
		virtual ActorComponent& stop_play();
		virtual ActorComponent& despawned();

		virtual ActorComponent& sync();

		inline bool is_playing() const { return m_is_playing; }

		class Actor* actor() const;
		class World* world() const;
		class Scene* scene() const;
		class ActorComponent& actor(Actor* actor);
	};
}// namespace Engine
