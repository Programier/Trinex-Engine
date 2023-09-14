#include <Core/class.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <Core/etl/singletone.hpp>
#include <Systems/engine_system.hpp>

namespace Engine
{
    EngineSystem* EngineSystem::_M_instance = nullptr;


    implement_class(EngineSystem, "Engine");
    implement_default_initialize_class(EngineSystem);
}// namespace Engine
