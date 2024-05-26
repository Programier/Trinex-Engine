#include <Core/base_engine.hpp>
#include <Core/engine_config.hpp>
#include <Core/global_config.hpp>
#include <Core/string_functions.hpp>
#include <Window/config.hpp>
#include <Core/config_manager.hpp>
#include <Core/arguments.hpp>

namespace Engine
{

    static float to_float(const JSON::Value& value, float default_value = 0.f)
    {
        if (value.type() == JSON::ValueType::Integer)
        {
            return static_cast<float>(value.checked_get<JSON::JsonInt>(default_value));
        }
        else
        {
            return value.checked_get<JSON::JsonFloat>(default_value);
        }
    }

    WindowConfig& WindowConfig::update()
    {
        const auto& window_json = global_config.checked_get("Window").checked_get<JSON::JsonObject>();
        title                   = window_json.checked_get("title").checked_get<String>();
        client                  = window_json.checked_get("client").checked_get<String>();

        {
            const auto& window_size = window_json.checked_get("size").checked_get<JSON::JsonObject>();
            size.x                  = to_float(window_size.checked_get("x"));
            size.y                  = to_float(window_size.checked_get("y"));
        }

        {
            const auto& window_pos = window_json.checked_get("position").checked_get<JSON::JsonObject>();
            position.x             = to_float(window_pos.checked_get("x"), -1.f);
            position.y             = to_float(window_pos.checked_get("y"), -1.f);
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

        api_name = ConfigManager::get_string("Engine::api");
        return *this;
    }


#define new_param(name, convert_func)                                                                                            \
    arg = Arguments::find("w_" #name);                                                                                           \
    if (arg && arg->type == Arguments::Type::String)                                                                             \
    {                                                                                                                            \
        name = convert_func(arg->get<const String&>());                                                                          \
    }

#define new_param_nc(name)                                                                                                       \
    arg = Arguments::find("w_" #name);                                                                                           \
    if (arg && arg->type == Arguments::Type::String)                                                                             \
    {                                                                                                                            \
        name = arg->get<const String&>();                                                                                        \
    }


    WindowConfig& WindowConfig::update_using_args()
    {
        using Arg = const Arguments::Argument*;
        Arg arg   = nullptr;

        new_param_nc(title);
        new_param_nc(client);
        new_param(size.x, Strings::convert<float>);
        new_param(size.y, Strings::convert<float>);
        new_param(position.x, Strings::convert<float>);
        new_param(position.y, Strings::convert<float>);
        new_param(vsync, Strings::convert<bool>);

        return *this;
    }

    ENGINE_EXPORT WindowConfig global_window_config;
}// namespace Engine
