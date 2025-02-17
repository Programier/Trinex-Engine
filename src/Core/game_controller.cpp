#include <Core/engine_loading_controllers.hpp>
#include <Core/event.hpp>
#include <Core/game_controller.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/registrar.hpp>
#include <Systems/game_controller_system.hpp>

namespace Engine
{
	GameController::GameController(Identifier controller_id) : m_ID(controller_id)
	{
		info_log("GameController", "Created game controller %zu", controller_id);
		std::fill(m_axis_values, m_axis_values + Axis::__COUNT__, 0);
	}

	void GameController::axis_motion_listener(const Event& e)
	{
		m_axis_values[e.gamepad.axis_motion.axis] = e.gamepad.axis_motion.value;
		on_axis_motion(this, e);
	}

	void GameController::controller_removed_listener()
	{
		on_destroy(this);
		info_log("GameController", "Removed game controller %zu", m_ID);
	}

	Identifier GameController::id() const
	{
		return m_ID;
	}

	float GameController::axis_value(Axis axis, float dead_zone) const
	{
		float result = m_axis_values[axis];
		if (glm::abs(result) >= glm::abs(dead_zone))
			return result;
		return 0.f;
	}

	GameController* GameController::find(Identifier id)
	{
		if (auto system = GameControllerSystem::instance())
		{
			return system->controller(id);
		}
		return nullptr;
	}

	static void on_init()
	{

		{
			ScriptEnumRegistrar axis_enum("Engine::GameController::Axis");
			axis_enum.set("Unknown", GameController::Unknown);
			axis_enum.set("LeftX", GameController::LeftX);
			axis_enum.set("LeftY", GameController::LeftY);
			axis_enum.set("RightX", GameController::RightX);
			axis_enum.set("RightY", GameController::RightY);
			axis_enum.set("TriggerLeft", GameController::TriggerLeft);
			axis_enum.set("TriggerRight", GameController::TriggerRight);
			axis_enum.set("__COUNT__", GameController::__COUNT__);
		}


		ScriptClassRegistrar::RefInfo info;
		info.implicit_handle = true;
		info.no_count        = true;

		ScriptClassRegistrar r = ScriptClassRegistrar::reference_class("Engine::GameController", info);

		r.static_function("GameController find(uint64 id)", &GameController::find);
		r.method("uint64 id() const", &GameController::id);
		r.method("float axis_value(Engine::GameController::Axis, float dead_zone = 0.f) const", &GameController::axis_value);
	}

	static ReflectionInitializeController init(on_init, "Engine::GameController");
}// namespace Engine
