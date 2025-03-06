#include <Core/reflection/class.hpp>
#include <Core/thread.hpp>
#include <Systems/engine_system.hpp>
#include <Systems/event_system.hpp>
#include <Systems/logic_system.hpp>

#include <Core/render_resource.hpp>
#include <Graphics/rhi.hpp>

namespace Engine
{
	LogicSystem& LogicSystem::create()
	{
		Super::create();
		EngineSystem::system_of<EngineSystem>()->register_subsystem(this);
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

	class Refl::Class* LogicSystem::depends_on() const
	{
		return EventSystem::static_class_instance();
	}

	trinex_implement_engine_class_default_init(LogicSystem, 0);
}// namespace Engine
