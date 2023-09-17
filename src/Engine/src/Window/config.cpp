#include <Core/global_config.hpp>
#include <Window/config.hpp>

namespace Engine
{

    static float to_float(const JSON::Value& value)
    {
        if (value.type() == JSON::ValueType::Integer)
        {
            return static_cast<float>(value.checked_get<JSON::JsonInt>());
        }
        else
        {
            return value.checked_get<JSON::JsonFloat>();
        }
    }

    WindowConfig& WindowConfig::update()
    {
        const auto& window_json = global_config.checked_get("Window").checked_get<JSON::JsonObject>();
        String title            = window_json.checked_get("title").checked_get<String>();

        {
            const auto& window_size = window_json.checked_get("size").checked_get<JSON::JsonObject>();
            size.x                  = to_float(window_size.checked_get("x"));
            size.y                  = to_float(window_size.checked_get("y"));
        }
        {
            const auto& attributes_array = window_json.checked_get("attributes").checked_get<JSON::JsonArray>();
            for (auto& ell : attributes_array)
            {
                if (ell.type() == JSON::ValueType::Integer)
                {
                    attributes.push_back(static_cast<WindowAttribute>(ell.get<JSON::JsonInt>()));
                }
            }
        }

        {
            const auto& orientations_array = window_json.checked_get("orientations").checked_get<JSON::JsonArray>();
            for (auto& ell : orientations_array)
            {
                if (ell.type() == JSON::ValueType::Integer)
                {
                    orientations.push_back(static_cast<WindowOrientation>(ell.get<JSON::JsonInt>()));
                }
            }
        }

        {
            vsync = window_json.checked_get_value("vsync", true);
        }
        return *this;
    }


    ENGINE_EXPORT WindowConfig global_window_config;
}// namespace Engine
