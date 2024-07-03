#include <Core/arguments.hpp>
#include <Core/base_engine.hpp>
#include <Core/string_functions.hpp>
#include <Engine/settings.hpp>
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
        attributes   = Settings::w_attributes.to_set<decltype(attributes)>();
        orientations = Settings::w_attributes.to_set<decltype(orientations)>();
        title        = Settings::w_title;
        client       = Settings::w_client;
        size.x       = Settings::w_size_x;
        size.y       = Settings::w_size_y;
        position.x   = Settings::w_pos_x;
        position.y   = Settings::w_pos_y;
        vsync        = Settings::w_vsync;
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
