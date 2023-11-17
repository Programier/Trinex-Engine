#include <Systems/logic_system.hpp>
#include <Core/class.hpp>

namespace Engine
{
    int_t LogicSystem::private_update()
    {
        return 0;
    }

    LogicSystem& LogicSystem::create()
    {
        Super::create();
        return *this;
    }

    LogicSystem& LogicSystem::update(float dt)
    {
        Super::update(dt);
        return *this;
    }

    LogicSystem& LogicSystem::shutdown()
    {
        Super::shutdown();
        return *this;
    }

    LogicSystem& LogicSystem::wait()
    {
        return *this;
    }

    implement_engine_class_default_init(LogicSystem);
}// namespace Engine
