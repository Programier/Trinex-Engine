#include <Core/config_manager.hpp>
#include <Core/engine_loading_controllers.hpp>
#include <editor_config.hpp>

namespace Engine::Settings
{
    String ed_font_path        = "resources/fonts/Source Code Pro/SourceCodePro-Bold.ttf";
    float ed_font_size         = 18.f;
    float ed_collapsing_indent = 5.f;

    static ReflectionInitializeController initialize([]() {
        ConfigManager::register_property("ed_font_path", ed_font_path, "editor");
        ConfigManager::register_property("ed_font_size", ed_font_size, "editor");
        ConfigManager::register_property("ed_collapsing_indent", ed_collapsing_indent, "editor");
    });
}// namespace Engine::Settings
