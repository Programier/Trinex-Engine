#include <Core/etl/scope_variable.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/level.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/registrar.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_engine.hpp>
#include <ScriptEngine/script_function.hpp>
#include <angelscript.h>

namespace Engine
{

	static ScriptFunction script_level_start_play;
	static ScriptFunction script_level_update;
	static ScriptFunction script_level_stop_play;

	static ScriptFunction script_level_spawn_actor;
	static ScriptFunction script_level_spawn_actor_t;

	static Actor* scriptable_spawn_actor(Level* level, class Refl::Class* self, const Vector3f& location,
	                                     const Vector3f& rotation, const Vector3f& scale, const Name& name)
	{
		return nullptr;
		//return level->spawn_actor(self, location, rotation, scale, name);
	}

	static void scriptable_spawn_actor_t(asIScriptGeneric* generic) {}

	trinex_implement_class(Engine::Level, Refl::Class::IsScriptable)
	{
		auto self = static_reflection();
		auto r    = ScriptClassRegistrar::existing_class(self);


		static constexpr const char* signatures[] = {
		        "void start_play()",
		        "void update(float dt)",
		        "void stop_play()",
		        "Actor@ spawn_actor(Class@ , const Vector3f&, const Vector3f&, const Vector3f&, const Name&) final",
		        "T@ spawn_actor<T>(const Vector3f&, const Vector3f&, const Vector3f&, const Name&) final",
		};


		script_level_start_play    = r.method(signatures[0], trinex_scoped_void_method(Level, start_play));
		script_level_update        = r.method(signatures[1], trinex_scoped_void_method(Level, update));
		script_level_stop_play     = r.method(signatures[2], trinex_scoped_void_method(Level, stop_play));
		script_level_spawn_actor   = r.method(signatures[3], scriptable_spawn_actor);
		script_level_spawn_actor_t = r.method(signatures[4], scriptable_spawn_actor_t, ScriptCallConv::Generic);

		r.method("bool is_playing() const final", &Level::is_playing);
		r.method("const Vector<Actor@>& actors() const final", &Level::actors);

		ScriptEngine::on_terminate.push([]() {
			script_level_update.release();
			script_level_start_play.release();
			script_level_stop_play.release();
		});
	}

	static thread_local Level* s_current_level = nullptr;

	Level::Level() : m_is_playing(false) {}

	Level::~Level()
	{
		while (!m_actors.empty())
		{
			Actor* actor = m_actors.back();
			actor->owner(nullptr);
		}
	}

	void Level::scriptable_start_play()
	{
		ScriptContext::execute(this, script_level_start_play, nullptr);
	}

	void Level::scriptable_update(float dt)
	{
		ScriptContext::execute(this, script_level_update, nullptr, dt);
	}

	void Level::scriptable_stop_play()
	{
		ScriptContext::execute(this, script_level_stop_play, nullptr);
	}

	bool Level::register_child(Object* child, uint32_t& index)
	{
		Actor* actor = instance_cast<Actor>(child);

		if (actor == nullptr)
			return Super::register_child(child, index);

		index = m_actors.size();
		m_actors.push_back(actor);

		actor->spawned();

		if (is_playing())
		{
			actor->start_play();
		}

		return true;
	}

	bool Level::unregister_child(Object* child)
	{
		Actor* actor = instance_cast<Actor>(child);

		if (actor == nullptr)
			return Super::unregister_child(child);

		if (actor->is_playing())
		{
			actor->stop_play();
		}

		actor->despawned();

		return actor->remove_from(m_actors);
	}

	Level& Level::spawned()
	{
		return *this;
	}

	Level& Level::start_play()
	{
		if (m_is_playing)
			return *this;

		for (size_t index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->start_play();
		}

		m_is_playing = true;

		return *this;
	}

	Level& Level::update(float dt)
	{
		if (!m_is_playing)
			return *this;

		ScopeVariable level_scope(s_current_level, this);

		for (size_t index = 0, count = m_actors.size(); index < count; ++index)
		{
			Actor* actor = m_actors[index];
			if (actor->is_playing())
			{
				m_actors[index]->update(dt);
			}
		}

		return *this;
	}

	Level& Level::stop_play()
	{
		if (!m_is_playing)
			return *this;

		for (size_t index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->stop_play();
		}

		m_is_playing = false;

		return *this;
	}

	Level& Level::despawned()
	{
		return *this;
	}

	World* Level::world()
	{
		return instance_cast<World>(owner());
	}
}// namespace Engine
