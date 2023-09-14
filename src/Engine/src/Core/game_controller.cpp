#include <Core/engine_loading_controllers.hpp>
#include <Core/game_controller.hpp>
#include <Core/logger.hpp>
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
    {}

    static InitializeController init(on_init);
}// namespace Engine
