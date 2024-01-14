#include <Core/global_config.hpp>
#include <Core/platform.hpp>
#include <editor_config.hpp>
#include <theme.hpp>

namespace Engine
{
    EditorConfig editor_config;

    static const String editor_default_font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";

    EditorConfig& EditorConfig::update()
    {
        const auto& editor_json = global_config.checked_get("Editor").checked_get<JSON::JsonObject>();
        font_path               = editor_json.checked_get_value<JSON::JsonString>("font_path", editor_default_font);
        font_size               = editor_json.checked_get_value<JSON::JsonFloat>("font_size", 18.f);

        return *this;
    }

    EditorConfig& EditorConfig::update_using_args()
    {
        return *this;
    }
}// namespace Engine
