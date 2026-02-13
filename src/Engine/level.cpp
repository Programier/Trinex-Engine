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
	static ScriptFunction script_level_update;
	static ScriptFunction script_level_start_play;
	static ScriptFunction script_level_stop_play;
	static ScriptFunction script_level_spawn_actor;
	static ScriptFunction script_level_spawn_actor_t;

	static Actor* scriptable_spawn_actor(Level* level, class Refl::Class* self, const Vector3f& location,
	                                     const Vector3f& rotation, const Vector3f& scale, const Name& name)
	{
		return level->spawn_actor(self, location, rotation, scale, name);
	}

	static void scriptable_spawn_actor_t(asIScriptGeneric* generic) {}

	trinex_implement_class(Engine::Level, Refl::Class::IsScriptable)
	{
		auto self = static_reflection();
		auto r    = ScriptClassRegistrar::existing_class(self);


		static constexpr const char* signatures[] = {
		        "void update(float dt)",
		        "void start_play()",
		        "void stop_play()",
		        "Actor@ spawn_actor(Class@ , const Vector3f&, const Vector3f&, const Vector3f&, const Name&) final",
		        "T@ spawn_actor<T>(const Vector3f&, const Vector3f&, const Vector3f&, const Name&) final",
		};

		script_level_update        = r.method(signatures[0], trinex_scoped_void_method(Level, update));
		script_level_start_play    = r.method(signatures[1], trinex_scoped_void_method(Level, start_play));
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
		stop_play();
		destroy_all_actors();
	}

	void Level::scriptable_update(float dt)
	{
		ScriptContext::execute(this, script_level_update, nullptr, dt);
	}

	void Level::scriptable_start_play()
	{
		ScriptContext::execute(this, script_level_start_play, nullptr);
	}

	void Level::scriptable_stop_play()
	{
		ScriptContext::execute(this, script_level_stop_play, nullptr);
	}

	Level& Level::destroy_actor(Actor* actor, bool ignore_playing)
	{
		if (!actor || actor->level() != this)
			return *this;

		if (!ignore_playing && actor->is_playing())
		{
			actor->stop_play();
			// Perhaps the method will be called before World::update, so we skip one frame and only then delete the actor
			DestroyActorInfo info;
			info.actor       = actor;
			info.skip_frames = 1;
			m_actors_to_destroy.push_back(info);
			return *this;
		}


		if (actor->is_playing())
			actor->stop_play();

		actor->despawned();
		actor->owner(nullptr);

		for (size_t index = 0, count = m_actors.size(); index < count; ++index)
		{
			if (m_actors[index] == actor)
			{
				m_actors.erase(m_actors.begin() + index);
				break;
			}
		}

		return *this;
	}

	Level& Level::destroy_all_actors()
	{
		for (auto& info : m_actors_to_destroy)
		{
			destroy_actor(info.actor, true);
		}

		m_actors_to_destroy.clear();

		while (!m_actors.empty())
		{
			destroy_actor(m_actors.front(), true);
		}

		return *this;
	}

	Actor* Level::spawn_actor(Actor* actor, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale)
	{
		if (actor == nullptr)
		{
			error_log("World", "Failed to allocate actor!");
			return nullptr;
		}

		actor->spawned();

		{
			SceneComponent* root = actor->scene_component();
			if (root)
			{
				root->location(location);
				root->rotation(rotation);
				root->scale(scale);
			}
		}

		if (m_is_playing)
		{
			actor->start_play();
		}

		m_actors.push_back(actor);
		return actor;
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

	Level& Level::update(float dt)
	{
		if (!m_is_playing)
			return *this;

		ScopeVariable level_scope(s_current_level, this);

		if (!m_actors_to_destroy.empty())
		{
			while (!m_actors_to_destroy.empty())
			{
				auto& front = m_actors_to_destroy.front();

				if (front.skip_frames == 0)
				{
					destroy_actor(front.actor, true);
					m_actors_to_destroy.pop_front();
				}
				else
					break;
			}

			for (auto& actor_info : m_actors_to_destroy)
			{
				--actor_info.skip_frames;
			}
		}

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

	World* Level::world()
	{
		return instance_cast<World>(owner());
	}

	Actor* Level::spawn_actor(class Refl::Class* self, const Vector3f& location, const Vector3f& rotation, const Vector3f& scale,
	                          const Name& actor_name)
	{
		if (!self)
		{
			error_log("World", "Failed to create actor, because class instance is nullptr");
			return nullptr;
		}

		if (!self->is_a(Actor::static_reflection()))
		{
			error_log("World", "Failed to create actor from non-Actor class '%s'!", self->name().c_str());
			return nullptr;
		}

		Actor* actor = self->create_object(actor_name, this)->instance_cast<Actor>();
		return spawn_actor(actor, location, rotation, scale);
	}

	Level& Level::destroy_actor(Actor* actor)
	{
		return destroy_actor(actor, false);
	}
}// namespace Engine
