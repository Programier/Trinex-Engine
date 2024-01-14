#include <Core/global_config.hpp>
#include <Core/platform.hpp>
#include <editor_config.hpp>
#include <theme.hpp>

namespace Engine
{
    EditorConfig editor_config;


    static String find_default_font_path()
    {
        //        Path path = Platform::find_default_font_path();
        //        if (!path.empty())
        //        {
        //            return path.string();
        //        }

        return "resources/fonts/Roboto/Roboto-Bold.ttf";
    }

    EditorConfig& EditorConfig::update()
    {
        const auto& editor_json = global_config.checked_get("Editor").checked_get<JSON::JsonObject>();
        font_path               = editor_json.checked_get_value<JSON::JsonString>("font_path", find_default_font_path());
        font_size               = editor_json.checked_get_value<JSON::JsonFloat>("font_size", 18.f);

        return *this;
    }

    EditorConfig& EditorConfig::update_using_args()
    {
        return *this;
    }
}// namespace Engine
