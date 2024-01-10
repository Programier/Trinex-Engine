#include <Core/engine_loading_controllers.hpp>
#include <Core/game_controller.hpp>
#include <Core/logger.hpp>
#include <ScriptEngine/registrar.hpp>
#include <numeric>

namespace Engine
{
    GameController::GameController(Identifier controller_id) : _M_ID(controller_id)
    {
        info_log("GameController", "Created game controller %zu", controller_id);
        std::fill(_M_axis_values, _M_axis_values + Axis::__COUNT__, 0);
    }

    Identifier GameController::id() const
    {
        return _M_ID;
    }

    GameController::~GameController()
    {
        info_log("GameController", "Removed game controller %zu", _M_ID);
    }

    short_t GameController::axis_value(Axis axis) const
    {
        return _M_axis_values[axis];
    }

    float GameController::axis_value_normalized(Axis axis) const
    {
        return static_cast<float>(_M_axis_values[axis]) / static_cast<float>(std::numeric_limits<short_t>::max());
    }

    static void on_init()
    {

        ScriptClassRegistrar::ClassInfo info;
        info.size  = sizeof(GameController);
        info.flags = ScriptClassRegistrar::Ref | ScriptClassRegistrar::NoCount;

        ScriptClassRegistrar registrar("Engine::GameController", info);
        ScriptEnumRegistrar axis_enum("Engine::GameController::Axis");

        axis_enum.set("None", GameController::None);
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

    static ScriptEngineInitializeController init(on_init, "Bind GameController");
}// namespace Engine
