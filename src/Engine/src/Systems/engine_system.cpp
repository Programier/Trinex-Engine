#include <Core/base_engine.hpp>
#include <Core/class.hpp>
#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Core/thread.hpp>
#include <Graphics/rhi.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{
    EngineSystem& EngineSystem::create()
    {
        Super::create();

        Package* package = Package::find_package("Engine::Systems", false);
        if (package)
        {
            package->flags(Object::IsSerializable, false);
        }
        add_reference();
        return *this;
    }

    EngineSystem& EngineSystem::create_systems_from_config()
    {
        Vector<String> systems = ConfigManager::get_string_array("Engine::systems");

        for (const String& system_name : systems)
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

    implement_class(EngineSystem, Engine, 0);
    implement_default_initialize_class(EngineSystem);
}// namespace Engine
