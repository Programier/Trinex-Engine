#include <Core/class.hpp>
#include <Core/engine.hpp>
#include <Core/engine_config.hpp>
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

        for (const String& system_name : engine_config.systems)
        {
            System* system = System::new_system_by_name(system_name);
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

    implement_class(EngineSystem, "Engine", 0);
    implement_default_initialize_class(EngineSystem);
}// namespace Engine
