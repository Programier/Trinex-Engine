#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/config_manager.hpp>
#include <Core/string_functions.hpp>
#include <Window/config.hpp>

namespace Engine
{
    template<typename T>
    static Set<T> to_set(const Vector<int_t>& input)
    {
        Set<T> result;

        for (auto ell : input)
        {
            result.insert(static_cast<T>(ell));
        }

        return result;
    }

    WindowConfig::WindowConfig()
    {
        initialize();
    }

    WindowConfig& WindowConfig::initialize()
    {
        attributes   = to_set<WindowAttribute>(ConfigManager::get_int_array("Window::attributes"));
        orientations = to_set<WindowOrientation>(ConfigManager::get_int_array("Window::orientations"));
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

    bool WindowConfig::contains_attribute(WindowAttribute attribute) const
    {
        auto it = attributes.find(attribute);
        if (it == attributes.end())
            return false;
        return true;
    }
}// namespace Engine
