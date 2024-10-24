#include <Core/base_engine.hpp>
#include <Core/reflection/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/thread.hpp>
#include <Engine/settings.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{
	EngineSystem& EngineSystem::create()
	{
		Super::create();

		Package* package = Package::static_find_package("Engine::Systems", false);
		if (package)
		{
			package->flags(Object::IsSerializable, false);
		}
		add_reference();
		return *this;
	}

	EngineSystem& EngineSystem::create_systems_from_config()
	{
		for (const String& system_name : Settings::e_systems)
		{
			System* system = System::new_system(system_name);
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
		return *this;
	}

	implement_engine_class_default_init(EngineSystem, 0);
}// namespace Engine
