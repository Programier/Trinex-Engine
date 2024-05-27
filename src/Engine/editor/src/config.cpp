#include <Core/config_manager.hpp>
#include <editor_config.hpp>
#include <theme.hpp>

namespace Engine
{
    EditorConfig editor_config;

    static const String editor_default_font = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";

    EditorConfig& EditorConfig::update()
    {
        ConfigManager::load_string_argument<String>("ed_font_path", "Editor::font_path", editor_default_font);
        ConfigManager::load_string_argument<float>("ed_font_size", "Editor::font_size", 18.f);
        ConfigManager::load_string_argument<float>("ed_collapsing_indent", "Editor::collapsing_indent", 5.f);

        font_path         = ConfigManager::get_path("Editor::font_path");
        font_size         = ConfigManager::get_float("Editor::font_size");
        collapsing_indent = ConfigManager::get_float("Editor::collapsing_indent");

        return *this;
    }
}// namespace Engine
