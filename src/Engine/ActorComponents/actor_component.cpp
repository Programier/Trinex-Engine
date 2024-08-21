#include <Core/class.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	static ScriptFunction script_actor_comp_update;
	static ScriptFunction script_actor_comp_start_play;
	static ScriptFunction script_actor_comp_stop_play;
	static ScriptFunction script_actor_comp_spawned;
	static ScriptFunction script_actor_comp_destroyed;

	implement_engine_class(ActorComponent, Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Class*) {
			script_actor_comp_start_play = r->method("void start_play()", &This::start_play);
			script_actor_comp_stop_play  = r->method("void stop_play()", &This::stop_play);
			script_actor_comp_update     = r->method("void update(float dt)", &This::update);
			script_actor_comp_spawned    = r->method("void spawned()", &This::spawned);
			script_actor_comp_destroyed  = r->method("void destroyed()", &This::destroyed);

			r->method("Actor actor() const final", method_of<Actor*>(&This::actor));
			r->method("void actor(Actor actor) const final", method_of<ActorComponent&, Actor*>(&This::actor));
		};
	}

	ActorComponentProxy::ActorComponentProxy()
	{}

	ActorComponentProxy::~ActorComponentProxy()
	{}

	const ScriptFunction& ActorComponent::script_update_func()
	{
		return script_actor_comp_update;
	}

	const ScriptFunction& ActorComponent::script_start_play_func()
	{
		return script_actor_comp_start_play;
	}

	const ScriptFunction& ActorComponent::script_stop_play_func()
	{
		return script_actor_comp_stop_play;
	}

	const ScriptFunction& ActorComponent::script_spawned_func()
	{
		return script_actor_comp_spawned;
	}

	const ScriptFunction& ActorComponent::script_destroyed_func()
	{
		return script_actor_comp_destroyed;
	}

	ActorComponent::ActorComponent() : m_proxy(nullptr)
	{}

	ActorComponent::~ActorComponent()
	{
		destroy_proxy();
	}

	void ActorComponent::destroy_proxy()
	{
		if (m_proxy)
		{
			delete m_proxy;
			m_proxy = nullptr;
		}
	}

	ActorComponent& ActorComponent::start_play()
	{
		return *this;
	}

	ActorComponent& ActorComponent::stop_play()
	{
		return *this;
	}

	ActorComponent& ActorComponent::update(float dt)
	{
		return *this;
	}

	ActorComponent& ActorComponent::spawned()
	{
		m_proxy = create_proxy();
		return *this;
	}

	ActorComponent& ActorComponent::destroyed()
	{
		destroy_proxy();
		return *this;
	}

	ActorComponentProxy* ActorComponent::create_proxy()
	{
		return nullptr;
	}

	ActorComponentProxy* ActorComponent::proxy() const
	{
		return m_proxy;
	}

	class Actor* ActorComponent::actor() const
	{
		return Super::owner()->instance_cast<Actor>();
	}

	class World* ActorComponent::world() const
	{
		Actor* owner_actor = actor();
		return owner_actor ? owner_actor->world() : nullptr;
	}

	class Scene* ActorComponent::scene() const
	{
		Actor* owner_actor = actor();
		return owner_actor ? owner_actor->scene() : nullptr;
	}

	class ActorComponent& ActorComponent::actor(Actor* actor)
	{
		owner(actor);
		return *this;
	}
}// namespace Engine
