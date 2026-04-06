#include <Core/etl/scope_variable.hpp>
#include <Core/logger.hpp>
#include <Core/reflection/class.hpp>
#include <Core/threading.hpp>
#include <Engine/ActorComponents/scene_component.hpp>
#include <Engine/Actors/actor.hpp>
#include <Engine/scene.hpp>
#include <Engine/world.hpp>
#include <ScriptEngine/script_context.hpp>
#include <ScriptEngine/script_object.hpp>
#include <Systems/logic_system.hpp>

namespace Trinex
{
	trinex_implement_engine_class_default_init(World, 0);

	static thread_local World* s_current_world = nullptr;

	World::World() : m_active(this), m_scene(trx_new Scene()) {}

	World::~World()
	{
		stop_play();

		while (!m_levels.empty())
		{
			LevelInstance* level = m_levels.back();
			level->owner(nullptr);
		}

		trx_delete m_scene;
	}

	World* World::current()
	{
		return s_current_world;
	}

	World& World::start_play()
	{
		if (is_playing())
			return *this;

		Super::start_play();

		for (LevelInstance* level : m_levels)
		{
			level->start_play();
		}

		return *this;
	}

	World& World::update(float dt)
	{
		ScopeVariable scope_world(s_current_world, this);
		Super::update(dt);

		for (LevelInstance* level : m_levels)
		{
			level->update(dt);
		}

		return *this;
	}

	World& World::stop_play()
	{
		if (!is_playing())
			return *this;

		for (LevelInstance* level : m_levels)
		{
			level->stop_play();
		}

		Super::stop_play();
		return *this;
	}

	World* World::world()
	{
		return this;
	}

	World& World::active_level(LevelInstance* instance)
	{
		if (instance->owner() == this || instance == this)
		{
			m_active = instance;
		}

		return *this;
	}

	Object* World::register_child(Object* child, u32& index)
	{
		LevelInstance* level = instance_cast<LevelInstance>(child);

		if (level == nullptr)
		{
			if (m_active != this && instance_cast<Actor>(child))
			{
				// Redirect to active level
				return m_active;
			}

			return Super::register_child(child, index);
		}

		index = m_levels.size();
		m_levels.push_back(level);

		level->spawned();

		if (is_playing())
		{
			level->start_play();
		}
		return this;
	}

	bool World::unregister_child(Object* child)
	{
		LevelInstance* level = instance_cast<LevelInstance>(child);

		if (level == nullptr)
			return Super::unregister_child(child);

		if (m_active == child)
			m_active = this;

		if (level->is_playing())
		{
			level->stop_play();
		}

		level->despawned();

		return level->remove_from(m_levels);
	}
}// namespace Trinex
