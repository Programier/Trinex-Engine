#include <Core/class.hpp>
#include <Core/thread.hpp>
#include <Systems/event_system.hpp>
#include <Systems/logic_system.hpp>

#include <Core/render_resource.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
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

    class Class* LogicSystem::depends_on() const
    {
        return EventSystem::static_class_instance();
    }

    implement_engine_class_default_init(LogicSystem);
}// namespace Engine
