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

namespace Engine
{
	trinex_implement_engine_class_default_init(World, 0);

	static thread_local World* s_current_world = nullptr;

	World::World() : m_scene(trx_new Scene()) {}

	World::~World()
	{
		stop_play();
		destroy_all_actors();
		trx_delete m_scene;
	}

	World* World::current()
	{
		return s_current_world;
	}

	World& World::update(float dt)
	{
		ScopeVariable scope_world(s_current_world, this);

		Super::update(dt);

		for (Level* level : m_levels)
		{
			level->update(dt);
		}

		return *this;
	}

	World* World::world()
	{
		return this;
	}

	bool World::register_child(Object* child)
	{
		if (Level* level = instance_cast<Level>(child))
		{
			for (size_t i = 0; i < m_levels.size(); ++i)
			{
				if (m_levels[i] == level)
				{
					m_levels.erase(m_levels.begin() + i);
					break;
				}
			}
			return true;
		}

		return Super::register_child(child);
	}

	bool World::unregister_child(Object* child)
	{
		if (Level* level = instance_cast<Level>(child))
		{
			m_levels.push_back(level);
			return true;
		}

		return Super::unregister_child(child);
	}

	World& World::add_level(Level* level)
	{
		return *this;
	}

	World& World::remove_level(Level* level)
	{
		return *this;
	}
}// namespace Engine
