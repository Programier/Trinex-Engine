#include <Core/archive.hpp>
#include <Core/buffer_manager.hpp>
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

namespace Trinex
{

	static ScriptFunction script_level_start_play;
	static ScriptFunction script_level_update;
	static ScriptFunction script_level_stop_play;

	static ScriptFunction script_level_spawn_actor;
	static ScriptFunction script_level_spawn_actor_t;

	static Actor* scriptable_spawn_actor(LevelInstance* level, class Refl::Class* self, const Vector3f& location,
	                                     const Vector3f& rotation, const Vector3f& scale, const Name& name)
	{
		return nullptr;
		//return level->spawn_actor(self, location, rotation, scale, name);
	}

	static void scriptable_spawn_actor_t(asIScriptGeneric* generic) {}

	trinex_implement_class(Trinex::LevelInstance, Refl::Class::IsScriptable)
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


		script_level_start_play    = r.method(signatures[0], trinex_scoped_void_method(LevelInstance, start_play));
		script_level_update        = r.method(signatures[1], trinex_scoped_void_method(LevelInstance, update));
		script_level_stop_play     = r.method(signatures[2], trinex_scoped_void_method(LevelInstance, stop_play));
		script_level_spawn_actor   = r.method(signatures[3], scriptable_spawn_actor);
		script_level_spawn_actor_t = r.method(signatures[4], scriptable_spawn_actor_t, ScriptCallConv::Generic);

		r.method("bool is_playing() const final", &LevelInstance::is_playing);
		r.method("const Vector<Actor@>& actors() const final", &LevelInstance::actors);

		ScriptEngine::on_terminate.push([]() {
			script_level_update.release();
			script_level_start_play.release();
			script_level_stop_play.release();
		});
	}

	static thread_local LevelInstance* s_current_level = nullptr;
	static thread_local Level* s_next_level            = nullptr;

	LevelInstance::LevelInstance() : m_level(s_next_level), m_is_playing(false), m_is_visible(true) {}

	LevelInstance::~LevelInstance()
	{
		while (!m_actors.empty())
		{
			Actor* actor = m_actors.back();
			actor->owner(nullptr);
		}
	}

	void LevelInstance::scriptable_start_play()
	{
		ScriptContext::execute(this, script_level_start_play, nullptr);
	}

	void LevelInstance::scriptable_update(float dt)
	{
		ScriptContext::execute(this, script_level_update, nullptr, dt);
	}

	void LevelInstance::scriptable_stop_play()
	{
		ScriptContext::execute(this, script_level_stop_play, nullptr);
	}

	Object* LevelInstance::register_child(Object* child, u32& index)
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

		return this;
	}

	bool LevelInstance::unregister_child(Object* child)
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

	LevelInstance& LevelInstance::spawned()
	{
		return *this;
	}

	LevelInstance& LevelInstance::start_play()
	{
		if (m_is_playing)
			return *this;

		for (usize index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->start_play();
		}

		m_is_playing = true;

		return *this;
	}

	LevelInstance& LevelInstance::update(float dt)
	{
		if (!m_is_playing)
			return *this;

		ScopeVariable level_scope(s_current_level, this);

		for (usize index = 0, count = m_actors.size(); index < count; ++index)
		{
			Actor* actor = m_actors[index];
			if (actor->is_playing())
			{
				m_actors[index]->update(dt);
			}
		}

		return *this;
	}

	LevelInstance& LevelInstance::stop_play()
	{
		if (!m_is_playing)
			return *this;

		for (usize index = 0; index < m_actors.size(); index++)
		{
			m_actors[index]->stop_play();
		}

		m_is_playing = false;

		return *this;
	}

	LevelInstance& LevelInstance::despawned()
	{
		if (is_playing())
		{
			stop_play();
		}

		for (usize index = 0, count = m_actors.size(); index < count; ++index)
		{
			Actor* actor = m_actors[index];
			actor->despawned();
		}

		return *this;
	}

	World* LevelInstance::world()
	{
		return instance_cast<World>(owner());
	}

	bool LevelInstance::serialize(Archive& archive)
	{
		if (!Super::serialize(archive))
			return false;

		return archive.serialize_childs(this, m_actors.begin(), m_actors.end());
	}

	trinex_implement_class(Trinex::Level, Refl::Class::IsScriptable | Refl::Class::IsAsset) {}

	Level::Level() : m_class(LevelInstance::static_reflection()) {}

	LevelInstance* Level::create_instance(StringView name, Object* owner)
	{
		ScopeVariable scope(s_next_level, this);

		if (name.empty())
			name = Level::name();

		LevelInstance* instance = nullptr;

		if (m_state.empty())
		{
			return instance_cast<LevelInstance>(m_class->create_object(name, owner));
		}
		else
		{
			VectorReader reader = &m_state;
			Archive archive     = &reader;
			archive.serialize_object(instance, name, owner);
		}

		return instance;
	}

	bool Level::update(LevelInstance* instance)
	{
		trinex_assert(instance);

		Buffer state;
		VectorWriter writer = &state;
		Archive archive     = &writer;

		if (archive.serialize_object(instance))
		{
			m_class = instance->class_instance();
			m_state.swap(state);
			return true;
		}

		return false;
	}

	bool Level::serialize(Archive& ar)
	{
		if (!Super::serialize(ar))
			return false;

		return ar.serialize_memory(m_state);
	}
}// namespace Trinex
