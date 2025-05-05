#pragma once
#include <Core/flags.hpp>
#include <Core/object.hpp>


namespace Engine
{
	class Actor;
	class ScriptFunction;

	class ENGINE_EXPORT ActorComponent : public Object
	{
		trinex_declare_class(ActorComponent, Object);

	public:
		class ENGINE_EXPORT Proxy{public: virtual ~Proxy(){}};

	private:
		Proxy* m_proxy;

		void destroy_proxy();

		void script_update(float dt);
		void script_start_play();
		void script_stop_play();
		void script_spawned();
		void script_destroyed();

	public:
		template<typename NativeType>
		struct Scriptable : public Super::Scriptable<NativeType> {
			Scriptable& start_play() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_start_play();
				return *this;
			}

			Scriptable& stop_play() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_stop_play();
				return *this;
			}

			Scriptable& update(float dt) override
			{
				reinterpret_cast<ActorComponent*>(this)->script_update(dt);
				return *this;
			}

			Scriptable& spawned() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_spawned();
				return *this;
			}

			Scriptable& destroyed() override
			{
				reinterpret_cast<ActorComponent*>(this)->script_destroyed();
				return *this;
			}
		};


		enum Flag
		{
			DisableRaycast = BIT(0),
		};

		Flags<Flag, Atomic<BitMask>> component_flags;

		ActorComponent();
		~ActorComponent();

		virtual ActorComponent& start_play();
		virtual ActorComponent& stop_play();
		virtual ActorComponent& update(float dt);
		virtual ActorComponent& spawned();
		virtual ActorComponent& destroyed();

		virtual Proxy* create_proxy();
		Proxy* proxy() const;

		template<typename ProxyType>
		ProxyType* typed_proxy() const
		{
			return reinterpret_cast<ProxyType*>(proxy());
		}

		class Actor* actor() const;
		class World* world() const;
		class Scene* scene() const;
		class ActorComponent& actor(Actor* actor);
	};
}// namespace Engine
