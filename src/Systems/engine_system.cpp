#include <Core/base_engine.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/package.hpp>
#include <Core/reflection/class.hpp>
#include <Core/thread.hpp>
#include <Core/tickable.hpp>
#include <Engine/settings.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{
	EngineSystem& EngineSystem::create()
	{
		Super::create();

		Package* package = Package::static_find_package("TrinexEngine::Systems", false);
		if (package)
		{
			package->flags(Object::IsSerializable, false);
		}
		add_reference();
		return *this;
	}

	EngineSystem& EngineSystem::create_systems_from_config()
	{
		for (const String& system_name : Settings::systems)
		{
			System* system = System::system_of(system_name);
			if (system && system->parent_system() == nullptr)
			{
				register_subsystem(system);
			}
		}

		return *this;
	}

	EngineSystem& EngineSystem::update(float dt)
	{
		Super::update(dt);

		for (auto object = TickableObject::static_first(); object; object = object->next())
		{
			if (object->is_tickable())
			{
				object->update(dt);
			}
		}
		return *this;
	}

	trinex_implement_engine_class_default_init(EngineSystem, 0);
}// namespace Engine
