#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/string_functions.hpp>
#include <Window/config.hpp>

namespace Engine
{
    template<typename T>
    static Vector<T> to_vector(const Vector<int_t>& input)
    {
        Vector<T> result(input.size());
        std::transform(input.begin(), input.end(), result.begin(), [](int_t value) { return static_cast<T>(value); });
        return result;
    }

    WindowConfig::WindowConfig()
    {
        initialize();
    }

    WindowConfig& WindowConfig::initialize()
    {
        attributes   = to_vector<WindowAttribute>(ConfigManager::get_int_array("Window::attributes"));
        orientations = to_vector<WindowOrientation>(ConfigManager::get_int_array("Window::orientations"));
        title        = ConfigManager::get_string("Window::title");
        api_name     = ConfigManager::get_string("Engine::api");
        client       = ConfigManager::get_string("Window::client");
        size.x       = ConfigManager::get_float("Window::size_x");
        size.y       = ConfigManager::get_float("Window::size_y");
        position.x   = ConfigManager::get_float("Window::pos_x");
        position.y   = ConfigManager::get_float("Window::pos_y");
        vsync        = ConfigManager::get_bool("Window::vsync");
        return *this;
    }
}// namespace Engine
