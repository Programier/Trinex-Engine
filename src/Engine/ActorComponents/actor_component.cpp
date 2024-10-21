#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_object.hpp>

namespace Engine
{
	static ScriptFunction script_actor_comp_update;
	static ScriptFunction script_actor_comp_start_play;
	static ScriptFunction script_actor_comp_stop_play;
	static ScriptFunction script_actor_comp_spawned;
	static ScriptFunction script_actor_comp_destroyed;

	implement_engine_class(ActorComponent, Refl::Class::IsScriptable)
	{
		static_class_instance()->script_registration_callback = [](ScriptClassRegistrar* r, Refl::Class*) {
			script_actor_comp_start_play = r->method("void start_play()", &This::scoped_start_play<ActorComponent>);
			script_actor_comp_stop_play  = r->method("void stop_play()", &This::scoped_stop_play<ActorComponent>);
			script_actor_comp_update     = r->method("void update(float dt)", &This::scoped_update<ActorComponent>);
			script_actor_comp_spawned    = r->method("void spawned()", &This::scoped_spawned<ActorComponent>);
			script_actor_comp_destroyed  = r->method("void destroyed()", &This::scoped_destroyed<ActorComponent>);

			r->method("Actor actor() const final", method_of<Actor*>(&This::actor));
			r->method("void actor(Actor actor) const final", method_of<ActorComponent&, Actor*>(&This::actor));
		};

		ScriptEngine::on_terminate.push([]() {
			script_actor_comp_update.release();
			script_actor_comp_start_play.release();
			script_actor_comp_stop_play.release();
			script_actor_comp_spawned.release();
			script_actor_comp_destroyed.release();
		});
	}

	ActorComponentProxy::ActorComponentProxy()
	{}

	ActorComponentProxy::~ActorComponentProxy()
	{}

	void ActorComponent::script_update(float dt)
	{
		ScriptObject(this).execute(script_actor_comp_update, dt);
	}

	void ActorComponent::script_start_play()
	{
		ScriptObject(this).execute(script_actor_comp_start_play);
	}

	void ActorComponent::script_stop_play()
	{
		ScriptObject(this).execute(script_actor_comp_stop_play);
	}

	void ActorComponent::script_spawned()
	{
		ScriptObject(this).execute(script_actor_comp_spawned);
	}

	void ActorComponent::script_destroyed()
	{
		ScriptObject(this).execute(script_actor_comp_destroyed);
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
		return instance_cast<Actor>(Super::owner());
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
