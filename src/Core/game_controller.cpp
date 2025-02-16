#include <Core/engine_loading_controllers.hpp>
#include <Core/game_controller.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/registrar.hpp>

namespace Engine
{
	GameController::GameController(Identifier controller_id) : m_ID(controller_id)
	{
		info_log("GameController", "Created game controller %zu", controller_id);
		std::fill(m_axis_values, m_axis_values + Axis::__COUNT__, 0);
	}

	Identifier GameController::id() const
	{
		return m_ID;
	}

	GameController::~GameController()
	{
		info_log("GameController", "Removed game controller %zu", m_ID);
	}

	short_t GameController::axis_value(Axis axis) const
	{
		return m_axis_values[axis];
	}

	float GameController::axis_value_normalized(Axis axis) const
	{
		return static_cast<float>(m_axis_values[axis]) / static_cast<float>(std::numeric_limits<short_t>::max());
	}

	static void on_init()
	{
		ScriptClassRegistrar registrar = ScriptClassRegistrar::reference_class("Engine::GameController");
		ScriptEnumRegistrar axis_enum("Engine::GameController::Axis");

		axis_enum.set("Unknown", GameController::Unknown);
		axis_enum.set("LeftX", GameController::LeftX);
		axis_enum.set("LeftY", GameController::LeftY);
		axis_enum.set("RightX", GameController::RightX);
		axis_enum.set("RightY", GameController::RightY);
		axis_enum.set("TriggerLeft", GameController::TriggerLeft);
		axis_enum.set("TriggerRight", GameController::TriggerRight);
		axis_enum.set("__COUNT__", GameController::__COUNT__);

		registrar.method("uint64 id() const", &GameController::id);
		registrar.method("int16 axis_value(Engine::GameController::Axis) const", &GameController::axis_value);
		registrar.method("float axis_value_normalized(Engine::GameController::Axis) const",
		                 &GameController::axis_value_normalized);
	}

	static ReflectionInitializeController init(on_init, "Engine::GameController");
}// namespace Engine
