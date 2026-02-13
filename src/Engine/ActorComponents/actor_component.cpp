#include <Core/etl/templates.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/actor_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>

namespace Engine
{
	static ScriptFunction script_actor_comp_update;
	static ScriptFunction script_actor_comp_sync;
	static ScriptFunction script_actor_comp_start_play;
	static ScriptFunction script_actor_comp_stop_play;
	static ScriptFunction script_actor_comp_spawned;
	static ScriptFunction script_actor_comp_despawned;

	trinex_implement_engine_class(ActorComponent, Refl::Class::IsScriptable)
	{
		auto r = ScriptClassRegistrar::existing_class(static_reflection());

		script_actor_comp_start_play = r.method("void start_play()", trinex_scoped_void_method(This, start_play));
		script_actor_comp_stop_play  = r.method("void stop_play()", trinex_scoped_void_method(This, stop_play));
		script_actor_comp_update     = r.method("void update(float dt)", trinex_scoped_void_method(This, update));
		script_actor_comp_sync       = r.method("void sync()", trinex_scoped_void_method(This, sync));
		script_actor_comp_spawned    = r.method("void spawned()", trinex_scoped_void_method(This, spawned));
		script_actor_comp_despawned  = r.method("void despawned()", trinex_scoped_void_method(This, despawned));

		r.method("Actor actor() const final", overload_of<Actor*()>(&This::actor));
		r.method("void actor(Actor actor) const final", overload_of<ActorComponent&()>(&This::actor));

		ScriptEngine::on_terminate.push([]() {
			script_actor_comp_update.release();
			script_actor_comp_sync.release();
			script_actor_comp_start_play.release();
			script_actor_comp_stop_play.release();
			script_actor_comp_spawned.release();
			script_actor_comp_despawned.release();
		});
	}

	void ActorComponent::script_update(float dt)
	{
		ScriptContext::execute(this, script_actor_comp_update, nullptr, dt);
	}

	void ActorComponent::script_sync()
	{
		ScriptContext::execute(this, script_actor_comp_sync, nullptr);
	}

	void ActorComponent::script_start_play()
	{
		ScriptContext::execute(this, script_actor_comp_start_play);
	}

	void ActorComponent::script_stop_play()
	{
		ScriptContext::execute(this, script_actor_comp_stop_play);
	}

	void ActorComponent::script_spawned()
	{
		ScriptContext::execute(this, script_actor_comp_spawned);
	}

	void ActorComponent::script_despawned()
	{
		ScriptContext::execute(this, script_actor_comp_despawned);
	}

	ActorComponent::ActorComponent() {}

	ActorComponent::~ActorComponent() {}

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

	ActorComponent& ActorComponent::sync()
	{
		return *this;
	}

	ActorComponent& ActorComponent::spawned()
	{
		return *this;
	}

	ActorComponent& ActorComponent::despawned()
	{
		return *this;
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
